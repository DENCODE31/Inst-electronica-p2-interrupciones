
/*
 * =====================================================================
 *  EJERCICIO 3 — Entradas Analógicas A/D: Dos LDRs
 *  Materia  : Instrumentación Electrónica
 *  Placa    : ESP32 DevKit v1
 * =====================================================================
 *
 *  OBJETIVO:
 *    Leer dos LDRs mediante el ADC del ESP32, calcular la resistencia
 *    de cada sensor y determinar la posición del foco de luz.
 *    Activar un LED indicador según el lado de mayor iluminación.
 *
 *  PRINCIPIO — DIVISOR DE VOLTAJE:
 *
 *    3.3V
 *     │
 *    LDR  (resistencia variable según luz)
 *     │
 *     ├──── ADC (lee el voltaje aquí)
 *     │
 *    10kΩ  (resistencia fija)
 *     │
 *    GND
 *
 *    V_adc = 3.3V × R_fija / (R_LDR + R_fija)
 *    Despejando:
 *    R_LDR = R_fija × (3.3V / V_adc  - 1)
 *
 *    Más luz → menor R_LDR → mayor V_adc → ADC más alto
 *
 *  CONEXIÓN:
 *    LDR Izquierdo → GPIO 34  |  LED Izquierdo → GPIO 25 + 220Ω → GND
 *    LDR Derecho   → GPIO 35  |  LED Derecho   → GPIO 26 + 220Ω → GND
 *
 *  NOTA: GPIO 34 y 35 son solo entrada (input-only), ideales para ADC.
 *        El ESP32 usa ADC de 12 bits → rango 0 a 4095.
 *
 * =====================================================================
 */

#include <Arduino.h>

// ── Pines ─────────────────────────────────────────────────────────────
#define LDR_LEFT_PIN    34    // ADC1 canal 6 — LDR izquierdo
#define LDR_RIGHT_PIN   35    // ADC1 canal 7 — LDR derecho
#define LED_LEFT_PIN    27    // LED alarma izquierda
#define LED_RIGHT_PIN   26    // LED alarma derecha

// ── Constantes del divisor de voltaje ─────────────────────────────────
#define V_REF           3.3f  // Voltaje de referencia del ESP32 [V]
#define R_FIXED         10000.0f  // Resistencia fija del divisor [Ω]
#define ADC_RESOLUTION  4095  // Resolución 12 bits del ESP32

// ── Umbral de decisión ────────────────────────────────────────────────
// Si la diferencia de resistencias supera este valor, se activa el LED
#define THRESHOLD       1000.0f   // [Ω]


// =====================================================================
//  FUNCIÓN: Convierte lectura ADC a resistencia del LDR
// =====================================================================
float adcToResistance(int adcValue) {

    // Evita división por cero si el pin está en cortocircuito a GND
    if (adcValue <= 0) return 999999.0f;

    // Escala ADC → Voltaje
    float voltage = (adcValue / (float)ADC_RESOLUTION) * V_REF;

    // Evita división por cero si el voltaje es prácticamente cero
    if (voltage <= 0.0f) return 999999.0f;

    // Fórmula del divisor de voltaje despejada para R_LDR
    return R_FIXED * ((V_REF / voltage) - 1.0f);
}


// =====================================================================
//  SETUP
// =====================================================================
void setup() {

    Serial.begin(115200);

    // Fuerza resolución de 12 bits en el ADC del ESP32
    analogReadResolution(12);

    pinMode(LED_LEFT_PIN,  OUTPUT);
    pinMode(LED_RIGHT_PIN, OUTPUT);

    // Header para Serial Plotter (Tools → Serial Plotter)
    Serial.println("R_LDR_Izq[Ohm],R_LDR_Der[Ohm]");

    Serial.println("================================================");
    Serial.println(" EJ3: Lectura de dos LDRs con conversion A/D");
    Serial.println("================================================");
}


// =====================================================================
//  LOOP
// =====================================================================
void loop() {

    // 1. Leer valores crudos del ADC
    int rawLeft  = analogRead(LDR_LEFT_PIN);
    int rawRight = analogRead(LDR_RIGHT_PIN);

    // 2. Convertir ADC → Resistencia del LDR
    float rLeft  = adcToResistance(rawLeft)/4.2;
    float rRight = adcToResistance(rawRight);

    // 3. Calcular diferencia
    //    diff > 0  →  rLeft > rRight  →  más luz a la DERECHA
    //    diff < 0  →  rLeft < rRight  →  más luz a la IZQUIERDA
    float diff = rLeft - rRight;

    // 4. Alarma visual con LEDs
    if (diff > THRESHOLD) {
        // Más luz en la derecha: apaga LED izq, enciende LED der
        digitalWrite(LED_LEFT_PIN,  LOW);
        digitalWrite(LED_RIGHT_PIN, HIGH);

    } else if (diff < -THRESHOLD) {
        // Más luz en la izquierda: enciende LED izq, apaga LED der
        digitalWrite(LED_LEFT_PIN,  HIGH);
        digitalWrite(LED_RIGHT_PIN, LOW);

    } else {
        // Luz equilibrada o centrada: ambos LEDs encendidos
        digitalWrite(LED_LEFT_PIN,  HIGH);
        digitalWrite(LED_RIGHT_PIN, HIGH);
    }

    // 5. Imprimir en monitor serial
    Serial.print("ADC_Izq="); Serial.print(rawLeft);
    Serial.print(" | R_Izq="); Serial.print(rLeft, 1); Serial.print(" Ohm");
    Serial.print("   ||   ");
    Serial.print("ADC_Der="); Serial.print(rawRight);
    Serial.print(" | R_Der="); Serial.print(rRight, 1); Serial.print(" Ohm");

    if (diff > THRESHOLD)        Serial.println("  ->  Foco: DERECHA");
    else if (diff < -THRESHOLD)  Serial.println("  ->  Foco: IZQUIERDA");
    else                         Serial.println("  ->  Foco: CENTRO");

    delay(200);
}