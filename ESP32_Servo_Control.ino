#include "ServoController.h"
#include "NetworkManager.h"

// Configuration - UPDATE THESE WITH YOUR WIFI CREDENTIALS
const char* WIFI_SSID = "L&M";
const char* WIFI_PASSWORD = "dezmir42";
const int SERVER_PORT = 3333;

// Hardware pins
const int SERVO_X_PIN = 18;  // Pan servo
const int SERVO_Y_PIN = 19;  // Tilt servo
const int LED_PIN = 2;       // Built-in LED

ServoController servoController;
NetworkManager networkManager(SERVER_PORT);

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 Servo Control Starting...");
    
    // Initialize LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    // Initialize servo controller
    servoController.begin();
    servoController.addServo(SERVO_X_PIN, "X", 500, 2500);  // Pan servo
    servoController.addServo(SERVO_Y_PIN, "Y", 500, 2500);  // Tilt servo
    servoController.setSpeed(60); // 60 degrees per second
    
    // Initialize network
    networkManager.begin(WIFI_SSID, WIFI_PASSWORD);
    networkManager.setCommandCallback(handleCommand);
    
    // Center servos
    servoController.centerAll();
    
    // Turn on LED to indicate ready
    digitalWrite(LED_PIN, HIGH);
    
    Serial.println("System ready!");
    Serial.println("IP: " + networkManager.getLocalIP());
}

void loop() {
    servoController.update();
    
    // Blink LED when servos are moving
    static unsigned long lastBlink = 0;
    if (servoController.isMoving()) {
        if (millis() - lastBlink > 100) {
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
            lastBlink = millis();
        }
    } else {
        digitalWrite(LED_PIN, HIGH);
    }
    
    delay(1); // Small delay for stability
}

void handleCommand(String command) {
    command.toUpperCase();
    
    if (command.startsWith("SERVO_X:")) {
        int angle = command.substring(8).toInt();
        servoController.setAngle("X", angle);
        networkManager.sendResponse("OK:SERVO_X:" + String(angle) + "\n");
    }
    else if (command.startsWith("SERVO_Y:")) {
        int angle = command.substring(8).toInt();
        servoController.setAngle("Y", angle);
        networkManager.sendResponse("OK:SERVO_Y:" + String(angle) + "\n");
    }
    else if (command == "SERVO_CENTER") {
        servoController.centerAll();
        networkManager.sendResponse("OK:CENTERED\n");
    }
    else if (command == "SERVO_HOME") {
        servoController.setAngle("X", 0);
        servoController.setAngle("Y", 0);
        networkManager.sendResponse("OK:HOME\n");
    }
    else if (command == "STATUS") {
        String status = "X:" + String(servoController.getCurrentAngle("X")) + 
                       ",Y:" + String(servoController.getCurrentAngle("Y")) + 
                       ",MOVING:" + (servoController.isMoving() ? "1" : "0");
        networkManager.sendResponse("STATUS:" + status + "\n");
    }
    else if (command == "STOP") {
        servoController.stopAll();
        networkManager.sendResponse("OK:STOPPED\n");
    }
    else {
        networkManager.sendResponse("ERROR:UNKNOWN_COMMAND\n");
    }
}
