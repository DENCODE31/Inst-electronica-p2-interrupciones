#include <Arduino.h>

// Example: Using Timer 1 for PWM on Arduino Uno (pins 9 and 10)
// This sets up PWM with a frequency and duty cycle

void setup() {
    // Configure Timer 1 for PWM mode
    TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);  // Fast PWM, non-inverting
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);     // Prescaler 8, Fast PWM
    ICR1 = 19999;  // Top value for 50Hz PWM (16MHz / (8 * (19999 + 1)) = 50Hz)
    
    // Set duty cycle (e.g., 1ms pulse for servo)
    OCR1A = 1999;  // Duty cycle for pin 9 (10% for 1ms at 50Hz)
    OCR1B = 1999;  // Duty cycle for pin 10
    
    pinMode(9, OUTPUT);
    pinMode(10, OUTPUT);
}

void loop() {
    // Adjust duty cycle in loop if needed
    // OCR1A = value;  // Change value for PWM on pin 9
}