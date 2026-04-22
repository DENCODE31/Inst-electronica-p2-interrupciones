/*
 * =====================================================================
 *  EJERCICIO 2 — Timers por Interrupciones
 *  Materia  : Instrumentación Electrónica
 *  Placa    : ESP32 DevKit v1
 * =====================================================================
 *
 *  OBJETIVO:
 *    Medir el tiempo transcurrido entre dos pulsaciones consecutivas
 *    del botón (con antirrebote) y mostrarlo en el monitor serial.
 *
 *  FUNCIONAMIENTO:
 *    En cada pulsación válida la ISR registra el timestamp con millis().
 *    Al detectar la segunda pulsación calcula la diferencia de tiempo
 *    y la envía al loop() mediante una variable compartida.
 *
 *  CONEXIÓN:
 *    Mismo circuito del EJ1:
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
#define BUTTON_PIN   18

// ── Parámetros ────────────────────────────────────────────────────────
#define DEBOUNCE_MS  50       // Umbral de antirrebote software [ms]

// ── Variables compartidas (ISR ↔ loop) ────────────────────────────────
volatile unsigned long lastPressTime = 0;   // Timestamp del pulso anterior
volatile unsigned long interval_ms   = 0;   // Intervalo calculado
volatile bool          newInterval   = false; // Bandera: hay dato nuevo


// =====================================================================
//  ISR — Mide el tiempo entre pulsaciones
// =====================================================================
void IRAM_ATTR onButtonPress() {

    unsigned long currentTime = millis();

    // Antirrebote software
    if ((currentTime - lastPressTime) >= DEBOUNCE_MS) {

        // Solo calcula intervalo a partir de la segunda pulsación
        if (lastPressTime != 0) {
            interval_ms  = currentTime - lastPressTime;
            newInterval  = true;    // Avisa al loop() que hay un dato nuevo
        }

        lastPressTime = currentTime;
    }
}


// =====================================================================
//  SETUP
// =====================================================================
void setup() {

    Serial.begin(115200);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    attachInterrupt(
        digitalPinToInterrupt(BUTTON_PIN),
        onButtonPress,
        FALLING
    );

    Serial.println("================================================");
    Serial.println(" EJ2: Tiempo entre pulsos");
    Serial.println(" Presiona el boton dos veces para medir.");
    Serial.println("================================================");
}


// =====================================================================
//  LOOP
// =====================================================================
void loop() {

    // Solo actúa cuando la ISR reportó un intervalo nuevo
    if (newInterval) {

        // Sección crítica: lectura atómica de variables volatile
        noInterrupts();
            unsigned long dt = interval_ms;
            newInterval      = false;
        interrupts();

        // Imprime en milisegundos y segundos
        Serial.print("Intervalo entre pulsos: ");
        Serial.print(dt);
        Serial.print(" ms   (");
        Serial.print(dt / 1000.0, 3);
        Serial.println(" s)");
    }
}