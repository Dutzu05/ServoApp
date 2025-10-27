#pragma once
#include <ESP32Servo.h>
#include <vector>

class ServoController {
public:
    struct ServoConfig {
        int pin;
        int minPulse = 500;   // microseconds
        int maxPulse = 2500;  // microseconds
        int currentAngle = 90;
        int targetAngle = 90;
        String name;
    };

    ServoController();
    void begin();
    void addServo(int pin, const String& name, int minPulse = 500, int maxPulse = 2500);
    void setAngle(int servoIndex, int angle);
    void setAngle(const String& name, int angle);
    void update(); // Call this in loop() for smooth movement
    void setSpeed(int degreesPerSecond) { movementSpeed = degreesPerSecond; }
    bool isMoving() const;
    void stopAll();
    void centerAll();
    
    // Get current positions
    int getCurrentAngle(int servoIndex) const;
    int getCurrentAngle(const String& name) const;
    
private:
    std::vector<Servo> servos;
    std::vector<ServoConfig> configs;
    unsigned long lastUpdateTime;
    int movementSpeed; // degrees per second
    
    int findServoByName(const String& name) const;
    void updateServoPosition(int index);
};
