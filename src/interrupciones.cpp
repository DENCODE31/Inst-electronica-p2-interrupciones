#include <Arduino.h>
// Pin definitions
const int interruptPin = 2;  // Interrupt pin (0 or 1 on most Arduino boards)
const int ledPin = 13;       // LED pin

// Volatile variable to track interrupt state
volatile int interruptCount = 0;

// Interrupt Service Routine (ISR)
void handleInterrupt() {
    interruptCount++;
    digitalWrite(ledPin, !digitalRead(ledPin));  // Toggle LED
}

void setup() {
    Serial.begin(9600);
    pinMode(ledPin, OUTPUT);
    pinMode(interruptPin, INPUT_PULLUP);
    
    // Attach interrupt to pin 2, trigger on FALLING edge
    attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING);
}

void loop() {
    Serial.print("Interrupt count: ");
    Serial.println(interruptCount);
    delay(1000);
}