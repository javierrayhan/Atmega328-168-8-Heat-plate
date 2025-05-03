/*
 *  Heat Plate Controller OLED v1.1
 *  Microcontroller: ATmega328P / ATmega8
 *  ---------------------------------------
 *  Developed by: Javier Rayhan
 *  Date        : 11 April 2025
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
#include <SSD1306Ascii.h>
#include <SSD1306AsciiWire.h>

// === OLED Setup ===
SSD1306AsciiWire oled;

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

const uint8_t sampleCount = 40;

// === Relay ===
#define RELAY_PIN 7

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);

  pinMode(encoderPin1, INPUT_PULLUP);
  pinMode(encoderPin2, INPUT_PULLUP);
  pinMode(encoderSwitchPin, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(encoderPin1), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPin2), updateEncoder, CHANGE);

  Wire.begin();
  oled.begin(&Adafruit128x64, 0x3C);
  oled.setFont(System5x7);

  oled.clear();
  oled.println("  Heat Plate");
  oled.println("  Controller");
  oled.println();
  oled.println("   v1.1");
  oled.println("Ext Technologies");

  delay(1500);
}

void loop() {
  // === Average Temperature ===
  float suhuRataRata = getAverageTemp(sampleCount);

  if (digitalRead(encoderSwitchPin) == LOW) {
    if (!heatingStatus) {
      heatingStatus = true;
    } else {
      heatingStatus = false;
      digitalWrite(RELAY_PIN, HIGH);  // turn off relay
    }
    delay(100);  // debounce
  }

  if (heatingStatus) {
    if (suhuRataRata < setTemperature) {
      digitalWrite(RELAY_PIN,LOW);
    } else {
      digitalWrite(RELAY_PIN, HIGH);
    }
  }

  setTemperature = encoderValue;
  
  // === OLED ===
  oled.setCursor(0, 0);
  oled.print("Set Temp: ");
  oled.print(setTemperature);
  oled.print((char)248); 
  oled.println("C   ");

  oled.setCursor(0, 1);
  oled.print("Current: ");
  oled.print(suhuRataRata, 1);
  oled.print((char)248);
  oled.println("C   ");

  oled.setCursor(0, 2);
  oled.print("Status: ");
  oled.println(heatingStatus ? "HEATING " : "IDLE    ");
}

// === Function to get average temperature using thermistor ===
float getAverageTemp(uint8_t jumlahSampel) {
  float total = 0;
  for (uint8_t i = 0; i < jumlahSampel; i++) {
    total += analog2temp(A0);  
  }
  delay(10);
  return (total / jumlahSampel)-6;
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
