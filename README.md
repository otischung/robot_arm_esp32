# ESP32 C++ Code for Controlling 5-Axis Robot Arm (Left)

We need the following items.

- Arduino IDE
- Node32MCU ESP32 Board



# Environment Settings

### Windows Driver for ESP32

1. Download the [CP210x Universal Windows Driver](https://www.silabs.com/documents/public/software/CP210x_Universal_Windows_Driver.zip).
2. Unzip the compressed file.
3. Right-click the `silabser.inf` and select "install".



### Arduino IDE

1. Install [Arduino IDE](https://support.arduino.cc/hc/en-us/articles/360019833020-Download-and-install-Arduino-IDE).

2. Open File > Preferences > Settings > Additional boards manager URLs, and then enter the following URL:

   > https://dl.espressif.com/dl/package_esp32_index.json

   Finally, click OK.

3. Open Tools > Board > Boards Manager, and then install `esp32` made by Espressif Systems.

4. Open Tools > Manage Libraries, and then install the following libraries.

   - `PID` made by Brett Beauregard
   - `ArduinoJson` made by Benoit Blanchon
   - `ESP32Servo` `0.13.0` made by Kevin Harrington, John K. Bennett
   - `ESP32Encoder` `0.9.1` made by Kevin Harrington
   - `EspSoftwareSerial` `8.0.1` made by Dirk Kaar, Peter Lerup
   - `Adafruit-PWM-Servo-Driver-Library` made by Adafruit



## Angles

### J1 (The arm lifts forward, upward, and lowers down)

- Range: [0°, 180°]
  - 0° (0 rad): The arm lowers down.
  - 90° (1.57 rad): The arm lifts forward.
  - 180° (3.14 rad): The arm lifts upward.



### J2

Range: [0°, 90°]

- 0° (0 rad): The arm moves closer to the body.
- 90° (1.57 rad): The arm opens to the left.



### J3

Range: [0°, 180°]

- The direction when J2 is 0°.

  - 0° (0 rad): The forearm points left.

  - 90° (1.57 rad): The forearm points forward.

  - 180° (3.14 rad): The forearm points right.



### J4 (Forearm and Arm)

- Range: [60°, 180°]

  - 60° (1.05 rad): The arm bends.
- 180° (3.14 rad): The arm straightens.



### J5 (Hand/Fixture)

- Range: [0°, 70°]

  - 0° (0 rad): Fixture clamps up.

  - 70° (1.22 rad): Fixture opens up.





## Code Definition and Features

### The Maximum and Minimum Servo Angles

We've defined the max. and min. angles for servos at the following code.

```c++
const uint8_t servoMinAngles[] = {0, 0, 0, 60, 0};
const uint8_t servoMaxAngles[] = {180, 90, 180, 180, 70};
```



### Initial Pose

The initial pose is defined via the following code.

```c++
void setup() {
    // ...
    // Create the arm control task
    for (uint8_t i = 0; i < NUM_OF_SERVOS; i++) {
        currentAngles[i] = (float) servoMinAngles[i];
        switch (i) {
            case 0:
                targetAngles[0] = 0;
                break;
            case 1:
                targetAngles[1] = 0;
                break;
            case 2:
                targetAngles[2] = 90;
                break;
            case 3:
                targetAngles[3] = 90;
                break;
            case 4:
                targetAngles[4] = 30;
                break;
            default:
                break;
        }
    }
    // ...
}
```

