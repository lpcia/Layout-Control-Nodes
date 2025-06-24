/* Model Railroad Layout Control Node
   Reciever 2
      MP3 Player  
      Traffic Light


Note:  This is set up for a Nano Expansion Module V3


MRRLayoutControlNode_RX_LCN02_V1
*/

#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>

// NRF24L01 pins
#define CE_PIN 7   // D7
#define CSN_PIN 8  // D8

// Motor control pins
#define MOTOR1_IN1 9   // D9
#define MOTOR1_IN2 10  // D10
#define MOTOR2_IN1 5   // D5
#define MOTOR2_IN2 6   // D6

// Potentiometer pins
#define POT1_PIN A1  // Potentiometer for Motor 1
#define POT2_PIN A2  // Potentiometer for Motor 2

// Create RF24 radio object
RF24 radio(CE_PIN, CSN_PIN);

// Set this to the receiver's ID (1-5)
const uint8_t RECEIVER_ID = 2;  // LCN02

// Variables for potentiometer readings
int pot1Value = 0;
int pot2Value = 0;
int lastPot1Value = 0;
int lastPot2Value = 0;

// Deadzone for potentiometer center position
const int POT_DEADZONE = 50;
const int POT_CENTER = 512;

// Define addresses for up to 5 receivers
const byte addresses[][6] = { "LCN01", "LCN02", "LCN03", "LCN04", "LCN05" };

// Message structure (must match transmitter)
struct CommandMessage {
  uint8_t receiverId;   // 0 = all receivers, 1-5 = specific receiver
  uint8_t commandType;  // 1 = short press, 2 = long press, 3 = silence
  char keyValue;        // The key that was pressed
};

// Motor control functions
void motor1Forward(int speed) {
  analogWrite(MOTOR1_IN1, speed);
  analogWrite(MOTOR1_IN2, 0);
}

void motor1Backward(int speed) {
  analogWrite(MOTOR1_IN1, 0);
  analogWrite(MOTOR1_IN2, speed);
}

void motor1Stop() {
  analogWrite(MOTOR1_IN1, 0);
  analogWrite(MOTOR1_IN2, 0);
}

void motor2Forward(int speed) {
  analogWrite(MOTOR2_IN1, speed);
  analogWrite(MOTOR2_IN2, 0);
}

void motor2Backward(int speed) {
  analogWrite(MOTOR2_IN1, 0);
  analogWrite(MOTOR2_IN2, speed);
}

void motor2Stop() {
  analogWrite(MOTOR2_IN1, 0);
  analogWrite(MOTOR2_IN2, 0);
}

void emergencyStop() {
  motor1Stop();
  motor2Stop();
  Serial.println("EMERGENCY STOP");
}

void processPotentiometers() {
  // Read potentiometer values
  pot1Value = analogRead(POT1_PIN);
  pot2Value = analogRead(POT2_PIN);

  // Only update motor 1 if value changed significantly
  if (abs(pot1Value - lastPot1Value) > 5) {
    lastPot1Value = pot1Value;

    // Process Motor 1
    if (pot1Value > POT_CENTER + POT_DEADZONE) {
      // Forward motion - map from deadzone to max
      int speed = map(pot1Value, POT_CENTER + POT_DEADZONE, 1023, 0, 255);
      motor1Forward(speed);
    } else if (pot1Value < POT_CENTER - POT_DEADZONE) {
      // Backward motion - map from min to deadzone
      int speed = map(pot1Value, 0, POT_CENTER - POT_DEADZONE, 255, 0);
      motor1Backward(speed);
    } else {
      // In deadzone - stop motor
      motor1Stop();
    }
  }

  // Only update motor 2 if value changed significantly
  if (abs(pot2Value - lastPot2Value) > 5) {
    lastPot2Value = pot2Value;

    // Process Motor 2
    if (pot2Value > POT_CENTER + POT_DEADZONE) {
      // Forward motion
      int speed = map(pot2Value, POT_CENTER + POT_DEADZONE, 1023, 0, 255);
      motor2Forward(speed);
    } else if (pot2Value < POT_CENTER - POT_DEADZONE) {
      // Backward motion
      int speed = map(pot2Value, 0, POT_CENTER - POT_DEADZONE, 255, 0);
      motor2Backward(speed);
    } else {
      // In deadzone - stop motor
      motor2Stop();
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("LCN02 - Motor Controller starting...");

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

  // Initialize motor control pins
  pinMode(MOTOR1_IN1, OUTPUT);
  pinMode(MOTOR1_IN2, OUTPUT);
  pinMode(MOTOR2_IN1, OUTPUT);
  pinMode(MOTOR2_IN2, OUTPUT);

  // Ensure motors are stopped at startup
  motor1Stop();
  motor2Stop();

  // Initialize analog pins for potentiometers
  pinMode(POT1_PIN, INPUT);
  pinMode(POT2_PIN, INPUT);

  Serial.println("LCN02 - Motor Controller ready");
}

void loop() {
  // Process potentiometer inputs for motor control
  processPotentiometers();

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
          // Also stop motors for safety
          emergencyStop();
          break;
      }
    }
  }

  delay(10);  // Small delay for stability
}

void handleShortPress(char key) {
  switch (key) {
    case '7':
      // Motor 1 function
      break;
    case '8':
      // Motor 2 function
      break;
    case '9':
      // Motor function
      break;
    case '#':
      emergencyStop();
      break;
    default: break;
  }
}

void handleLongPress(char key) {
  switch (key) {
    case '7':
      // Motor 1 function
      break;
    case '8':
      // Motor 2 function
      break;
    case '9':
      // Motor function
      break;
    case '#':
      // Additional emergency stop function if needed
      emergencyStop();
      break;
    default: break;
  }
}
