#include <SPI.h>
#include <RF24.h>

RF24 radio(7, 8); // CE, CSN

const byte address[6] = "NODE1";

struct Payload {
  float latitude;
  float longitude;
  char message[32];
};

Payload dataToSend;

void setup() {
  Serial.begin(9600);
  Serial.println("🔧 Starting Transmitter...");

  if (!radio.begin()) {
    Serial.println("❌ NRF24L01 not responding. Check wiring and power!");
    while (1);
  }

  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(108);
  radio.openWritingPipe(address);
  radio.stopListening(); // TX mode

  delay(1000); // Give time for receiver to start
  Serial.println("✅ Transmitter initialized. Sending data...");
}

void loop() {
  dataToSend.latitude = 51.5074;
  dataToSend.longitude = -0.1278;
  strcpy(dataToSend.message, "Ping from TX!");

  Serial.println("📡 Sending packet...");
  Serial.print("Latitude: "); Serial.println(dataToSend.latitude, 6);
  Serial.print("Longitude: "); Serial.println(dataToSend.longitude, 6);
  Serial.print("Message: "); Serial.println(dataToSend.message);

  bool sent = radio.write(&dataToSend, sizeof(dataToSend));

  if (sent) {
    Serial.println("✅ Data sent successfully!");
  } else {
    Serial.println("❌ Failed to send data.");
  }

  Serial.println("-------------------------------");
  delay(2000); // Slower loop
}

