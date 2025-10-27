#include "ServoController.h"

ServoController::ServoController() : lastUpdateTime(0), movementSpeed(90) {}

void ServoController::begin() {
    lastUpdateTime = millis();
}

void ServoController::addServo(int pin, const String& name, int minPulse, int maxPulse) {
    Servo servo;
    servo.attach(pin, minPulse, maxPulse);
    servos.push_back(servo);
    
    ServoConfig config;
    config.pin = pin;
    config.name = name;
    config.minPulse = minPulse;
    config.maxPulse = maxPulse;
    config.currentAngle = 90;
    config.targetAngle = 90;
    configs.push_back(config);
    
    // Set initial position
    servo.write(90);
    
    Serial.println("Added servo '" + name + "' on pin " + String(pin));
}

void ServoController::setAngle(int servoIndex, int angle) {
    if (servoIndex >= 0 && servoIndex < configs.size()) {
        angle = constrain(angle, 0, 180);
        configs[servoIndex].targetAngle = angle;
        Serial.println("Servo " + configs[servoIndex].name + " target: " + String(angle) + "°");
    }
}

void ServoController::setAngle(const String& name, int angle) {
    int index = findServoByName(name);
    if (index >= 0) {
        setAngle(index, angle);
    }
}

void ServoController::update() {
    unsigned long currentTime = millis();
    unsigned long deltaTime = currentTime - lastUpdateTime;
    
    if (deltaTime >= 20) { // Update every 20ms (50Hz)
        for (int i = 0; i < configs.size(); i++) {
            updateServoPosition(i);
        }
        lastUpdateTime = currentTime;
    }
}

void ServoController::updateServoPosition(int index) {
    ServoConfig& config = configs[index];
    
    if (config.currentAngle != config.targetAngle) {
        // Calculate movement step based on speed
        float degreesPerUpdate = (movementSpeed * 20.0) / 1000.0; // 20ms update interval
        
        if (abs(config.targetAngle - config.currentAngle) <= degreesPerUpdate) {
            // Close enough, snap to target
            config.currentAngle = config.targetAngle;
        } else {
            // Move towards target
            if (config.targetAngle > config.currentAngle) {
                config.currentAngle += degreesPerUpdate;
            } else {
                config.currentAngle -= degreesPerUpdate;
            }
        }
        
        servos[index].write(config.currentAngle);
    }
}

bool ServoController::isMoving() const {
    for (const auto& config : configs) {
        if (config.currentAngle != config.targetAngle) {
            return true;
        }
    }
    return false;
}

void ServoController::stopAll() {
    for (auto& config : configs) {
        config.targetAngle = config.currentAngle;
    }
}

void ServoController::centerAll() {
    for (int i = 0; i < configs.size(); i++) {
        setAngle(i, 90);
    }
}

int ServoController::getCurrentAngle(int servoIndex) const {
    if (servoIndex >= 0 && servoIndex < configs.size()) {
        return configs[servoIndex].currentAngle;
    }
    return -1;
}

int ServoController::getCurrentAngle(const String& name) const {
    int index = findServoByName(name);
    return index >= 0 ? getCurrentAngle(index) : -1;
}

int ServoController::findServoByName(const String& name) const {
    for (int i = 0; i < configs.size(); i++) {
        if (configs[i].name.equalsIgnoreCase(name)) {
            return i;
        }
    }
    return -1;
}
