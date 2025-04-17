#include <SPI.h>
#include <RF24.h>

RF24 radio(7, 8); // CE, CSN

const byte address[6] = "NODE1";

struct Payload {
  float latitude;
  float longitude;
  char message[32];
};

Payload receivedData;
Payload sendData;

void setup() {
  Serial.begin(9600);
  Serial.println("🔧 Starting Transceiver...");

  if (!radio.begin()) {
    Serial.println("❌ NRF24L01 not responding. Check wiring and power!");
    while (1);
  }

  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(108);
  radio.openReadingPipe(1, address);
  radio.openWritingPipe(address);
  radio.startListening(); // Begin in listening mode

  Serial.println("✅ Transceiver initialized. Type a message to send:");
}

void loop() {
  // === RECEIVE PACKETS ===
  if (radio.available()) {
    radio.read(&receivedData, sizeof(receivedData));

    Serial.println("📨 Packet received!");
    Serial.print("Latitude: "); Serial.println(receivedData.latitude, 6);
    Serial.print("Longitude: "); Serial.println(receivedData.longitude, 6);
    Serial.print("Message: "); Serial.println(receivedData.message);
    Serial.println("-------------------------------");
  }

  // === READ SERIAL AND SEND ===
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.length() > 0) {
      input.toCharArray(sendData.message, sizeof(sendData.message));
      sendData.latitude = 0.0;
      sendData.longitude = 0.0;

      radio.stopListening(); // Switch to transmit mode
      bool success = radio.write(&sendData, sizeof(sendData));
      radio.startListening(); // Switch back to receive mode

      if (success) {
        Serial.println("📤 Message sent!");
      } else {
        Serial.println("❌ Failed to send message.");
      }
    }
  }

  delay(100); // Keep loop responsive
}

