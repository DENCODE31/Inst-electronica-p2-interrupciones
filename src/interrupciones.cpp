/*
 * =====================================================================
 *  EJERCICIO 1 - Interrupciones con senal emulada
 *  Materia  : Instrumentacion Electronica
 *  Placa    : ESP32V3 (ESP32-S3)
 * =====================================================================
 *
 *  OBJETIVO:
 *    Emular una senal digital desde un pin de salida y detectarla con
 *    una interrupcion externa en otro pin. Cada cambio en la entrada
 *    alterna el estado del LED integrado de la placa y aumenta un
 *    contador mostrado en la pantalla OLED integrada.
 *
 *  EQUIVALENCIA CON EL EJEMPLO ORIGINAL:
 *    Arduino emuPin = 10  -> ESP32V3 GPIO 47
 *    Arduino intPin = 2   -> ESP32V3 GPIO 48
 *    Arduino LEDPin = 13  -> ESP32V3 LED integrado GPIO 35
 *
 *  ESQUEMA DE CONEXION:
 *
 *    PIN MAP ESP32V3:
 *
 *    GPIO 47 -------------- GPIO 48
 *    salida emulada         entrada de interrupcion
 *
 *    LED integrado: GPIO 35
 *    OLED integrado: SDA GPIO 17, SCL GPIO 18, RST GPIO 21
 *
 *  NOTAS:
 *    - Unimos fisicamente GPIO 47 con GPIO 48 usando un jumper.
 *    - No conectes GPIO 47 ni GPIO 48 a 5V; la ESP32V3 trabaja a 3.3V.
 *    - Se usa CHANGE para activar la interrupcion tanto en subida como
 *      en bajada, igual que en el codigo base.
 *
 * =====================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// =====================================================================
//  Pines adaptados para ESP32V3 WiFi V3
// =====================================================================
#define EMU_PIN   47  // Pin que genera la senal emulada.
#define LED_PIN   35  // Pin del LED integrado de la ESP32V3.
#define INT_PIN   48  // Pin que recibe la interrupcion desde EMU_PIN.
#define VEXT_PIN  36  // Pin que habilita la alimentacion externa/OLED.

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
//  Variables compartidas con la ISR; usan volatile porque cambian dentro
//  de la interrupcion y el loop debe leer siempre su valor mas reciente.
// =====================================================================
volatile bool          ledState            = LOW;   // Guarda el estado actual del LED.
volatile bool          displayNeedsUpdate  = true;  // Avisa al loop que debe actualizar la OLED.
volatile unsigned long interruptCount      = 0;     // Cuenta cuantas interrupciones han ocurrido.

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RST);

// =====================================================================
//  ISR: se ejecuta con cada cambio detectado en INT_PIN; alterna el LED,
//  suma una interrupcion al contador y marca la OLED para refrescarse.
// =====================================================================
void IRAM_ATTR blink() {
    ledState = !ledState;
    interruptCount++;
    displayNeedsUpdate = true;

    digitalWrite(LED_PIN, ledState);
}

// =====================================================================
//  Actualiza la pantalla OLED con el contador y el estado del LED
// =====================================================================
void updateDisplay(unsigned long count, bool ledOn) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    display.setTextSize(1);
    display.setCursor(2, 0);
    display.println("Interrupciones");
    

    display.setTextSize(2);
    display.setCursor(0, 26);
    display.print("Cnt: ");
    display.println(count);

    display.setTextSize(1);
    display.setCursor(0, 54);
    display.print("LED: ");
    display.println(ledOn ? "ON" : "OFF");

    display.display();
}

// =====================================================================
//  SETUP
// =====================================================================
void setup() {
    Serial.begin(115200);

    pinMode(EMU_PIN, OUTPUT);        // Configura GPIO47 como salida.
    pinMode(LED_PIN, OUTPUT);        // Configura el LED integrado como salida.
    pinMode(INT_PIN, INPUT_PULLUP);  // Configura GPIO48 como entrada con pull-up.
    pinMode(VEXT_PIN, OUTPUT);       // Configura el control de alimentacion OLED.

    digitalWrite(EMU_PIN, LOW);      // Inicia la senal emulada en estado bajo.
    digitalWrite(LED_PIN, LOW);      // Inicia el LED apagado.

    // Habilita la alimentacion externa/OLED en ESP32V3.
    digitalWrite(VEXT_PIN, LOW);
    delay(100);

    Wire.begin(OLED_SDA, OLED_SCL);

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println("No se detecto la pantalla OLED");
    } else {
        updateDisplay(0, ledState);
    }

    attachInterrupt(digitalPinToInterrupt(INT_PIN), blink, CHANGE);  // Ejecuta blink() en cada cambio de GPIO48.
}

// =====================================================================
//  LOOP
// =====================================================================
void loop() {
    static unsigned long lastToggle = 0;
    static bool          emuState   = LOW;

    // Emula una senal cuadrada para disparar la interrupcion.
    if (millis() - lastToggle >= 150) {  // Cambia el estado cada 150 ms.
        lastToggle = millis();       // Guarda el tiempo del ultimo cambio.
        emuState   = !emuState;      // Invierte la senal emulada HIGH/LOW.
        digitalWrite(EMU_PIN, emuState);
    }

    if (displayNeedsUpdate) {
        noInterrupts();

        unsigned long count = interruptCount;
        bool          ledOn = ledState;

        displayNeedsUpdate  = false;

        interrupts();

        Serial.print("Interrupciones: ");
        Serial.print(count);
        Serial.print(" | LED: ");
        Serial.println(ledOn ? "ON" : "OFF");

        updateDisplay(count, ledOn);
    }
}
