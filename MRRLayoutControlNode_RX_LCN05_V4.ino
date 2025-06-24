/* Model Railroad Layout Control Node
   Reciever 5
      Shift Register / Welder


Note:  This is set up for a Nano Expansion Module V3

*/


#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>

// NRF24L01 pins
#define CE_PIN 7   // D7
#define CSN_PIN 8  // D8

// Shift Register pins
#define DATA_PIN 2   // D2
#define CLOCK_PIN 3  // D3
#define LATCH_PIN 4  // D4

// Create RF24 radio object
RF24 radio(CE_PIN, CSN_PIN);

// Set this to the receiver's ID (1-5)
const uint8_t RECEIVER_ID = 5;  // LCN05

// Define addresses for up to 5 receivers
const byte addresses[][6] = { "LCN01", "LCN02", "LCN03", "LCN04", "LCN05" };

// Message structure (must match transmitter)
struct CommandMessage {
  uint8_t receiverId;   // 0 = all receivers, 1-5 = specific receiver
  uint8_t commandType;  // 1 = short press, 2 = long press, 3 = silence
  char keyValue;        // The key that was pressed
};

// Function to update shift register
void updateShiftRegister(byte data) {
  digitalWrite(LATCH_PIN, LOW);
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, data);
  digitalWrite(LATCH_PIN, HIGH);
}

// Function to start 8 lights sequence
void start8Lights() {
  // Example sequence - modify as needed
  for (int i = 0; i < 8; i++) {
    byte data = (1 << i);
    updateShiftRegister(data);
    delay(200);
  }

  // Turn all on
  updateShiftRegister(0xFF);
  delay(500);

  // Turn all off
  updateShiftRegister(0x00);
}

// Function to simulate welder
void startWelder() {
  // Random flashing to simulate welding
  for (int i = 0; i < 20; i++) {
    byte data = random(0, 256);  // Random pattern
    updateShiftRegister(data);
    delay(random(50, 150));  // Random timing
  }

  // Turn all off
  updateShiftRegister(0x00);
}

void setup() {
  Serial.begin(115200);
  Serial.println("LCN05 - Shift Register Controller starting...");

  // Initialize radio
  if (!radio.begin()) {
    Serial.println("Radio hardware not responding!");
    while (1) {}  // Hold in infinite loop
  }

  // Radio setup
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(76);

  // Open reading pipe for this receiver's address
  radio.openReadingPipe(1, addresses[RECEIVER_ID - 1]);
  radio.startListening();

  // Initialize shift register pins
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);

  // Initialize shift register to all LEDs off
  updateShiftRegister(0x00);

  Serial.println("LCN05 - Shift Register Controller ready");
}

void loop() {
  if (radio.available()) {
    CommandMessage msg;
    radio.read(&msg, sizeof(msg));

    // Process message if it's for this receiver
    if (msg.receiverId == RECEIVER_ID) {
      Serial.print("Command received: Type=");
      Serial.print(msg.commandType);
      Serial.print(", Key=");
      Serial.println(msg.keyValue);

      // Process the command
      switch (msg.commandType) {
        case 1:  // Short press
          handleShortPress(msg.keyValue);
          break;
        case 2:  // Long press
          handleLongPress(msg.keyValue);
          break;
        case 3:  // Silence command
          // Turn all lights off
          updateShiftRegister(0x00);
          break;
      }
    }
  }

  delay(10);  // Small delay for stability
}

void handleShortPress(char key) {
  switch (key) {
    case '0':
      start8Lights();
      break;
    default: break;
  }
}

void handleLongPress(char key) {
  switch (key) {
    case '0':
      startWelder();
      break;
    default: break;
  }
}
