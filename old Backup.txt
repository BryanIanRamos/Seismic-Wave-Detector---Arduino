#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd_1(0x27, 16, 2);

const int buttonPin0 = A0;
const int buttonPin1 = A1;
const int buttonPin2 = A2;

const int ledPin2 = 2;
const int ledPin3 = 3;
const int ledPin4 = 4;

const int buzzerPin = 8;

const float voltageReference = 5.0;
const float zeroGVoltage = voltageReference / 2.0;
const float sensitivity = 0.3;
const int samples = 10;
const int buttonPin = 9;
bool i = false;

// const float xThresholdLow = 2.5;    // Increase to reduce sensitivity along X-axis
// const float xThresholdHigh = 1.1;   // Decrease to reduce sensitivity along X-axis
// const float yThresholdLow = 3.5;    // Increase to reduce sensitivity along Y-axis
// const float yThresholdHigh = 0.5;   // Decrease to reduce sensitivity along Y-axis

const float xThresholdLow = 3.0;    // Increase to reduce sensitivity along X-axis
const float xThresholdHigh = 2.1;   // Decrease to reduce sensitivity along X-axis
const float yThresholdLow = 3.0;    // Increase to reduce sensitivity along Y-axis
const float yThresholdHigh = 2.1;   // Decrease to reduce sensitivity along Y-axis

// (xAcceleration > xThresholdLow || xAcceleration < xThresholdHigh) ||
//        (yAcceleration > yThresholdLow || yAcceleration < yThresholdHigh)

unsigned long lastShakeTime = 0;
unsigned long buzzerStartTime = 0; // Variable to store the start time of the buzzer
bool isBuzzerActive = false;
bool isTriggered = false;
bool normalLed = true;
unsigned long lastChangeTime = 0; // Variable to store the last change time for buzzer beep

void setup() {
  lcd_1.init();
  lcd_1.backlight();

  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);

  Serial.begin(115200);

  pinMode(buttonPin0, INPUT);
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);

  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  pinMode(ledPin4, OUTPUT);

  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  unsigned long currentTime = millis();

  if (digitalRead(buttonPin) == HIGH) {
    Serial.println("Pressed");
    digitalWrite(buzzerPin, LOW); // Turn off the buzzer
    Serial.println("Reset"); // Print "Reset" via Serial
    lcd_1.clear(); // Clear the LCD
    lcd_1.setCursor(0, 0);
    lcd_1.print("Resetting...");
    delay(6000); // Display message for 10 seconds
    isTriggered = false;
    i=false;
    normalLed = true;
  } else {
    Serial.println("Not Pressed");
  
    int xRaw = 0, yRaw = 0, zRaw = 0;
  for (int i = 0; i < samples; i++) {
    xRaw += analogRead(A0);
    yRaw += analogRead(A1);
    zRaw += analogRead(A2);
  }
  xRaw /= samples;
  yRaw /= samples;
  zRaw /= samples;

  float xVoltage = xRaw * (voltageReference / 1023.0);
  float yVoltage = yRaw * (voltageReference / 1023.0);
  float zVoltage = zRaw * (voltageReference / 1023.0);

  float xAcceleration = abs((xVoltage - zeroGVoltage) / sensitivity);
  float yAcceleration = abs((yVoltage - zeroGVoltage) / sensitivity);
  float zAcceleration = abs((zVoltage - zeroGVoltage) / sensitivity);

  // Print the X, Y, and Z acceleration values with labels
  Serial.print("X: ");
  Serial.print(xAcceleration, 4); // Print with 4 decimal places
  Serial.print(" g, Y: ");
  Serial.print(yAcceleration, 4);
  Serial.print(" g, Z: ");
  Serial.println(zAcceleration, 4);


  if (((xAcceleration > xThresholdLow || xAcceleration < xThresholdHigh) ||
       (yAcceleration > yThresholdLow || yAcceleration < yThresholdHigh))) {
        normalLed = false;
      digitalWrite(ledPin2, HIGH);
      digitalWrite(ledPin3, LOW);
      digitalWrite(ledPin4, LOW);
    if (!isBuzzerActive) {
      digitalWrite(buzzerPin, HIGH);
      isBuzzerActive = true;
      buzzerStartTime = currentTime; // Record the start time of the buzzer
    } 
    lastShakeTime = currentTime; // Reset the time whenever motion is detected
  } else {
    if (isBuzzerActive && (currentTime - lastShakeTime >= 5000)) {
      digitalWrite(buzzerPin, LOW);
      isBuzzerActive = false;
    }
    // normalLed = false;
  }

  if(normalLed){
    digitalWrite(ledPin4, HIGH);
    digitalWrite(ledPin2, LOW);
    digitalWrite(ledPin3, LOW);
  }

  if (!isBuzzerActive) {
    lcd_1.setCursor(0, 0);
    lcd_1.print("Status          ");
    lcd_1.setCursor(0, 1);
    lcd_1.print(" Stable              ");
    // isTriggered = false;
    i=false;
  } else {
    lcd_1.setCursor(0, 0);
    lcd_1.print("Status          ");
    lcd_1.setCursor(0, 1);
    lcd_1.print(" Earthquake!");
    isTriggered = true;
    i = true;

  }

  // Inserting the provided code snippet
  if (currentTime - buzzerStartTime >= 10000 && isTriggered && !i) {
    if (currentTime - lastChangeTime >= 1000) { // Increased delay to 1000 ms (1 second)
        lcd_1.setCursor(0, 0);
        lcd_1.print("Please get out");
        lcd_1.setCursor(0, 1);
        lcd_1.print("with caution!");
        // lcd_1.setBacklight(HIGH); // Turn on the LCD backlight

        digitalWrite(ledPin3, HIGH);
        digitalWrite(ledPin2, LOW);
        digitalWrite(ledPin4, LOW);

        if (isBuzzerActive) {
            digitalWrite(buzzerPin, LOW); // Turn off the buzzer
            isBuzzerActive = false;
        } else if (isTriggered && !isBuzzerActive) {
            digitalWrite(buzzerPin, HIGH); // Turn on the buzzer
            lcd_1.setBacklight(LOW);
            delay(600); // Adjusted delay to 1100 ms
            lcd_1.setBacklight(HIGH); // Turn on the LCD backlight
            delay(600); // Adjusted delay to 1100 ms
            isBuzzerActive = true;
        }

        lastChangeTime = currentTime;
    }
}

  }
  

}
