/* Model Railroad Layout Control Node
   Transmitter (or Master)

A system with one transmitter module and up to 5 receiver modules using NRF24L01 radios.
The transmitter will send keypad commands to the receivers, which will play audio files accordingly.

Required Libraries
Make sure to install these libraries in your Arduino IDE:

RF24 by TMRh20 (for NRF24L01 communication)
DYPlayerArduino (for audio playback)
I2CKeyPad (for keypad input)
Implementation Notes

Transmitter Configuration:
The transmitter reads the keypad and silence button
It detects short and long presses
It sends commands to specific receivers or all receivers
Receiver Configuration:

Each receiver has a unique ID (1-5)
They listen for commands addressed to them or to all receivers
They play audio files based on the received commands
Addressing System:

Each receiver has a unique address ("LCN01", "LCN02", etc.)
The transmitter can target specific receivers or broadcast to all
Message Structure:

receiverId: 0 for all receivers, 1-5 for specific receivers
commandType: 1=short press, 2=long press, 3=silence
keyValue: The key that was pressed
Customization:

Change the RECEIVER_ID constant in each receiver's code to match its position (1-5)
Adjust radio power level (RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX) based on your range needs
Modify the channel if you experience interference

Setup NOTES:
If Using Nano Terminal Adapter V2.0 (Better for transmitter - More open PWM pins)
#define CE_PIN 10  // D7 for Nano Terminal Adapter V3 D10 for V2  V2 better for TX
#define CSN_PIN 9  // D8 for Nano Terminal Adapter V3 D9 for V2
D11 is also a PWM Pin that is being wasted

Replace these lines if Transmitter is using Nano Terminal Adapter Version 2.

// NRF24L01 pins
#define CE_PIN 10  // D7 for Nano Terminal Adapter V3 D10 for V2  V2 better for TX
#define CSN_PIN 9  // D8 for Nano Terminal Adapter V3 D9 for V2


*/


#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include "I2CKeyPad.h"

// NRF24L01 pins
#define CE_PIN 7   // D7
#define CSN_PIN 8  // D8

// Define addresses for up to 5 receivers
const byte addresses[][6] = { "LCN01", "LCN02", "LCN03", "LCN04", "LCN05" };
const uint8_t NUM_RECEIVERS = 5;

// Create RF24 radio object
RF24 radio(CE_PIN, CSN_PIN);

// Keypad setup
const uint8_t KEYPAD_ADDRESS = 0x20;
I2CKeyPad keyPad(KEYPAD_ADDRESS);
uint32_t lastKeyPressed = 0;
char iChar = ' ';
char lastKey = 'N';

// Long press detection
const unsigned long LONG_PRESS_THRESHOLD = 1000;  // 1 second
unsigned long pressStartTime = 0;
bool longPressProcessed = false;
bool shortPressProcessed = false;

// Silence button
const int SILENCE_PIN = 2;  // D2 on Nano

// Message structure
struct CommandMessage {
  uint8_t receiverId;   // 0 = all receivers, 1-5 = specific receiver
  uint8_t commandType;  // 1 = short press, 2 = long press, 3 = silence
  char keyValue;        // The key that was pressed
};

void setup() {
  Serial.begin(115200);
  Serial.println("Transmitter starting...");

  // Initialize radio
  if (!radio.begin()) {
    Serial.println("Radio hardware not responding!");
    while (1) {}  // Hold in infinite loop
  }

  // Radio setup
  radio.setPALevel(RF24_PA_LOW);    // RF24_PA_MAX for longer range
  radio.setDataRate(RF24_250KBPS);  // Slower for better range
  radio.setChannel(76);             // Channel (0-125, away from WiFi)
  radio.setRetries(3, 5);           // Delay, count

  // Setup for broadcasting to all receivers
  radio.stopListening();

  // Setup keypad
  if (!keyPad.begin()) {
    Serial.println("Keypad not found!");
    while (1)
      ;
  }

  // Setup silence button with internal pullup
  pinMode(SILENCE_PIN, INPUT_PULLUP);

  Serial.println("Transmitter ready");
}

// Function to send to a specific receiver
void sendToReceiver(CommandMessage &msg, uint8_t receiverId) {
  if (receiverId > 0 && receiverId <= NUM_RECEIVERS) {
    radio.openWritingPipe(addresses[receiverId - 1]);
    radio.write(&msg, sizeof(msg));
  }
}

// Function to determine which receiver to send to based on key
uint8_t getReceiverForKey(char key) {
  // According to the table:
  // Keys 1-6, A, B -> LCN01
  // Keys 7-9, # -> LCN02
  // Keys C, D -> LCN03
  // Key * -> LCN04
  // Key 0 -> LCN05

  if (key >= '1' && key <= '6' || key == 'A' || key == 'B') {
    return 1;  // LCN01
  } else if (key >= '7' && key <= '9' || key == '#') {
    return 2;  // LCN02
  } else if (key == 'C' || key == 'D') {
    return 3;  // LCN03
  } else if (key == '*') {
    return 4;  // LCN04
  } else if (key == '0') {
    return 5;  // LCN05
  }

  return 0;  // Default (should not reach here)
}

void loop() {
  uint32_t now = millis();
  char plpKey[19] = "123A456B789C*0#DNF";
  uint8_t index = keyPad.getKey();

  // Check silence button
  if (digitalRead(SILENCE_PIN) == LOW) {
    CommandMessage msg = { 0, 3, 'S' };  // Send to all receivers
    for (int i = 0; i < NUM_RECEIVERS; i++) {
      radio.openWritingPipe(addresses[i]);
      radio.write(&msg, sizeof(msg));
      delay(5);  // Small delay between transmissions
    }
    Serial.println("Silence command sent to all receivers");
    delay(50);  // Simple debounce
  }

  if (index < strlen(plpKey)) {
    iChar = plpKey[index];
  } else {
    iChar = 'N';
  }

  // Key press handling with long press detection
  if (iChar != 'N' && iChar != lastKey) {  // New key press
    pressStartTime = now;
    lastKey = iChar;
    longPressProcessed = false;
    shortPressProcessed = false;
  } else if (iChar != 'N' && iChar == lastKey) {  // Key still held
    if (!longPressProcessed && (now - pressStartTime >= LONG_PRESS_THRESHOLD)) {
      // Long press detected
      longPressProcessed = true;
      shortPressProcessed = true;

      // Determine which receiver to send to
      uint8_t receiverId = getReceiverForKey(iChar);

      CommandMessage msg = { receiverId, 2, iChar };  // Long press
      sendToReceiver(msg, receiverId);

      Serial.print("Long press sent: ");
      Serial.print(iChar);
      Serial.print(" to receiver: LCN0");
      Serial.println(receiverId);
    }
  } else if (iChar == 'N' && lastKey != 'N') {  // Key released
    if (!longPressProcessed && !shortPressProcessed) {
      // Short press detected

      // Determine which receiver to send to
      uint8_t receiverId = getReceiverForKey(lastKey);

      CommandMessage msg = { receiverId, 1, lastKey };  // Short press
      sendToReceiver(msg, receiverId);

      Serial.print("Short press sent: ");
      Serial.print(lastKey);
      Serial.print(" to receiver: LCN0");
      Serial.println(receiverId);
    }
    lastKey = 'N';
  }

  delay(50);  // Small delay for stability
}


