/*
 * =====================================================================
 *  EJERCICIO 1 — Interrupciones y Antirrebote
 *  Materia  : Instrumentación Electrónica
 *  Placa    : ESP32 DevKit v1
 * =====================================================================
 *
 *  OBJETIVO:
 *    Contar pulsos de un botón usando interrupciones externas.
 *    Se aplica antirrebote por HARDWARE y por SOFTWARE para evitar
 *    conteos falsos causados por el rebote mecánico del botón.
 *
 *  ANTIRREBOTE HARDWARE:
 *    Condensador de 100nF entre el pin del botón y GND.
 *    Filtra los picos de voltaje generados por el rebote mecánico.
 *
 *  ANTIRREBOTE SOFTWARE:
 *    Dentro de la ISR se verifica que haya pasado al menos
 *    DEBOUNCE_MS milisegundos desde el último pulso válido.
 *
 *  ESQUEMA DE CONEXIÓN:
 *
 *    3.3V ── 10kΩ ──┬── GPIO 18
 *                   │
 *                 Botón ── 100nF ── GND
 *                   │
 *                  GND
 *
 * =====================================================================
 */

#include <Arduino.h>

// ── Pines ─────────────────────────────────────────────────────────────
#define BUTTON_PIN   18       // GPIO con soporte de interrupción externa

// ── Parámetros ────────────────────────────────────────────────────────
#define DEBOUNCE_MS  50       // Tiempo mínimo entre pulsos válidos [ms]

// ── Variables compartidas entre ISR y loop() ──────────────────────────
// "volatile" indica que pueden cambiar en cualquier momento (en la ISR)
// y evita que el compilador las optimice incorrectamente.
volatile unsigned long pulseCount       = 0;
volatile unsigned long lastDebounceTime = 0;


// =====================================================================
//  ISR — Rutina de Servicio de Interrupción
//  Se ejecuta automáticamente en cada flanco de bajada del botón.
//  Debe ser corta y rápida: sin Serial, sin delay, sin malloc.
// =====================================================================
void IRAM_ATTR onButtonPress() {

    unsigned long currentTime = millis();

    // Antirrebote software: solo cuenta si pasó el tiempo mínimo
    if ((currentTime - lastDebounceTime) >= DEBOUNCE_MS) {
        pulseCount++;
        lastDebounceTime = currentTime;
    }
}


// =====================================================================
//  SETUP
// =====================================================================
void setup() {

    Serial.begin(115200);

    // Pull-up interno: pin en HIGH en reposo, LOW al presionar el botón
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // Registra la ISR en flanco de bajada (HIGH → LOW)
    attachInterrupt(
        digitalPinToInterrupt(BUTTON_PIN),  // número de interrupción del GPIO
        onButtonPress,                       // función a ejecutar
        FALLING                              // tipo de flanco
    );

    Serial.println("================================================");
    Serial.println(" EJ1: Conteo de pulsos con antirrebote");
    Serial.println(" Presiona el boton y observa el conteo.");
    Serial.println("================================================");
}


// =====================================================================
//  LOOP
// =====================================================================
void loop() {

    static unsigned long lastPrint = 0;

    if (millis() - lastPrint >= 200) {
        lastPrint = millis();

        // Sección crítica: deshabilita interrupciones para leer
        // la variable volatile de forma segura (operación atómica)
        noInterrupts();
            unsigned long count = pulseCount;
        interrupts();

        Serial.print("Pulsos contados: ");
        Serial.println(count);
    }
}