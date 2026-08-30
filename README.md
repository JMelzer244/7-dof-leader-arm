7-DOF Leader Arm

A passive 7-degree-of-freedom leader arm developed for WiscoHumanoids. The arm measures each joint angle and provides position data that can be used to teleoperate a humanoid robot arm.

Status: Active development. The mechanical design, electronics, and firmware may change as the system is tested.

Project Media

Overall Build

Photo coming soon: Full assembled 7-DOF leader arm.

<!-- Replace the line above with:
![Overall leader arm build](docs/images/overall-build.jpg)
-->

Electronics

Photo coming soon: ESP32, multiplexer, encoder, and OLED electronics assembly.

<!-- Replace the line above with:
![Leader arm electronics](docs/images/electronics.jpg)
-->

Demo Video

Video coming soon: Live joint sensing and humanoid-arm teleoperation demo.

<!-- Replace the line above with the following and insert the video URL:
[![Leader arm demo video](docs/images/demo-thumbnail.jpg)](DEMO_VIDEO_URL)
-->

System Overview

The leader arm has no powered joints. Each joint uses an AS5600 magnetic encoder to measure its rotation. An ESP32 reads all seven encoders through a TCA9548A I2C multiplexer and displays the joint angles on an OLED screen.

Current Features

Seven independently measured joints

Contactless magnetic angle sensing

Live joint-angle display

Compact ESP32-based electronics

Wireless communication capability for future robot control

Hardware

Component

Quantity

Purpose

ESP32 development board

1

Main controller

AS5600 magnetic encoder

7

Measures joint angles

TCA9548A I2C multiplexer

1

Connects encoders with identical I2C addresses

128 x 64 SSD1309 OLED

1

Displays all joint readings

Diametric magnets

7

Provides the magnetic field for each encoder

Custom mechanical joints

7

Replicates the motion of a human arm

Electronics Architecture

The TCA9548A allows the ESP32 to communicate with multiple AS5600 sensors even though every sensor uses the same 0x36 I2C address. Each encoder is connected to a separate multiplexer channel. The OLED uses a separate software-I2C bus.

Current Connections

Device

Connection

TCA9548A SDA

ESP32 GPIO 9

TCA9548A SCL

ESP32 GPIO 8

TCA9548A address

0x70

AS5600 address

0x36

OLED SDA

ESP32 GPIO 14

OLED SCL

ESP32 GPIO 13

OLED reset

ESP32 GPIO 12

OLED address

0x3C

The seven encoders currently use TCA9548A channels 0, 1, 7, 6, 5, 4, and 3 for joints J1 through J7.

Firmware Setup

Install the Arduino IDE.

Install ESP32 board support through the Arduino IDE Boards Manager.

Install the U8g2 library through the Arduino IDE Library Manager.

Open the leader-arm firmware file.

Select the correct ESP32 board and serial port.

Compile and upload the firmware.

Repository Structure

7-dof-leader-arm/
|-- firmware/       ESP32 and sensor code
|-- hardware/
|   |-- cad/        Mechanical CAD exports
|   `-- electronics/ Wiring diagrams and PCB files
|-- docs/           Build notes, images, and testing documentation
|-- LICENSE
`-- README.md

Roadmap

Finalize all seven mechanical joints

Calibrate joint offsets and directions

Stream joint positions wirelessly

Map leader-arm motion to the humanoid follower arm

Document assembly and wiring

Validate repeatability and range of motion

License

The software in this repository is available under the MIT License. Hardware design files may receive a separate hardware-specific license as the project develops.
