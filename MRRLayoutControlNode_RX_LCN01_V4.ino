/* Model Railroad Layout Control Node
   Reciever 1
      MP3 Player  
      Traffic Light


Note:  This is set up for a Nano Expansion Module V3

*/

#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include "DYPlayerArduino.h"

// NRF24L01 pins
#define CE_PIN 7   // D7
#define CSN_PIN 8  // D8

// LED pins
#define GREEN_LED 3   // D3
#define RED_LED 2     // D2
#define YELLOW_LED 4  // D4

// Traffic light timing
const unsigned long TRAFFIC_GREEN_TIME = 3000;   // 3 seconds
const unsigned long TRAFFIC_YELLOW_TIME = 1000;  // 1 second
const unsigned long TRAFFIC_RED_TIME = 3000;     // 3 seconds

// Create RF24 radio object
RF24 radio(CE_PIN, CSN_PIN);

// Audio player
DY::Player player;

// Set this to the receiver's ID (1-5)
const uint8_t RECEIVER_ID = 1;  // LCN01

// LED state tracking
uint8_t currentLed = 1;  // 1 = green, 2 = yellow, 3 = red, 0 = off

// Define addresses for up to 5 receivers
const byte addresses[][6] = { "LCN01", "LCN02", "LCN03", "LCN04", "LCN05" };

// Message structure (must match transmitter)
struct CommandMessage {
  uint8_t receiverId;   // 0 = all receivers, 1-5 = specific receiver
  uint8_t commandType;  // 1 = short press, 2 = long press, 3 = silence
  char keyValue;        // The key that was pressed
};

void cycleLeds() {
  // Turn all LEDs off first
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);

  // Increment the current LED (0-3)
  currentLed = (currentLed + 1) % 4;  // Cycle through 0-3 (0 = all off)

  // Turn on the new current LED
  switch (currentLed) {
    case 1:
      digitalWrite(GREEN_LED, HIGH);
      break;
    case 2:
      digitalWrite(YELLOW_LED, HIGH);
      break;
    case 3:
      digitalWrite(RED_LED, HIGH);
      break;
    case 0:
      // All LEDs remain off
      break;
  }
}

void trafficLightCycle() {
  // Green phase
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  delay(TRAFFIC_GREEN_TIME);

  // Yellow phase
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, HIGH);
  digitalWrite(RED_LED, LOW);
  delay(TRAFFIC_YELLOW_TIME);

  // Red phase
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, HIGH);
  delay(TRAFFIC_RED_TIME);
}

void setup() {
  Serial.begin(115200);
  Serial.println("LCN01 - Audio and Traffic Light Controller starting...");

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

  // Initialize audio player
  player.begin();
  player.setVolume(30);
  player.setEq(DY::Eq::Rock);
  player.setCycleMode(DY::PlayMode::OneOff);

  // Initialize LED pins
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);

  Serial.println("LCN01 - Audio and Traffic Light Controller ready");
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
          player.playSpecifiedDevicePath(DY::Device::Flash, "/013.mp3");
          break;
      }
    }
  }

  delay(10);  // Small delay for stability
}

void handleShortPress(char key) {
  switch (key) {
    case '1': player.playSpecifiedDevicePath(DY::Device::Flash, "/001.mp3"); break;
    case '2': player.playSpecifiedDevicePath(DY::Device::Flash, "/002.mp3"); break;
    case '3': player.playSpecifiedDevicePath(DY::Device::Flash, "/003.mp3"); break;
    case '4': player.playSpecifiedDevicePath(DY::Device::Flash, "/004.mp3"); break;
    case '5': player.playSpecifiedDevicePath(DY::Device::Flash, "/005.mp3"); break;
    case '6': player.playSpecifiedDevicePath(DY::Device::Flash, "/006.mp3"); break;
    case 'A': cycleLeds(); break;  // Traffic light cycling
    case 'B': player.playSpecifiedDevicePath(DY::Device::Flash, "/617.mp3"); break;
    default: break;
  }
}

void handleLongPress(char key) {
  switch (key) {
    case '1': player.playSpecifiedDevicePath(DY::Device::Flash, "/601.mp3"); break;
    case '2': player.playSpecifiedDevicePath(DY::Device::Flash, "/602.mp3"); break;
    case '3': player.playSpecifiedDevicePath(DY::Device::Flash, "/603.mp3"); break;
    case '4': player.playSpecifiedDevicePath(DY::Device::Flash, "/604.mp3"); break;
    case '5': player.playSpecifiedDevicePath(DY::Device::Flash, "/605.mp3"); break;
    case '6': player.playSpecifiedDevicePath(DY::Device::Flash, "/606.mp3"); break;
    case 'A':
      // Run traffic light cycle 3 times
      for (int i = 0; i < 3; i++) {
        trafficLightCycle();
      }
      // Return to the previous LED state
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(YELLOW_LED, LOW);
      digitalWrite(RED_LED, LOW);
      switch (currentLed) {
        case 1: digitalWrite(GREEN_LED, HIGH); break;
        case 2: digitalWrite(YELLOW_LED, HIGH); break;
        case 3:
          digitalWrite(RED_LED, HIGH);
          break;
          // case 0: all LEDs remain off
      }
      break;
    default: break;
  }
}
