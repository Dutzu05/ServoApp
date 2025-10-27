#include <ESP32Servo.h>

// Servo Controller Class
class ServoController {
private:
  Servo servo;
  int pin;
  int currentAngle;
  int minAngle;
  int maxAngle;
  unsigned long lastUpdate;
  int updateInterval;

public:
  ServoController(int servoPin, int minAng = 0, int maxAng = 180, int interval = 50) {
    pin = servoPin;
    minAngle = minAng;
    maxAngle = maxAng;
    currentAngle = 90; // Start at middle position
    updateInterval = interval;
    lastUpdate = 0;
  }

  void begin() {
    servo.attach(pin);
    servo.write(currentAngle);
  }

  void setAngle(int angle) {
    angle = constrain(angle, minAngle, maxAngle);
    if (millis() - lastUpdate >= updateInterval) {
      currentAngle = angle;
      servo.write(currentAngle);
      lastUpdate = millis();
    }
  }

  void incrementAngle(int step = 5) {
    setAngle(currentAngle + step);
  }

  void decrementAngle(int step = 5) {
    setAngle(currentAngle - step);
  }

  int getCurrentAngle() {
    return currentAngle;
  }

  int getMinAngle() {
    return minAngle;
  }

  int getMaxAngle() {
    return maxAngle;
  }
};

// Potentiometer Reader Class
class PotentiometerReader {
private:
  int pin;
  int rawValue;
  int smoothedValue;
  int numReadings;
  int* readings;
  int readIndex;
  int total;

public:
  PotentiometerReader(int potPin, int smoothingReadings = 5) {
    pin = potPin;
    numReadings = smoothingReadings;
    readings = new int[numReadings];
    readIndex = 0;
    total = 0;
    rawValue = 0;
    smoothedValue = 0;
    
    // Initialize readings array
    for (int i = 0; i < numReadings; i++) {
      readings[i] = 0;
    }
  }

  ~PotentiometerReader() {
    delete[] readings;
  }

  void update() {
    // Remove oldest reading
    total = total - readings[readIndex];
    
    // Read new value
    readings[readIndex] = analogRead(pin);
    rawValue = readings[readIndex];
    
    // Add to total
    total = total + readings[readIndex];
    
    // Advance index
    readIndex = (readIndex + 1) % numReadings;
    
    // Calculate average
    smoothedValue = total / numReadings;
  }

  int getRawValue() {
    return rawValue;
  }

  int getSmoothedValue() {
    return smoothedValue;
  }

  int mapToRange(int minVal, int maxVal) {
    return map(smoothedValue, 0, 4095, minVal, maxVal);
  }
};

// Wave Generator Class
class WaveGenerator {
private:
  unsigned long previousTime;
  bool waveState;
  int period;
  int dutyCycle;

public:
  WaveGenerator(int wavePeriod = 100) {
    previousTime = 0;
    waveState = false;
    period = wavePeriod;
    dutyCycle = 50;
  }

  void setDutyCycle(int duty) {
    dutyCycle = constrain(duty, 5, 95);
  }

  void setPeriod(int newPeriod) {
    period = newPeriod;
  }

  int generateWave() {
    unsigned long currentTime = millis();
    
    int highTime = (period * dutyCycle) / 100;
    int lowTime = period - highTime;
    
    if (waveState == false) {
      if (currentTime - previousTime >= lowTime) {
        waveState = true;
        previousTime = currentTime;
      }
      return 0;
    } else {
      if (currentTime - previousTime >= highTime) {
        waveState = false;
        previousTime = currentTime;
      }
      return 1;
    }
  }

  int getDutyCycle() {
    return dutyCycle;
  }
};

// Input Handler Class
class InputHandler {
private:
  String inputBuffer;

public:
  InputHandler() {
    inputBuffer = "";
  }

  char checkForInput() {
    if (Serial.available() > 0) {
      char incomingChar = Serial.read();
      
      // Convert to uppercase for consistency
      if (incomingChar >= 'a' && incomingChar <= 'z') {
        incomingChar = incomingChar - 32;
      }
      
      // Only return valid commands
      if (incomingChar == 'W' || incomingChar == 'S') {
        return incomingChar;
      }
    }
    return 0; // No valid input
  }
};

// Main Application Class
class ServoControlApp {
private:
  ServoController* servoController;
  PotentiometerReader* potReader;
  WaveGenerator* waveGen;
  InputHandler* inputHandler;
  bool keyboardMode;
  unsigned long lastPrint;

public:
  ServoControlApp() {
    servoController = new ServoController(18);
    potReader = new PotentiometerReader(34, 8);
    waveGen = new WaveGenerator(100);
    inputHandler = new InputHandler();
    keyboardMode = false;
    lastPrint = 0;
  }

  ~ServoControlApp() {
    delete servoController;
    delete potReader;
    delete waveGen;
    delete inputHandler;
  }

  void begin() {
    Serial.begin(115200);
    servoController->begin();
    
    Serial.println("=== Servo Control System ===");
    Serial.println("Controls:");
    Serial.println("- Turn potentiometer for analog control");
    Serial.println("- Press 'W' to increase angle");
    Serial.println("- Press 'S' to decrease angle");
    Serial.println("- Keyboard input overrides potentiometer");
    Serial.println("=============================");
  }

  void update() {
    // Update potentiometer reading
    potReader->update();
    
    // Check for keyboard input
    char keyInput = inputHandler->checkForInput();
    
    if (keyInput == 'W') {
      servoController->incrementAngle(10);
      keyboardMode = true;
      Serial.println("Keyboard: Angle increased");
    } else if (keyInput == 'S') {
      servoController->decrementAngle(10);
      keyboardMode = true;
      Serial.println("Keyboard: Angle decreased");
    }
    
    // If no keyboard input for a while, switch back to potentiometer mode
    static unsigned long lastKeyInput = 0;
    if (keyInput != 0) {
      lastKeyInput = millis();
    }
    
    if (keyboardMode && (millis() - lastKeyInput > 3000)) {
      keyboardMode = false;
      Serial.println("Switching back to potentiometer control");
    }
    
    // Control servo based on mode
    if (!keyboardMode) {
      int potAngle = potReader->mapToRange(0, 180);
      servoController->setAngle(potAngle);
    }
    
    // Update wave generator with current servo angle
    int currentAngle = servoController->getCurrentAngle();
    int dutyCycle = map(currentAngle, 0, 180, 5, 95);
    waveGen->setDutyCycle(dutyCycle);
    
    // Generate wave
    int waveValue = waveGen->generateWave();
    
    // Send labeled data to Serial Plotter
    Serial.print("Wave:");
    Serial.print(waveValue * 50);    // Square wave (0 or 50)
    Serial.print(",Angle:");
    Serial.print(currentAngle);      // Servo angle (0-180)
    Serial.print(",Duty:");
    Serial.println(dutyCycle);       // Duty cycle percentage (5-95)
    
    // Print status occasionally (but not to plotter)
    if (millis() - lastPrint >= 2000) {
      // Temporarily pause plotter output for status
      delay(10);
      Serial.println("=== STATUS ===");
      Serial.print("Mode: ");
      Serial.print(keyboardMode ? "Keyboard" : "Potentiometer");
      Serial.print(" | Angle: ");
      Serial.print(currentAngle);
      Serial.print("° | Duty Cycle: ");
      Serial.print(dutyCycle);
      Serial.println("%");
      Serial.println("==============");
      lastPrint = millis();
    }
  }

private:
  
};

// Global application instance
ServoControlApp* app;

void setup() {
  app = new ServoControlApp();
  app->begin();
}

void loop() {
  app->update();
  delay(2);
}
