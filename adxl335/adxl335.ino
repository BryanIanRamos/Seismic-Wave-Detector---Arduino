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
const int buttonPinReset = 9; // Renamed to clarify its purpose
bool i = false;

// Motion (earthquake/seismic wave) set normal value for stable.
const float xThresholdLow = 3.0;    // Increase to reduce sensitivity along X-axis
const float xThresholdHigh = 2.1;   // Decrease to reduce sensitivity along X-axis
const float yThresholdLow = 3.0;    // Increase to reduce sensitivity along Y-axis
const float yThresholdHigh = 2.1;   // Decrease to reduce sensitivity along Y-axis

unsigned long lastShakeTime = 0;
unsigned long buzzerStartTime = 0; 
bool isBuzzerActive = false;
bool isTriggered = false;
bool normalLed = true;
unsigned long lastChangeTime = 0;

// Setting up all the ButtonPins, Buzzer, and LEDPins
void setup() {
  lcd_1.init();
  lcd_1.backlight();
  Serial.begin(9600);
  pinMode(buttonPinReset, INPUT_PULLUP); // Renamed for clarity
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

  // Check if the reset button is pressed
  if (digitalRead(buttonPinReset) == HIGH) { // Renamed for clarity
    Serial.println("Pressed");
    digitalWrite(buzzerPin, LOW); 
    Serial.println("Reset"); 
    lcd_1.clear(); 
    lcd_1.setCursor(0, 0);
    lcd_1.print("Resetting...");
    delay(6000); 
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

    // Reference voltage of all the data received from sensor X, Y, Z axis
    float xVoltage = xRaw * (voltageReference / 1023.0);
    float yVoltage = yRaw * (voltageReference / 1023.0);
    float zVoltage = zRaw * (voltageReference / 1023.0);

    float xAcceleration = abs((xVoltage - zeroGVoltage) / sensitivity);
    float yAcceleration = abs((yVoltage - zeroGVoltage) / sensitivity);
    float zAcceleration = abs((zVoltage - zeroGVoltage) / sensitivity);

    // Display the seismic wave intensity data on the serial
    Serial.print("X: ");
    Serial.print(xAcceleration, 4); 
    Serial.print(" g, Y: ");
    Serial.print(yAcceleration, 4);
    Serial.print(" g, Z: ");
    Serial.println(zAcceleration, 4);


    // Check if the Earthquake or seismic wave triggers the sensor standard value
    if (((xAcceleration > xThresholdLow || xAcceleration < xThresholdHigh) ||
         (yAcceleration > yThresholdLow || yAcceleration < yThresholdHigh))) {
      normalLed = false;
      digitalWrite(ledPin2, HIGH);
      digitalWrite(ledPin3, LOW);
      digitalWrite(ledPin4, LOW);
      if (!isBuzzerActive) {
        digitalWrite(buzzerPin, HIGH);
        isBuzzerActive = true;
        buzzerStartTime = currentTime; 
      } 
      lastShakeTime = currentTime; 
    } else {
      if (isBuzzerActive && (currentTime - lastShakeTime >= 5000)) {
        digitalWrite(buzzerPin, LOW);
        isBuzzerActive = false;
      }
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
      i=false;
    } else {
      lcd_1.setCursor(0, 0);
      lcd_1.print("Status          ");
      lcd_1.setCursor(0, 1);
      lcd_1.print(" Earthquake!");
      isTriggered = true;
      i = true;
    }

    // Block of code responsible for informing students to get out in the building
    if (currentTime - buzzerStartTime >= 10000 && isTriggered && !i) {
      if (currentTime - lastChangeTime >= 1000) { 
        lcd_1.setCursor(0, 0);
        lcd_1.print("Please get out");
        lcd_1.setCursor(0, 1);
        lcd_1.print("with caution!");
        digitalWrite(ledPin3, HIGH);
        digitalWrite(ledPin2, LOW);
        digitalWrite(ledPin4, LOW);
        if (isBuzzerActive) {
          digitalWrite(buzzerPin, LOW); 
          isBuzzerActive = false;
        } else if (isTriggered && !isBuzzerActive) {
          digitalWrite(buzzerPin, HIGH); 
          lcd_1.setBacklight(LOW);
          delay(600); 
          lcd_1.setBacklight(HIGH); 
          delay(600); 
          isBuzzerActive = true;
        }
        lastChangeTime = currentTime;
      }
    }
  }
}
