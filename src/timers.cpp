/*
 * =====================================================================
 *  EJERCICIO 2 - Medidor de tiempo entre pulsos con interrupciones
 *  Materia  : Instrumentacion Electronica
 *  Placa    : Heltec WiFi LoRa 32 V3 (ESP32-S3)
 * =====================================================================
 *
 *  OBJETIVO:
 *    Medir el tiempo entre pulsos externos usando interrupciones.
 *    El resultado se muestra en el Monitor Serial y en la pantalla OLED
 *    integrada de la Heltec.
 *
 *  ESQUEMA DE CONEXION:
 *
 *    Boton:
 *
 *    GPIO 48 -------------- Boton -------------- GND
 *
 *    OLED integrado: SDA GPIO 17, SCL GPIO 18, RST GPIO 21
 *
 *  NOTAS:
 *    - Se usa INPUT_PULLUP para que el pin permanezca en HIGH sin pulsar.
 *    - La interrupcion se activa en FALLING: HIGH -> LOW al presionar.
 *    - El debounce por software acepta un pulso solo si ya pasaron
 *      150 ms desde el ultimo pulso valido.
 *    - No uses 5V en los GPIO de la Heltec; trabaja con 3.3V.
 *
 * =====================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// =====================================================================
//  Pines adaptados para Heltec WiFi LoRa 32 V3
// =====================================================================
const int intPin = 48;  // Pin de entrada que recibe el pulso del boton.
#define VEXT_PIN  36    // Pin que habilita la alimentacion externa/OLED.

// =====================================================================
//  Pantalla OLED integrada
// =====================================================================
#define OLED_WIDTH   128
#define OLED_HEIGHT  64
#define OLED_ADDR    0x3C
#define OLED_SDA     17
#define OLED_SCL     18
#define OLED_RST     21

// =====================================================================
//  Parametros de medicion
// =====================================================================
const int timeThreshold = 200;  // Debounce: ignora rebotes menores a 200 ms.

// =====================================================================
//  Variables compartidas con la ISR; usan volatile porque cambian dentro
//  de la interrupcion y el loop debe leer siempre su valor mas reciente.
// =====================================================================
volatile unsigned long lastPulseTime = 0;      // Guarda el tiempo del pulso valido anterior.
volatile unsigned long currentTime   = 0;      // Guarda el tiempo del pulso actual.
volatile unsigned long interval      = 0;      // Guarda el intervalo entre dos pulsos.
volatile int           ISRCounter    = 0;      // Cuenta pulsos validos dentro de la ISR.
volatile bool          newData       = false;  // Avisa al loop que hay una nueva medicion.

int  counter   = 0;  // Copia local del contador para detectar cambios en loop().
long startTime = 0;  // Guarda el ultimo pulso aceptado para debounce.

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RST);

// =====================================================================
//  Muestra los campos de medicion iniciales en la pantalla OLED
// =====================================================================
void showTestInstructions() {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);

    display.setCursor(0, 0);
    display.println("Tiempo entre pulsos:");

    display.setTextSize(2);
    display.setCursor(0, 14);
    display.println("-- us");

    display.setTextSize(1);
    display.setCursor(0, 38);
    display.println("Frecuencia estimada:");

    display.setTextSize(2);
    display.setCursor(0, 50);
    display.println("-- Hz");

    display.display();
}

// =====================================================================
//  ISR: aplica debounce por software; si pasaron 150 ms desde el ultimo
//  pulso valido, aumenta el contador y calcula el intervalo con micros().
// =====================================================================
void IRAM_ATTR debounceCount() {
    unsigned long nowMs = millis();

    if (nowMs - startTime > timeThreshold) {
        currentTime = micros();

        if (lastPulseTime != 0) {
            interval = currentTime - lastPulseTime;
            newData  = true;
        }

        lastPulseTime = currentTime;
        ISRCounter++;
        startTime = nowMs;
    }
}

// =====================================================================
//  Actualiza la pantalla OLED
// =====================================================================
void updateDisplay(unsigned long localInterval, float frequency) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Tiempo entre pulsos:");

    display.setTextSize(2);
    display.setCursor(0, 12);
    display.print(localInterval);
    display.print(" us");

    display.setTextSize(1);
    display.setCursor(0, 36);
    display.println("Frecuencia estimada:");

    display.setTextSize(2);
    display.setCursor(0, 48);
    display.print(frequency, 2);
    display.print(" Hz");

    display.display();
}

// =====================================================================
//  Imprime en Serial 
// =====================================================================
void printMeasurement(unsigned long localInterval, float frequency) {
    Serial.print("Tiempo entre pulsos: ");
    Serial.print(localInterval);
    Serial.println(" us");

    Serial.print("Frecuencia estimada: ");
    Serial.print(frequency, 2);
    Serial.println(" Hz");
}

// =====================================================================
//  SETUP
// =====================================================================
void setup() {
    Serial.begin(115200);

    pinMode(intPin, INPUT_PULLUP);  // Configura GPIO48 como entrada con pull-up.
    pinMode(VEXT_PIN, OUTPUT);      // Configura el control de alimentacion OLED.

    // Habilita la alimentacion externa/OLED en Heltec.
    digitalWrite(VEXT_PIN, LOW);
    delay(100);

    Wire.begin(OLED_SDA, OLED_SCL);

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println("No se detecto la pantalla OLED");
    } else {
        showTestInstructions();
    }

    attachInterrupt(digitalPinToInterrupt(intPin), debounceCount, FALLING);  // Ejecuta la ISR al presionar.

    Serial.println("================================================");
    Serial.println(" EJ2: Tiempo entre pulsos con interrupciones");
    Serial.println(" Prueba:");
    Serial.println(" 1. Abre el Monitor Serial a 115200 baudios.");
    Serial.println(" 2. Presiona el boton varias veces.");
    Serial.println(" 3. Veras el tiempo entre pulsos y la frecuencia.");
    Serial.println(" Presiona el boton conectado entre GPIO48 y GND.");
    Serial.println(" Ejemplo:");
    Serial.println(" Tiempo entre pulsos: 240123 us");
    Serial.println(" Frecuencia estimada: 4.16 Hz");
    Serial.println("================================================");
}

// =====================================================================
//  LOOP
// =====================================================================
void loop() {
    if (counter != ISRCounter) {
        noInterrupts();  // Pausa interrupciones para copiar datos sin cambios inesperados.

        unsigned long localInterval = interval;    // Copia el tiempo entre pulsos calculado en la ISR.
        int           localCounter  = ISRCounter;  // Copia el contador de pulsos validos.
        bool          hasNewData    = newData;     // Copia si existe una medicion nueva para mostrar.

        newData = false;  // Limpia la bandera para esperar la siguiente medicion.

        interrupts();  // Reactiva las interrupciones despues de copiar los datos.

        counter = localCounter;  // Actualiza el contador local usado por el loop.

        if (hasNewData) {  // Solo muestra resultados cuando ya hay intervalo valido entre dos pulsos.
            float frequency = 1000000.0 / localInterval;  // Calcula la frecuencia en Hz desde microsegundos.

            printMeasurement(localInterval, frequency);
            updateDisplay(localInterval, frequency);
        }
    }
}
