# 7-dof-leader-arm

A passive 7-DOF leader arm that measures the operator's arm pose for teleoperation. Seven AS5600 magnetic encoders track the joints, and an ESP32 reads and displays the angles in real time.

Because all seven encoders share the same fixed I2C address, they connect through a TCA9548A multiplexer. The OLED runs on a separate software-I2C bus so all seven joint readings can be displayed at once.

## Demo

Overall build:

<img width="1086" height="1111" alt="leader arm" src="https://github.com/user-attachments/assets/e54041f2-99cd-4700-824c-0b401a5d335b" />

Electronics:

<img width="4032" height="3024" alt="IMG_7308" src="https://github.com/user-attachments/assets/677da52e-ac23-47be-b324-65ae947ea1c1" />



Demo video — live joint-angle tracking:

<!-- Drag the demo video below this line. -->

## How it works

- Each joint rotates a diametric magnet over an AS5600, providing contactless absolute-angle measurement.
- A TCA9548A multiplexer lets the ESP32 read all seven encoders even though they share the same `0x36` I2C address.
- The ESP32 selects one multiplexer channel at a time, reads the sensor, and converts its 12-bit output to degrees.
- The joint angles are displayed as J1 through J7 on a 128 x 64 OLED.
- The mechanism is fully passive; the operator moves the joints directly and no motors are used in the leader arm.
- The measured angles will be streamed to the follower arm for teleoperation.

## Hardware

- ESP32 development board
- 7 AS5600 magnetic encoders
- 7 diametric magnets
- TCA9548A I2C multiplexer
- 128 x 64 SSD1309 OLED
- Custom 7-DOF passive mechanical assembly

## Wiring

- TCA9548A: SDA on GPIO 9, SCL on GPIO 8, address `0x70`
- AS5600 encoders: address `0x36`, using multiplexer channels `0`, `1`, `7`, `6`, `5`, `4`, and `3` for J1 through J7
- OLED: SDA on GPIO 14, SCL on GPIO 13, reset on GPIO 12, address `0x3C`

## Running it

Install the ESP32 board package and the `U8g2` library through the Arduino IDE. Open the leader-arm firmware, select the correct ESP32 board and serial port, then compile and upload it.
