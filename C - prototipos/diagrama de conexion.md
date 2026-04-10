# Conexión del Botón Reset WiFi

## Esquema de Conexión

```
┌──────────────────────┐
│      ESP32           │
│                      │
│  GPIO 34 ●───────┐   │
│                  │   │
│      GND ●───┐   │   │
│              │   │   │
└──────────────┼───┼───┘
               │   │
               │   │
             ──┴─  │
            ( BTN )│
             ──┬─  │
               │   │
               └───┘
```

## Conexión Física

**Opción 1 (Recomendada) - GPIO 34:**
```
Botón Pin 1 ──→ GPIO 34 (ESP32)
Botón Pin 2 ──→ GND (ESP32)
```

**Opción 2 - GPIO 0 (BOOT):**
```
Botón Pin 1 ──→ GPIO 0 (ESP32)
Botón Pin 2 ──→ GND (ESP32)
```

## Características del Pin GPIO 34

- ✅ **Input-only** (ideal para botones)
- ✅ **No interfiere con el boot**
- ✅ **Pull-up interno habilitado por software**
- ✅ **Nivel lógico: 3.3V**

## ⚠️ NO Usar Resistencia Externa

El código configura automáticamente la resistencia pull-up interna:
```cpp
pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
```

Esto significa:
- **Estado normal (sin presionar):** Pin = HIGH (3.3V)
- **Estado presionado:** Pin = LOW (GND - 0V)

## Funcionamiento

1. **Presionar el botón** durante **3 segundos**
2. El ESP32 muestra en serial: 
   ```
   🔘 Botón presionado - mantener 3 seg para reset WiFi
   ⏱️  Manteniendo... 1/3 seg
   ⏱️  Manteniendo... 2/3 seg
   ⏱️  Manteniendo... 3/3 seg
   🔄 Reseteando configuración WiFi...
   🔄 Reiniciando ESP32...
   ```
3. El LCD muestra: `Reset WiFi...`
4. El ESP32 se reinicia y entra en **modo AP** para configuración

## Tipo de Botón

Cualquier pulsador normalmente abierto (NA / NO):
- Push button táctil
- Microswitch
- Pulsador momentáneo

**Ejemplo de botones compatibles:**
- Pulsador táctil 6x6mm
- Push button 12x12mm
- Microswitch
- Botón arcade

## Alternativa: Usar GPIO 0 (BOOT)

Si prefieres usar el botón BOOT que ya tiene el ESP32:

```cpp
const int RESET_BUTTON_PIN = 0;  // GPIO 0 (botón BOOT)
```

**Ventaja:** No necesitas conectar nada extra
**Desventaja:** Es el mismo botón usado para programar

## Diagrama Completo del Sistema

```
┌────────────────────────────────────┐
│           ESP32 DEVKIT V1          │
├────────────────────────────────────┤
│                                    │
│  GPIO 34 ●── [Botón Reset] ── GND │
│                                    │
│  GPIO 21 (SDA) ●──┐                │
│  GPIO 22 (SCL) ●──┼───→ LCD I2C   │
│      3.3V ●───────┤                │
│      GND ●────────┘                │
│                                    │
│  USB ●─────────→ PC (Programación) │
│                                    │
└────────────────────────────────────┘
```

## Comportamiento del Botón

| Acción | Tiempo | Resultado |
|--------|--------|-----------|
| Presionar < 3 seg | - | Nada (ignorado) |
| Presionar ≥ 3 seg | 3000 ms | Reset WiFi + Reinicio |
| Soltar antes de 3 seg | - | Cancelado |

## Código Relevante

El botón se verifica en cada ciclo del `loop()`:
```cpp
void loop() {
  checkResetButton();  // ← Verifica el botón constantemente
  // ... resto del código
}
```

## Tips de Instalación

1. **Usa cables cortos** (< 30cm) para evitar ruido
2. **Solda bien las conexiones** para evitar falsos contactos
3. **Aísla las conexiones** con termo-retráctil
4. **Monta el botón** en la carcasa de forma accesible
5. **Etiqueta el botón** como "RESET WiFi"

## Troubleshooting

**Problema:** El botón no responde
- ✓ Verifica conexión GPIO 34 ↔ Botón
- ✓ Verifica conexión GND ↔ Botón
- ✓ Comprueba que el botón funciona (prueba con multímetro)
- ✓ Revisa el monitor serial para mensajes

**Problema:** Se resetea sin presionar
- ✓ Verifica que no haya cortos
- ✓ Comprueba que el cable no esté roto
- ✓ Asegúrate de que el botón sea NA (normalmente abierto)

**Problema:** Necesita más/menos tiempo
- Modifica `HOLD_TIME` en `conexiones.cpp`:
  ```cpp
  const unsigned long HOLD_TIME = 5000;  // 5 segundos
  ```
