/*
 * =====================================================================
 *  EJERCICIO 4 — Salidas Analógicas PWM: Servo por Posición de Luz
 *  Materia  : Instrumentación Electrónica
 *  Placa    : ESP32 DevKit v1
 * =====================================================================
 *
 *  OBJETIVO:
 *    Controlar la posición de un servo de 0° a 180° según la dirección
 *    del foco de luz detectada por dos LDRs (integra el EJ3).
 *    Además mostrar los valores de resistencia y ángulo en el serial.
 *
 *  LÓGICA DE MAPEO:
 *
 *    Toda la luz a la IZQUIERDA  →  servo a   0°
 *    Luz centrada (equilibrada)  →  servo a  90°
 *    Toda la luz a la DERECHA    →  servo a 180°
 *
 *    Se calcula el ratio:  ratio = R_Der / (R_Izq + R_Der)
 *    Si hay más luz a la derecha → R_Der baja → ratio baja → ángulo bajo
 *
 *  PWM en ESP32:
 *    El ESP32 no usa analogWrite() nativo para servos.
 *    Se usa la librería ESP32Servo que gestiona los canales LEDC.
 *    Pulso SG90: 500µs (0°) a 2400µs (180°) @ 50Hz
 *
 *  CONEXIÓN:
 *    LDR Izquierdo → GPIO 34  |  LED Izquierdo → GPIO 25 + 220Ω → GND
 *    LDR Derecho   → GPIO 35  |  LED Derecho   → GPIO 26 + 220Ω → GND
 *    Servo señal   → GPIO 18
 *    Servo VCC     → 5V (fuente externa recomendada, GND común)
 *
 *  LIBRERÍA REQUERIDA:
 *    ESP32Servo by Kevin Harrington (madhephaestus/ESP32Servo)
 *    Instalar desde PlatformIO Library Manager o platformio.ini:
 *      lib_deps = madhephaestus/ESP32Servo @ ^0.13.0
 *
 * =====================================================================
 */

#include <Arduino.h>
#include <ESP32Servo.h>

// ── Pines ─────────────────────────────────────────────────────────────
#define LDR_LEFT_PIN    34
#define LDR_RIGHT_PIN   35
#define LED_LEFT_PIN    27
#define LED_RIGHT_PIN   26
#define SERVO_PIN       13

// ── Constantes del divisor de voltaje ─────────────────────────────────
#define V_REF           3.3f
#define R_FIXED         10000.0f
#define ADC_RESOLUTION  4095

// ── Umbral para alarma de LEDs ────────────────────────────────────────
#define THRESHOLD       1000.0f   // [Ω]

// ── Objeto servo ──────────────────────────────────────────────────────
Servo myServo;


// =====================================================================
//  FUNCIÓN: Convierte lectura ADC a resistencia del LDR
// =====================================================================
float adcToResistance(int adcValue) {

    if (adcValue <= 0)    return 999999.0f;

    float voltage = (adcValue / (float)ADC_RESOLUTION) * V_REF;

    if (voltage <= 0.0f)  return 999999.0f;

    return R_FIXED * ((V_REF / voltage) - 1.0f);
}


// =====================================================================
//  SETUP
// =====================================================================
void setup() {

    Serial.begin(115200);
    analogReadResolution(12);

    pinMode(LED_LEFT_PIN,  OUTPUT);
    pinMode(LED_RIGHT_PIN, OUTPUT);

    // Configura el servo con los pulsos del SG90
    // attach(pin, pulsoMin_us, pulsoMax_us)
    myServo.attach(SERVO_PIN, 500, 2400);
    myServo.write(90);    // Posición inicial: centro

    Serial.println("================================================");
    Serial.println(" EJ4: Servo controlado por posicion de luz");
    Serial.println("================================================");
    Serial.println("R_Izq[Ohm],R_Der[Ohm],Angulo[deg]");
}


// =====================================================================
//  LOOP
// =====================================================================
void loop() {

    // 1. Leer ADC
    int rawLeft  = analogRead(LDR_LEFT_PIN);
    int rawRight = analogRead(LDR_RIGHT_PIN);

    // 2. Convertir a resistencia
    float rLeft  = adcToResistance(rawLeft)/4;
    float rRight = adcToResistance(rawRight);

    // 3. Calcular ángulo del servo
    //    ratio = R_Der / (R_Izq + R_Der)
    //    ratio alto  → R_Der grande → LDR der oscuro → luz a la izquierda → ángulo alto
    //    ratio bajo  → R_Der chico  → LDR der iluminado → luz a la derecha → ángulo bajo
    float rSum    = rLeft + rRight;
    float ratio   = (rSum > 0.0f) ? (rRight / rSum) : 0.5f;
    int   angle   = constrain((int)(ratio * 180.0f), 0, 180);

    // 4. Mover servo
    myServo.write(angle);

    // 5. Alarma visual con LEDs (igual que EJ3)
    float diff = rLeft - rRight;

    if (diff > THRESHOLD) {
        digitalWrite(LED_LEFT_PIN,  LOW);
        digitalWrite(LED_RIGHT_PIN, HIGH);

    } else if (diff < -THRESHOLD) {
        digitalWrite(LED_LEFT_PIN,  HIGH);
        digitalWrite(LED_RIGHT_PIN, LOW);

    } else {
        digitalWrite(LED_LEFT_PIN,  HIGH);
        digitalWrite(LED_RIGHT_PIN, HIGH);
    }

    // 6. Monitor serial
    Serial.print("R_Izq=");  Serial.print(rLeft,  1); Serial.print(" Ohm");
    Serial.print(" | R_Der="); Serial.print(rRight, 1); Serial.print(" Ohm");
    Serial.print(" | Servo="); Serial.print(angle);    Serial.println(" deg");

    // 7. Serial Plotter — descomentar para graficar los tres valores
    // Serial.print(rLeft);   Serial.print(",");
    // Serial.print(rRight);  Serial.print(",");
    // Serial.println(angle);

    delay(100);
}