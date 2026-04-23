/*
 * =====================================================================
 *  EJERCICIO 1 - Interrupciones con senal emulada
 *  Materia  : Instrumentacion Electronica
 *  Placa    : ESP32V3 (ESP32-S3)
 * =====================================================================
 *
 *  OBJETIVO:
 * El debounce por software tiene la ventaja de no requerir componentes adicionales. Resolvemos el rebote únicamente modificando el código de nuestro programa.
    Como desventaja, incrementa levemente el tiempo de ejecución y la complejidad del código. Además, si no aplicamos el código correctamente podemos ignorar interrupciones “verdaderas”.
    La forma más sencilla de aplicar un debounce por software es comprobar el tiempo entre disparos de la interrupción. Si el tiempo es inferior a un determinado umbral de tiempo (threshold) simplemente ignoramos la interrupción. En definitiva, hemos definido una “zona muerta” en la que ignoramos las interrupciones generadas.
    Para aplicar el debounce por software, modificamos la función ISR de la siguiente forma.
 *
 *
 *  ESQUEMA DE CONEXION:
 *
 *    PIN MAP ESP32V3:
 *
 *     --------GPIO 48
 *             Pulsador
 *
 *    LED integrado: GPIO 35
 *    OLED integrado: SDA GPIO 17, SCL GPIO 18, RST GPIO 21
 *

 *
 * =====================================================================
 */
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ── Pines OLED ESP32 WiFi LoRa 32 V3 ─────────────────────────────────────
#define OLED_SDA    17
#define OLED_SCL    18
#define OLED_RST    21
#define VEXT_PIN    36     // ← Habilita alimentación del display (activo LOW)
#define SCREEN_W   128
#define SCREEN_H    64

Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, OLED_RST);

// ── Configuración de interrupción ──────────────────────────────────────────
const int          TIME_THRESHOLD = 150;
const int          INT_PIN        = 48;

volatile int           isrCounter = 0;
volatile unsigned long startTime  = 0;
int counter = 0;

// ── ISR ────────────────────────────────────────────────────────────────────
void IRAM_ATTR debounceCount() {
  if (millis() - startTime > TIME_THRESHOLD) {
    isrCounter++;
    startTime = millis();
  }
}

// ── Función auxiliar: refresca pantalla ───────────────────────────────────
void updateDisplay(int count) {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Contador pulsos:");

  display.setTextSize(4);
  display.setCursor(20, 24);
  display.println(count);

  display.display();
}

// ── Setup ──────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  // Habilitar Vext ANTES de cualquier operación con el display
  pinMode(VEXT_PIN, OUTPUT);
  digitalWrite(VEXT_PIN, LOW);   // LOW = Vext ON en la Heltec V3
  delay(100);                    // Tiempo para que el rail estabilice

  // Reset manual del SSD1306
  Wire.begin(OLED_SDA, OLED_SCL);
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("ERROR: display SSD1306 no encontrado");
    while (true);
  }

  display.clearDisplay();
  display.display();
  updateDisplay(0);

  pinMode(INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INT_PIN), debounceCount, FALLING);
}

// ── Loop ───────────────────────────────────────────────────────────────────
void loop() {
  if (counter != isrCounter) {
    counter = isrCounter;
    Serial.println(counter);
    updateDisplay(counter);
  }
}