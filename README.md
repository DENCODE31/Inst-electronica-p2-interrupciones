# ⚡ Inst-Electronica-P2-Interrupciones

> **Instrumentación Electrónica** · Universidad Nacional de Colombia, sede Manizales  
> Implementación de interrupciones, temporizadores, conversión A/D y PWM sobre **ESP32 DevKit v1** con framework Arduino + PlatformIO.

---

## 📋 Contenido del Proyecto

| # | Archivo | Tema | Concepto clave |
|---|---------|------|---------------|
| 1 | `interrupciones.cpp` | Interrupciones externas | ISR + Debounce HW/SW |
| 2 | `timers.cpp` | Timers por interrupción | Medición de intervalos |
| 3 | `conversionAD.cpp` | Entradas analógicas A/D | Divisor de voltaje + LDR |
| 4 | `pwm.cpp` | Salidas PWM | Servo controlado por luz |

---

## 🔧 Hardware

**Placa:** ESP32 DevKit v1 · **Framework:** Arduino · **IDE:** PlatformIO + VS Code

### Componentes por ejercicio

```
EJ1 & EJ2 — Interrupciones / Timers
  ├── Push button
  ├── Resistencia pull-up 10kΩ
  └── Condensador 100nF (debounce hardware)

EJ3 — Conversión A/D
  ├── 2× LDR (fotorresistor)
  ├── 2× Resistencia fija 10kΩ (divisor de voltaje)
  └── 2× LED + resistencia 220Ω (alarma visual)

EJ4 — PWM
  ├── Todo lo del EJ3 +
  └── Servo SG90
```

### Mapa de pines

| Señal | GPIO | Nota |
|-------|------|------|
| Botón (EJ1/EJ2) | 18 | INPUT_PULLUP + 100nF a GND |
| LDR Izquierdo | 34 | Solo entrada · ADC1_CH6 |
| LDR Derecho | 35 | Solo entrada · ADC1_CH7 |
| LED Izquierdo | 25 | + 220Ω a GND |
| LED Derecho | 26 | + 220Ω a GND |
| Servo señal | 18 | PWM · 500–2400µs |

---

## 🧠 Principios implementados

### EJ1 — Antirrebote doble
```
Hardware: condensador 100nF filtra picos del rebote mecánico
Software: ISR verifica Δt ≥ 50ms antes de contar el pulso
```

### EJ3 — Fórmula del divisor de voltaje
```
       3.3V
        │
       LDR   ← resistencia variable con la luz
        │
       ADC   ← V_adc = 3.3 × R_fija / (R_LDR + R_fija)
        │
      10kΩ
        │
       GND

  R_LDR = R_fija × (3.3 / V_adc  − 1)
```

### EJ4 — Mapeo luz → ángulo
```
  ratio  = R_Der / (R_Izq + R_Der)
  ángulo = ratio × 180°   [0° – 180°]
```

---

## 🚀 Setup y compilación

```bash
# Clonar el repositorio
git clone https://github.com/DENCODE31/Inst-electronica-p2-interrupciones.git
cd Inst-electronica-p2-interrupciones

# Compilar (todos los ejercicios)
~/.platformio/penv/Scripts/pio.exe run

# Flashear a la placa
~/.platformio/penv/Scripts/pio.exe run --target upload

# Abrir monitor serial
~/.platformio/penv/Scripts/pio.exe device monitor
```

> **Nota:** Los 4 archivos `.cpp` están en `src/`. Para compilar y flashear un ejercicio específico, comenta los demás `setup()` / `loop()` o usa los entornos `#ifdef` del `platformio.ini`.

---

## 📁 Estructura del repositorio

```
Inst-electronica-p2-interrupciones/
├── src/
│   ├── interrupciones.cpp   # EJ1 – Interrupciones + debounce
│   ├── timers.cpp           # EJ2 – Tiempo entre pulsos
│   ├── conversionAD.cpp     # EJ3 – Lectura LDRs + ADC
│   └── pwm.cpp              # EJ4 – Servo por posición de luz
├── include/
├── lib/
├── test/
├── platformio.ini
└── README.md
```

---

## 👥 Autores

**DENCODE31** — Grupo de Instrumentación Electrónica  
Universidad Nacional de Colombia · Sede Manizales · 2025-I

---

<div align="center">
  <sub>Construido con ESP32 · PlatformIO · Arduino Framework</sub>
</div>