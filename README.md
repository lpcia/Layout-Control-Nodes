# Layout-Control-Nodes
Model Railroad wireless controls for Scenery

Designed and built from an idea presented by https://thenscaler.com/
AI was used to help implement the NRF24L01's

# Summary of Layout Control Nodes

## LCN00 - Transmitter Node

Hardware: Arduino with NRF24L01 radio module and I2C 16 button telephone keypad\
Function: Central control unit that sends commands to all other nodes

### Key Handling:
- Routes each key press to the appropriate receiver node
- Distinguishes between short and long presses
- Has a dedicated silence button for emergency stop

### Routing Logic:
- Keys 1-6, A, B → LCN01 (Audio & Traffic Light)
- Keys 7-9, # → LCN02 (Motor Control)
- Keys C, D → LCN03 (Reserved for future use)
- Key * → LCN04 (Reserved for future use)
- Key 0 → LCN05 (Shift Register/Lighting)

### Communication:
- Uses NRF24L01 radio to send commands wirelessly

## LCN01 - Audio & Traffic Light Controller
Hardware: Arduino with NRF24L01 radio, DY audio player, and 3 LEDs (Red, Yellow, Green)

### Functions:
- Audio Playback: Plays MP3 files from the DY player module
- Traffic Light Control: Controls a set of traffic lights

### Key Commands:
- Keys 1-6: Play audio files (/001.mp3 through /006.mp3)
- Long press 1-6: Play alternate audio files (/601.mp3 through /606.mp3)
- Key A (short): Cycle traffic lights (Green → Yellow → Red → Off → Green)
- Key A (long): Run traffic light sequence 3 times
- Key B: Play audio file /617.mp3 (Silence) 

## LCN02 - Motor Controller
Hardware: Arduino with NRF24L01 radio, DRV8833 motor driver, and 2 potentiometers

### Functions:
- Motor Control: Controls 2 DC motors using potentiometers
- Emergency Stop: Provides emergency stop functionality
### Control Method:
- Potentiometer on A1: Controls Motor 1 speed and direction
- Potentiometer on A2: Controls Motor 2 speed and direction
- Center position stops the motors
### Key Commands:
- Keys 7-9: Reserved for motor functions (not fully implemented)
- Key # (short): Emergency stop - immediately stops both motors
- Silence command: Also triggers emergency stop

## LCN05 - Shift Register Controller (Lighting Effects)
Hardware: Arduino with NRF24L01 radio and shift register(s)
### Functions:
- Light Sequence: Controls a sequence of 8 lights
- Welder Effect: Creates a random flashing effect to simulate welding
### Key Commands:
- Key 0 (short): Start the 8-light sequence
- Key 0 (long): Start the welder effect simulation
- Silence command: Turn off all lights

##General System Features

### Wireless Communication: 
All nodes communicate wirelessly using NRF24L01 radio modules
Addressing: Each node has a unique address (LCN01 through LCN05)
### Command Types:
- Short press (commandType = 1)
- Long press (commandType = 2)
- Silence/emergency (commandType = 3)

Expandability: System can be expanded with additional nodes (LCN03 and LCN04 are reserved)
This modular system allows for distributed control of various model railroad functions, with each node handling specific tasks while receiving commands from the central transmitter. The wireless nature makes installation easier without needing to run control wires throughout the layout.

### NOTE:  Still lots of testing to do, but LCN01 works solid.
