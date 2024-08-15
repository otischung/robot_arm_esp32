#include <Adafruit_PWMServoDriver.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include "armDriver.h"

#define UPDATE_ARM_DELAY 3.0
#define SERIAL_READ_DELAY 10
#define SERIAL_WRITE_DELAY 250
const uint8_t NUM_OF_SERVOS = 5;
const uint8_t servoMinAngles[] = {0, 0, 0, 0, 0};
const uint8_t servoMaxAngles[] = {180, 120, 160, 180, 110};

// Enum to define the arm actions
enum class ArmAction {
    NO_ACTION,
    TURN_BASE_LEFT,
    TURN_BASE_RIGHT,
    MOVE_ARM_UP,
    MOVE_ARM_DOWN,
    SHIFT_ARM_RIGHT,
    SHIFT_ARM_LEFT,
    ROTATE_FINGER_RIGHT,
    ROTATE_FINGER_LEFT
};

ArmAction armAction = ArmAction::NO_ACTION;
float currentAngles[NUM_OF_SERVOS];
uint8_t targetAngles[NUM_OF_SERVOS];

// Define task handles
TaskHandle_t armControlTask;
TaskHandle_t serialCommunicationTask;
TaskHandle_t serialWriterTask;

// Function declarations
void armControlTaskFunction(void *parameter);
void serialCommunicationTaskFunction(void *parameter);
void serialWriterTaskFunction(void *parameter);

void setup() {
    // Start serial communication
    Serial.begin(115200);

    // Initialize target angles to initial values
    for (uint8_t i = 0; i < NUM_OF_SERVOS; i++) {
        currentAngles[i] = (float) servoMinAngles[i];
        switch (i) {
            case 0:
                targetAngles[0] = 90;
                break;
            case 1:
                targetAngles[1] = 20;
                break;
            case 2:
                targetAngles[2] = 160;
                break;
            case 3:
                targetAngles[3] = 50;
                break;
            case 4:
                targetAngles[4] = 10;
                break;
            default:
                break;
        }
    }

    // Create the arm control task
    Serial.println("xTaskCreatePinnedToCore armControlTaskFunction ");
    delay(100);
    xTaskCreatePinnedToCore(
            armControlTaskFunction, // Task function
            "Arm Control Task",     // Task name
            2048,                   // Stack size (in bytes)
            NULL,                   // Task parameters
            1,                      // Task priority
            &armControlTask,        // Task handle
            0                       // Core ID (0 or 1)
    );

    // Create the serial communication task
    Serial.println("xTaskCreatePinnedToCore serialCommunicationTaskFunction ");
    delay(100);
    xTaskCreatePinnedToCore(
            serialCommunicationTaskFunction, // Task function
            "Serial Communication Task",     // Task name
            2048,                            // Stack size (in bytes)
            NULL,                            // Task parameters
            3,                               // Task priority
            &serialCommunicationTask,        // Task handle
            1                                // Core ID (0 or 1)
    );

    // Create the serial writer task
    Serial.println("xTaskCreatePinnedToCore serialWriterTaskFunction ");
    delay(100);
    xTaskCreatePinnedToCore(
            serialWriterTaskFunction, // Task function
            "Serial Writer Task",     // Task name
            2048,                     // Stack size (in bytes)
            NULL,                     // Task parameters
            2,                        // Task priority
            &serialWriterTask,        // Task handle
            0                         // Core ID (0 or 1)
    );
    delay(100);
}

void loop() {
    // Empty. Tasks are running in the background.
}

// Task function for arm control
void armControlTaskFunction(void *parameter) {
    // Create an instance of ArmManager
    ArmManager armManager(NUM_OF_SERVOS, servoMinAngles, servoMaxAngles);

    Serial.println("armControlTaskFunction start");

    for (;;) {
        // Control the robot arm using ArmManager
        for (uint8_t i = 0; i < NUM_OF_SERVOS; i++) {
            armManager.setServoTargetAngle(i, targetAngles[i]);
        }

        armManager.moveArm();
        armManager.getCurrentAngles(currentAngles);
        armAction = ArmAction::NO_ACTION;

        // Wait for some time before the next iteration
        vTaskDelay(UPDATE_ARM_DELAY / portTICK_PERIOD_MS);
    }
}

// Task function for serial reader
void serialCommunicationTaskFunction(void *parameter) {
    Serial.println("serialCommunicationTaskFunction start ");
    String jsonString = "";

    for (;;) {
        if (Serial.available()) {
            // Read the incoming serial data
            jsonString = Serial.readStringUntil('\n');
            Serial.println("receive:");
            Serial.println(jsonString);

            // Parse the JSON string
            StaticJsonDocument<256> doc;
            DeserializationError error = deserializeJson(doc, jsonString);

            // Check if the JSON parsing was successful
            if (error) {
                Serial.print("JSON parsing error: ");
                Serial.println(error.c_str());
            } else {
                // Check if the parsed JSON document contains the "servo_target_angles" array
                if (doc.containsKey("servo_target_angles")) {
                    // Get the "servo_target_angles" array from the JSON document
                    JsonArray servoAngles = doc["servo_target_angles"].as<JsonArray>();

                    // Ensure we only process up to NUM_OF_SERVOS angles
                    uint8_t numAngles = (uint8_t)min((uint8_t)servoAngles.size(), NUM_OF_SERVOS);

                    // Update the target angles in the ArmManager
                    for (uint8_t i = 0; i < numAngles; i++) {
                        int angle = servoAngles[i];
                        if (angle >= 0) {
                            targetAngles[i] = angle;
                        } else {
                            // Keep the current angle if the value is negative
                            targetAngles[i] = currentAngles[i];
                        }
                    }
                } else {
                    Serial.println("Missing servo_target_angles in JSON");
                }
            }
        }

        // Wait for some time before the next iteration
        vTaskDelay(SERIAL_READ_DELAY / portTICK_PERIOD_MS);
    }
}

// Task function for serial writer
void serialWriterTaskFunction(void *parameter) {
    Serial.println("serialWriterTaskFunction start");

    for (;;) {
        StaticJsonDocument<512> doc;
        // Populate the JSON document with the current angles
        JsonArray servoAngles = doc.createNestedArray("servo_current_angles");

        for (uint8_t i = 0; i < NUM_OF_SERVOS; i++) {
            servoAngles.add(currentAngles[i]);
        }

        // Serialize the JSON document to a string
        String jsonString;
        serializeJson(doc, jsonString);

        // Print the JSON string
        Serial.println(jsonString);

        // Wait for some time before the next iteration
        vTaskDelay(SERIAL_WRITE_DELAY / portTICK_PERIOD_MS);
    }
}
