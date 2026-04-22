
/*
 * =====================================================================
 *  EJERCICIO 3 — Entradas Analógicas A/D: Dos LDRs
 *  Materia  : Instrumentación Electrónica
 *  Placa    : ESP32 DevKit v1
 * git add .
git commit -m "Se agrega código para lectura de dos LDRs con ADC, cálculo de resistencia y posición de luz, y control de LEDs indicadores."
git push
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
// ==================== DEFINICIÓN DE PINES ====================
const int LDR1_PIN = A0;    // LDR 1 conectado a A0
const int LDR2_PIN = A1;    // LDR 2 conectado a A1
const int LED1_PIN = 13;    // LED indicador LDR 1
const int LED2_PIN = 12;    // LED indicador LDR 2

// ==================== CONSTANTES DE CALIBRACIÓN ====================
const float VCC = 5.0;           // Voltaje de alimentación
const float R_FIXED = 10000.0;   // Resistencia fija en divisor (10kΩ)
const int NUM_SAMPLES = 10;      // Número de muestras para promediar

// ==================== VARIABLES GLOBALES ====================
float resistance_LDR1 = 0;
float resistance_LDR2 = 0;
float light_position = 0;        // -100 a 100: negativo=más luz en LDR1, positivo=más luz en LDR2

// ==================== FUNCIÓN SETUP ====================
void setup() 
{
    Serial.begin(9600);
    pinMode(LED1_PIN, OUTPUT);
    pinMode(LED2_PIN, OUTPUT);
    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, LOW);
    
    delay(1000);
    Serial.println("=== Sistema de Detección de Posición de Luz ===");
    Serial.println("Iniciando lectura de sensores LDR...\n");
}

// ==================== FUNCIÓN LOOP PRINCIPAL ====================
void loop() 
{
    // Leer y promediar valores analógicos
    int raw_LDR1 = readAverageSensor(LDR1_PIN);
    int raw_LDR2 = readAverageSensor(LDR2_PIN);
    
    // Convertir a voltaje
    float voltage_LDR1 = (raw_LDR1 / 1023.0) * VCC;
    float voltage_LDR2 = (raw_LDR2 / 1023.0) * VCC;
    
    // Calcular resistencias usando divisor de voltaje: Vout = VCC * R_LDR / (R_fixed + R_LDR)
    resistance_LDR1 = calculateResistance(voltage_LDR1);
    resistance_LDR2 = calculateResistance(voltage_LDR2);
    
    // Calcular posición relativa del foco de luz (-100 a 100)
    light_position = calculateLightPosition(resistance_LDR1, resistance_LDR2);
    
    // Controlar LEDs según posición
    controlLEDs(light_position);
    
    // Enviar datos al monitor serial
    printSerialData(raw_LDR1, raw_LDR2, resistance_LDR1, resistance_LDR2, light_position);
    
    delay(500);  // Actualizar cada 500ms
}

// ==================== FUNCIÓN: LEER Y PROMEDIAR SENSOR ====================
int readAverageSensor(int pin) 
{
    long sum = 0;
    
    for (int i = 0; i < NUM_SAMPLES; i++) 
    {
        sum += analogRead(pin);
        delay(10);
    }
    
    return sum / NUM_SAMPLES;
}

// ==================== FUNCIÓN: CALCULAR RESISTENCIA ====================
float calculateResistance(float voltage) 
{
    if (voltage >= VCC || voltage <= 0) 
    {
        return 0;
    }
    
    // R_LDR = R_fixed * (VCC - V_out) / V_out
    return R_FIXED * (VCC - voltage) / voltage;
}

// ==================== FUNCIÓN: CALCULAR POSICIÓN DEL FOCO ====================
float calculateLightPosition(float res1, float res2) 
{
    // Normalizar: valores más bajos de resistencia = más luz
    // Rango: -100 (luz en LDR1) a +100 (luz en LDR2)
    
    if (res1 + res2 == 0) 
    {
        return 0;
    }
    
    // Calcular porcentaje de luz en cada sensor
    float light_ratio = (res2 - res1) / (res1 + res2);
    return light_ratio * 100.0;
}

// ==================== FUNCIÓN: CONTROLAR LEDs ====================
void controlLEDs(float position) 
{
    const float THRESHOLD = 10.0;  // Umbral de diferencia
    
    if (position < -THRESHOLD) 
    {
        // Más luz en LDR1
        digitalWrite(LED1_PIN, HIGH);
        digitalWrite(LED2_PIN, LOW);
    } 
    else if (position > THRESHOLD) 
    {
        // Más luz en LDR2
        digitalWrite(LED1_PIN, LOW);
        digitalWrite(LED2_PIN, HIGH);
    } 
    else 
    {
        // Luz equilibrada
        digitalWrite(LED1_PIN, LOW);
        digitalWrite(LED2_PIN, LOW);
    }
}

// ==================== FUNCIÓN: IMPRIMIR DATOS EN SERIAL ====================
void printSerialData(int raw1, int raw2, float res1, float res2, float position) 
{
    Serial.print("ADC1: ");
    Serial.print(raw1);
    Serial.print(" | ADC2: ");
    Serial.print(raw2);
    Serial.print(" | R_LDR1: ");
    Serial.print(res1);
    Serial.print("Ω | R_LDR2: ");
    Serial.print(res2);
    Serial.print("Ω | Posición: ");
    Serial.print(position, 1);
    Serial.println("%");
    
    // Indicador visual en serial
    Serial.print("Luz: [");
    for (int i = 0; i < 20; i++) 
    {
        if (i < 10 - (position / 10)) 
        {
            Serial.print(">");
        }
        else if (i > 10 + (position / 10)) 
        {
            Serial.print("<");
        }
        else 
        {
            Serial.print("-");
        }
    }
    Serial.println("]");
}