/*
 *  Heat Plate Controller 16x2 I2C v1.2
 *  Microcontroller: ATmega328P / ATmega8
 *  ---------------------------------------
 *  Developed by: Javier Rayhan
 *  Date        : 28 April 2025
 *  
 *  Project Description:
 *  This project controls the heating process using a microcontroller,
 *  allowing precise temperature management for a heat plate system.
 *  It can be used in DIY soldering stations, 3D printer heated beds, or lab tools.
 * 
 *  Contact & Portfolio:
 *  Instagram : @zv.cpp
 *  GitHub    : github.com/javieryql
 *  LinkedIn  : linkedin.com/in/javierrayhan
 * 
 *  © 2025 Javier Rayhan. All rights reserved.
 */

#include <thermistor.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// === LCD Setup ===
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Address 0x27, 16 columns, 2 rows

// === Thermistor Setup ===
const float BETA = 3950; // Beta Coefficient for 10k Thermistor (cek datasheet kalo beda)

// === Encoder Pins ===
#define encoderPin1 2
#define encoderPin2 3
#define encoderSwitchPin 6

volatile uint8_t lastEncoded = 0;
volatile uint16_t encoderValue = 40;
int setTemperature = 40;
bool heatingStatus = false;
bool lastButtonState = HIGH;

const uint8_t sampleCount = 40;

unsigned long previousMillisTempStatus = 0;
unsigned long previousMillisSetTemp = 0;
const unsigned long intervalTempStatus = 800;   
const unsigned long intervalSetTemp = 10;    

// === Relay ===
#define RELAY_PIN 7

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  pinMode(encoderPin1, INPUT_PULLUP);
  pinMode(encoderPin2, INPUT_PULLUP);
  pinMode(encoderSwitchPin, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(encoderPin1), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPin2), updateEncoder, CHANGE);

  Wire.begin();
  lcd.init();  // Initialize the LCD with 16 columns and 2 rows
  lcd.setBacklight(1); // Turn on backlight
  lcd.setCursor(0, 0);
  lcd.print("Heat Plate");
  lcd.setCursor(0, 1);
  lcd.print("Controller v1.2");
  
  delay(1500);
  lcd.clear();

  lcd.setCursor(3, 0);
  lcd.print("by @zv.cpp");

  delay(1000);
  lcd.clear();
}

void loop() {
  unsigned long currentMillis = millis();
  // === Average Temperature ===
  float suhuRataRata = getAverageTemp(sampleCount);

  bool currentButtonState = digitalRead(encoderSwitchPin);

  if (lastButtonState == HIGH && currentButtonState == LOW) {
    heatingStatus = !heatingStatus;

    if (!heatingStatus) {
      digitalWrite(RELAY_PIN, LOW);  
    }
    delay(20); 
  }

  lastButtonState = currentButtonState;


  if (heatingStatus) {
    if (suhuRataRata < (setTemperature-6)) {
      digitalWrite(RELAY_PIN, HIGH);
    } else {
      digitalWrite(RELAY_PIN, LOW);
    }
  }

  setTemperature = encoderValue;
  
// === Update Current Temp and Heating Status === 
  if (currentMillis - previousMillisTempStatus >= intervalTempStatus) {
    previousMillisTempStatus = currentMillis;

    lcd.setCursor(0, 0);
    lcd.print("Set: ");
    lcd.print(setTemperature);
    lcd.print("C ");

    lcd.setCursor(10, 0);
    lcd.print(heatingStatus ? "[HEAT]" : "[IDLE]");

    lcd.setCursor(0, 1);
    lcd.print("Curr: ");
    lcd.print(suhuRataRata, 1);
    lcd.print("C ");
  }

  // === Update Set Temp 200 milisecond ===
  if (currentMillis - previousMillisSetTemp >= intervalSetTemp) {
    previousMillisSetTemp = currentMillis;

    lcd.setCursor(0, 0);
    lcd.print("Set: ");
    lcd.print(setTemperature);
    lcd.print("C ");
  }
}

// === Function to get average temperature using thermistor ===
float getAverageTemp(uint8_t jumlahSampel) {
  float total = 0;
  for (uint8_t i = 0; i < jumlahSampel; i++) {
    total += analog2temp(A0);  
  }
  // delay(10);
  return (total / jumlahSampel) - 3.5;
}

// === Function to convert analog value to temperature ===
float analog2temp(int analogPin) {
  int analogValue = analogRead(analogPin);
  float celsius = 1 / (log(1 / (1023. / analogValue - 1)) / BETA + 1.0 / 298.15) - 273.15;
  return celsius;
}

// === Encoder Interrupt ===
void updateEncoder() {
  int MSB = digitalRead(encoderPin1);  // Most Significant Bit
  int LSB = digitalRead(encoderPin2);  // Least Significant Bit

  int encoded = (MSB << 1) | LSB;
  int sum = (lastEncoded << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue++;  // clockwise
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue--;  // counter-clockwise

  if (encoderValue < 40) encoderValue = 40;
  if (encoderValue > 250) encoderValue = 250;

  lastEncoded = encoded;
}
