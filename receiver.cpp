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

void setup() {
  Serial.begin(9600);
  Serial.println("🔧 Starting Receiver...");

  if (!radio.begin()) {
    Serial.println("❌ NRF24L01 not responding. Check wiring and power!");
    while (1);
  }

  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(108);
  radio.openReadingPipe(1, address);
  radio.startListening(); // RX mode

  Serial.println("✅ Receiver initialized. Waiting for packets...");
}

void loop() {
  if (radio.available()) {
    radio.read(&receivedData, sizeof(receivedData));

    Serial.println("📨 Packet received!");
    Serial.print("Latitude: "); Serial.println(receivedData.latitude, 6);
    Serial.print("Longitude: "); Serial.println(receivedData.longitude, 6);
    Serial.print("Message: "); Serial.println(receivedData.message);
    Serial.println("-------------------------------");

    delay(500); // Brief delay to avoid flooding serial
  } else {
    delay(100); // Wait before polling again
  }
}

