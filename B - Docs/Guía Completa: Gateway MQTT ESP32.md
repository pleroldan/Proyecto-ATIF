# 📘 Guía Completa: Gateway MQTT ESP32

## 🎯 Tabla de Contenidos

1. [Arquitectura del Proyecto](#arquitectura)
2. [Cómo Funciona la Memoria Persistente](#memoria-persistente)
3. [Estructura Modular del Código](#estructura-modular)
4. [Paso a Paso de Implementación](#paso-a-paso)
5. [Flujo de Ejecución](#flujo-ejecucion)
6. [Tips de Programación Modular](#tips-modular)
7. [Troubleshooting](#troubleshooting)

---

## 📐 Arquitectura del Proyecto {#arquitectura}

```
┌─────────────────────────────────────────┐
│           main.cpp (Orquestador)        │
│  - setup(): Inicializa módulos          │
│  - loop(): Coordina operaciones         │
└────────┬────────────────────────────────┘
         │
         ├─→ conexiones.cpp (WiFi + MQTT)
         │   ├─ Gestión WiFi
         │   ├─ WebSocket
         │   ├─ MQTT Manual
         │   └─ Botón Reset
         │
         ├─→ lcdplus.cpp (Pantalla)
         │   ├─ Control LCD
         │   └─ Actualización Estado
         │
         ├─→ web_portal.cpp (Configuración)
         │   ├─ Servidor Web
         │   ├─ Portal Captive
         │   └─ Guardado Config
         │
         └─→ mqtt_manual.h (Protocolo MQTT)
             ├─ Construcción Paquetes
             ├─ Publicar/Suscribir
             └─ Procesamiento Mensajes
```

---

## 💾 Cómo Funciona la Memoria Persistente {#memoria-persistente}

### **Preferences: Memoria No Volátil del ESP32**

El ESP32 tiene una partición especial llamada **NVS (Non-Volatile Storage)** que mantiene datos incluso después de:
- ✅ Reinicio (reset)
- ✅ Apagado total
- ✅ Pérdida de energía
- ✅ Actualización de firmware (OTA)

### **Cómo se Usa en el Proyecto**

```cpp
#include <Preferences.h>
Preferences preferences;  // Objeto global

// ========== GUARDAR DATOS ==========
void guardarWiFi(String ssid, String password) {
    preferences.begin("wifi_cfg", false);  // Abrir namespace
    //                  ↑           ↑
    //              Namespace   ReadOnly=false
    
    preferences.putString("ssid", ssid);         // Guardar SSID
    preferences.putString("password", password); // Guardar Password
    
    preferences.end();  // Cerrar (guarda automáticamente)
}

// ========== LEER DATOS ==========
void leerWiFi() {
    preferences.begin("wifi_cfg", false);
    
    String ssid = preferences.getString("ssid", "");  // "" = valor por defecto
    String password = preferences.getString("password", "");
    
    preferences.end();
}

// ========== BORRAR TODO ==========
void borrarWiFi() {
    preferences.begin("wifi_cfg", false);
    preferences.clear();  // Borra TODAS las claves del namespace
    preferences.end();
}
```

### **¿Por Qué Sobrevive a Reinicios?**

```
┌──────────────────────────────────────┐
│         Memoria Flash ESP32          │
├──────────────────────────────────────┤
│                                      │
│  [Bootloader]  ← Siempre aquí       │
│  [App Firmware] ← Tu código         │
│  [NVS/Preferences] ← ¡AQUÍ SE GUARDA!│
│  [SPIFFS/LittleFS] ← Archivos       │
│                                      │
└──────────────────────────────────────┘
```

La sección **NVS** es independiente del firmware:
- No se borra al reprogramar
- No se borra con `ESP.restart()`
- Solo se borra con `preferences.clear()` o flash completo

### **Ejemplo Real del Flujo**

```
1️⃣ Primera vez:
   - ESP arranca
   - preferences.getString("ssid", "") → Devuelve ""
   - wifiConfigured = false
   - Inicia modo AP

2️⃣ Usuario configura WiFi:
   - Ingresa "MiWiFi" y "password123"
   - web_portal.cpp llama preferences.putString()
   - Datos escritos en NVS Flash ✓

3️⃣ ESP se reinicia:
   - ESP arranca de nuevo
   - preferences.getString("ssid", "") → Devuelve "MiWiFi"
   - wifiConfigured = true
   - Se conecta automáticamente ✓

4️⃣ Usuario presiona botón reset:
   - preferences.clear()
   - NVS se borra
   - ESP reinicia → Vuelve a paso 1️⃣
```

---

## 🧩 Estructura Modular del Código {#estructura-modular}

### **Principio: Separación de Responsabilidades**

Cada archivo tiene **UNA sola responsabilidad**:

```cpp
// ❌ MAL - Todo en main.cpp
void setup() {
    Serial.begin(115200);
    WiFi.begin("ssid", "pass");
    lcd.init();
    server.begin();
    webSocket.begin();
    // ... 300 líneas más
}

// ✅ BIEN - Modular
void setup() {
    Serial.begin(115200);
    initLCD();        // → lcdplus.cpp
    initWiFi();       // → conexiones.cpp
    initMQTT();       // → conexiones.cpp
    setupWebServer(); // → web_portal.cpp
}
```

### **Archivos .h (Headers): Contratos**

Los `.h` definen **QUÉ** hace el módulo (interfaz):

```cpp
// conexiones.h - CONTRATO
#ifndef CONEXIONES_H
#define CONEXIONES_H

// Declara QUÉ funciones existen
void initWiFi();
void conectarWifi();
void publishStatus();

// Declara QUÉ variables se comparten
extern bool mqttConnected;
extern String ssid;

#endif
```

### **Archivos .cpp (Implementation): Implementación**

Los `.cpp` definen **CÓMO** lo hace (implementación):

```cpp
// conexiones.cpp - IMPLEMENTACIÓN
#include "conexiones.h"

// Define CÓMO funciona cada función
void initWiFi() {
    preferences.begin("wifi_cfg", false);
    ssid = preferences.getString("ssid", "");
    // ...
}

void conectarWifi() {
    WiFi.begin(ssid.c_str(), password.c_str());
    // ...
}
```

### **¿Por Qué `extern`?**

```cpp
// conexiones.h
extern bool mqttConnected;  // "Esta variable existe en algún lugar"

// conexiones.cpp
bool mqttConnected = false; // "AQUÍ está la variable real"

// main.cpp
#include "conexiones.h"
void loop() {
    if (mqttConnected) {  // Puede usarla
        // ...
    }
}
```

`extern` = "Prometo que esta variable existe, la encontrarás al compilar"

---

## 🚀 Paso a Paso de Implementación {#paso-a-paso}

### **Paso 1: Preparar el Hardware**

#### **Materiales Necesarios:**
- ✅ ESP32 DOIT DevKit V1
- ✅ Pantalla LCD I2C 16x2
- ✅ Botón pulsador (NA)
- ✅ 4 cables jumper hembra-hembra (para LCD)
- ✅ 2 cables para el botón
- ✅ Cable USB micro

#### **Conexiones:**

```
LCD I2C 16x2:
  VCC  → 3.3V (ESP32)
  GND  → GND
  SDA  → GPIO 21
  SCL  → GPIO 22

Botón Reset:
  Pin 1 → GPIO 34
  Pin 2 → GND
```

**Foto de Referencia:**
```
        ┌─────────┐
    VCC─┤●      ●├─SDA → GPIO 21
    GND─┤●      ●├─SCL → GPIO 22
        │ LCD I2C│
        │ 16 x 2 │
        └─────────┘

      [BTN]
       │ │
       │ └──→ GND
       └────→ GPIO 34
```

---

### **Paso 2: Crear el Proyecto en PlatformIO**

```bash
# En VSCode con PlatformIO
1. Ctrl+Shift+P → "PlatformIO: New Project"
2. Name: gateway_mqtt
3. Board: DOIT ESP32 DEVKIT V1
4. Framework: Arduino
```

---

### **Paso 3: Configurar platformio.ini**

```ini
[env:esp32doit-devkit-v1]
platform = espressif32
board = esp32doit-devkit-v1
framework = arduino
lib_deps = 
    bblanchon/ArduinoJson@^7.4.2
    marcoschwartz/LiquidCrystal_I2C@^1.1.4
    links2004/WebSockets@^2.7.1
monitor_speed = 115200
```

---

### **Paso 4: Crear Estructura de Archivos**

```
gateway_mqtt/
├── src/
│   ├── main.cpp              ← Crear primero
│   ├── conexiones.h          ← Crear
│   ├── conexiones.cpp        ← Crear
│   ├── lcdplus.h             ← Crear
│   ├── lcdplus.cpp           ← Crear
│   ├── web_portal.h          ← Crear
│   ├── web_portal.cpp        ← Crear
│   └── mqtt_manual.h         ← Crear
└── platformio.ini
```

---

### **Paso 5: Copiar el Código**

#### **5.1 mqtt_manual.h**
Copia el código del artifact "mqtt_manual.h"

#### **5.2 lcdplus.h**
Copia el código del artifact "lcdplus.h"

#### **5.3 lcdplus.cpp**
Copia el código del artifact "lcdplus.cpp"

**⚠️ IMPORTANTE:** Si tu LCD está en dirección **0x3F** en lugar de **0x27**, cambia esta línea:
```cpp
// lcdplus.cpp - línea 7
static LiquidCrystal_I2C lcd(0x3F, 16, 2);  // Cambiar aquí
```

**¿Cómo saber la dirección?** Usa este sketch:
```cpp
#include <Wire.h>
void setup() {
  Wire.begin();
  Serial.begin(115200);
  Serial.println("Escaneando I2C...");
  for(byte i = 1; i < 127; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.print("Encontrado: 0x");
      Serial.println(i, HEX);
    }
  }
}
void loop() {}
```

#### **5.4 conexiones.h**
Copia el código del artifact "conexiones.h"

#### **5.5 conexiones.cpp**
Copia el código del artifact "conexiones.cpp"

**Personaliza estos valores:**
```cpp
// conexiones.cpp - líneas 6-9
const char* gatewayId = "Gat_01";              // ← Tu ID único
const char* mqtt_server = "mqtt.ispciot.org";  // ← Tu servidor
const uint16_t mqtt_port = 80;
const char* mqtt_path = "/mqtt";
const int RESET_BUTTON_PIN = 34;               // ← Pin del botón
```

#### **5.6 web_portal.h**
```cpp
#ifndef WEB_PORTAL_H
#define WEB_PORTAL_H

void setupWebServer();
void handleWebRequests();

#endif
```

#### **5.7 web_portal.cpp**
Copia el código del artifact "web_portal.cpp"

#### **5.8 main.cpp**
Copia el código del artifact "main.cpp"

---

### **Paso 6: Verificar Dirección I2C del LCD**

**Método 1: Con Scanner I2C**

```cpp
// Temporal - scanner_i2c.cpp
#include <Arduino.h>
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); // SDA, SCL
  Serial.println("\n=== Scanner I2C ===");
}

void loop() {
  byte error, address;
  int devices = 0;

  Serial.println("Escaneando...");

  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("Dispositivo encontrado en 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      devices++;
    }
  }

  if (devices == 0)
    Serial.println("No se encontraron dispositivos I2C");
  else
    Serial.println("Escaneo completo");

  delay(5000);
}
```

Resultado esperado:
```
Escaneando...
Dispositivo encontrado en 0x27  ← Esta es tu dirección
Escaneo completo
```

---

### **Paso 7: Compilar y Cargar**

```bash
# En VSCode
1. Conectar ESP32 por USB
2. Presionar "Build" (✓) - debe compilar sin errores
3. Presionar "Upload" (→) - carga el firmware
4. Presionar "Monitor" (🔌) - ver mensajes serial
```

**Salida esperada en Serial:**
```
=== Gateway MQTT ESP32 ===

AP iniciado: GatewayMQTT_Gat_01
Servidor web iniciado en 192.168.4.1
```

---

### **Paso 8: Configurar WiFi**

1. **Buscar WiFi:** Desde tu celular, busca red `GatewayMQTT_Gat_01`
2. **Conectarse** (sin contraseña)
3. **Abrir navegador:** Ir a `http://192.168.4.1`
4. **Ingresar datos:**
   - SSID: Tu red WiFi
   - Password: Tu contraseña WiFi
5. **Guardar y Conectar**
6. **ESP reinicia automáticamente**

**LCD mostrará:**
```
Primera vez:
Modo AP activo
192.168.4.1

Después de configurar:
WiFi: OK
MQTT: OK
```

---

### **Paso 9: Verificar Conexión MQTT**

**Serial Monitor mostrará:**
```
✅ WiFi conectado!
IP: 192.168.100.74
MQTT/WebSocket inicializado
Esperando WebSocket...
✅ WebSocket conectado
📤 MQTT CONNECT enviado
✅ MQTT Conectado!
📤 Suscrito a: gateway/Gat_01/cmd
✅ SUBACK recibido
📤 Publicado [gateway/Gat_01/status]: {"gateway_id":"Gat_01",...}
```

---

### **Paso 10: Probar Botón Reset**

1. **Presionar botón** (GPIO 34 → GND)
2. **Mantener 3 segundos**
3. **Ver progreso en LCD:**
   ```
   Reset WiFi: 1/3
   ████░░░░░░░░░░░░
   
   Reset WiFi: 2/3
   ████████░░░░░░░░
   
   Reset WiFi: 3/3
   ████████████████
   
   Borrando WiFi
   Reiniciando...
   ```
4. **ESP reinicia en modo AP**

---

## 🔄 Flujo de Ejecución {#flujo-ejecucion}

### **Diagrama de Estados**

```
┌─────────────┐
│   INICIO    │
└──────┬──────┘
       │
       ▼
┌──────────────────┐
│ ¿WiFi guardado?  │
└──────┬───────┬───┘
       │ NO    │ SÍ
       │       │
       ▼       ▼
   ┌──────┐ ┌────────────┐
   │ Modo │ │ Conectar   │
   │  AP  │ │ WiFi       │
   └──┬───┘ └─────┬──────┘
      │           │
      │           ▼
      │     ┌──────────────┐
      │     │ Iniciar MQTT │
      │     └──────┬───────┘
      │            │
      │            ▼
      │     ┌──────────────┐
      │     │ Loop Normal  │◄────┐
      │     │ - WebSocket  │     │
      │     │ - MQTT       │     │
      │     │ - LCD        │     │
      │     │ - Botón      │─────┘
      │     └──────────────┘
      │
      ▼
┌─────────────────┐
│ Portal Config   │
│ Esperar 3 min   │
└─────────────────┘
```

### **Secuencia de Llamadas**

```cpp
// ========== SETUP ==========
main.cpp:setup()
  │
  ├─→ Serial.begin(115200)
  ├─→ initLCD()                    → lcdplus.cpp
  │    └─→ lcd.init()
  │    └─→ lcd.backlight()
  │
  ├─→ initWiFi()                   → conexiones.cpp
  │    └─→ preferences.begin()
  │    └─→ preferences.getString("ssid")
  │    └─→ wifiConfigured = (ssid != "")
  │
  ├─→ if (wifiConfigured) {
  │    ├─→ conectarWifi()          → conexiones.cpp
  │    │    └─→ WiFi.begin(ssid, pass)
  │    │
  │    └─→ initMQTT()              → conexiones.cpp
  │         ├─→ new MQTTManual()
  │         └─→ webSocket.begin()
  │
  └─→ else {
       ├─→ iniciarAP()             → conexiones.cpp
       │    └─→ WiFi.softAP()
       │
       └─→ setupWebServer()        → web_portal.cpp
            └─→ server.begin()

// ========== LOOP ==========
main.cpp:loop()
  │
  ├─→ checkResetButton()           → conexiones.cpp
  │    └─→ if (3 seg) ESP.restart()
  │
  ├─→ if (apMode) {
  │    └─→ handleWebRequests()     → web_portal.cpp
  │         └─→ server.handleClient()
  │
  └─→ else {
       ├─→ webSocket.loop()
       ├─→ mqttLoop()              → conexiones.cpp
       ├─→ publishStatus()         → conexiones.cpp
       └─→ actualizarLCD()         → lcdplus.cpp
```

---

## 💡 Tips de Programación Modular {#tips-modular}

### **Tip 1: Un Módulo = Una Responsabilidad**

```cpp
// ✅ BIEN: lcdplus.cpp solo maneja LCD
void initLCD() { /* ... */ }
void actualizarLCD() { /* ... */ }
LiquidCrystal_I2C& getLCD() { /* ... */ }

// ❌ MAL: lcdplus.cpp hace de todo
void initLCD() { /* ... */ }
void conectarWiFi() { /* ... */ }  // ← Esto va en conexiones.cpp
void publishMQTT() { /* ... */ }   // ← Esto va en conexiones.cpp
```

### **Tip 2: Headers (.h) = Documentación**

```cpp
// conexiones.h
#ifndef CONEXIONES_H
#define CONEXIONES_H

// ========== VARIABLES GLOBALES ==========
extern bool mqttConnected;    // Estado conexión MQTT
extern String ssid;           // SSID guardado

// ========== FUNCIONES PÚBLICAS ==========
void initWiFi();              // Inicializa WiFi desde Preferences
void conectarWifi();          // Conecta al WiFi guardado
void publishStatus();         // Publica estado del gateway

#endif
```

El `.h` es como el **manual de instrucciones** del módulo.

### **Tip 3: Variables `extern` vs Locales**

```cpp
// ========== VARIABLE GLOBAL (compartida) ==========
// conexiones.h
extern bool mqttConnected;

// conexiones.cpp
bool mqttConnected = false;  // Definición real

// main.cpp puede usarla:
if (mqttConnected) { /* ... */ }

// ========== VARIABLE LOCAL (privada) ==========
// conexiones.cpp
static unsigned long lastPing = 0;  // Solo visible aquí
```

**Regla:** Si solo un archivo la necesita → `static`. Si varios → `extern`.

### **Tip 4: Funciones Privadas**

```cpp
// conexiones.cpp

// ========== FUNCIÓN PRIVADA ==========
static String formatearHora() {  // Solo visible en este archivo
    // ...
}

// ========== FUNCIÓN PÚBLICA ==========
String obtenerHoraArgentina() {  // Declarada en .h, visible en todos
    return formatearHora();
}
```

### **Tip 5: Incluir Solo lo Necesario**

```cpp
// lcdplus.cpp
#include "lcdplus.h"           // ✅ Su propio header
#include "conexiones.h"        // ✅ Necesita mqttConnected
#include <LiquidCrystal_I2C.h> // ✅ Usa LCD
#include <WiFi.h>              // ✅ Usa WiFi.status()

// NO incluir cosas que no usa:
// #include "web_portal.h"     // ❌ No lo necesita
// #include <WebSocketsClient.h> // ❌ No lo necesita
```

### **Tip 6: main.cpp es el Director de Orquesta**

```cpp
// main.cpp - Simple y claro
void setup() {
    Serial.begin(115200);
    
    // Inicializar cada módulo
    initLCD();
    initWiFi();
    if (wifiConfigured) {
        conectarWifi();
        initMQTT();
    } else {
        iniciarAP();
        setupWebServer();
    }
}

void loop() {
    // Coordinar módulos
    checkResetButton();
    
    if (apMode) {
        handleWebRequests();
    } else {
        webSocket.loop();
        mqttLoop();
        if (shouldPublish()) publishStatus();
    }
    
    actualizarLCD();
}
```

**main.cpp NO debe tener lógica compleja**, solo coordinar.

---

## 🔧 Troubleshooting {#troubleshooting}

### **Problema: LCD no muestra nada**

**Síntomas:**
- Backlight encendido pero sin texto
- Pantalla completamente negra

**Soluciones:**

1. **Verificar dirección I2C:**
   ```cpp
   // En lcdplus.cpp, probar:
   static LiquidCrystal_I2C lcd(0x27, 16, 2);  // Común
   // O
   static LiquidCrystal_I2C lcd(0x3F, 16, 2);  // Alternativa
   ```

2. **Ajustar contraste:**
   - Girar el potenciómetro azul en el módulo I2C

3. **Verificar conexiones:**
   ```
   VCC → 3.3V (NO 5V para evitar dañar ESP32)
   GND → GND
   SDA → GPIO 21
   SCL → GPIO 22
   ```

4. **Probar scanner I2C** (ver Paso 6)

---

### **Problema: No se conecta a WiFi**

**Síntomas:**
- LCD muestra "WiFi: Descon."
- Serial: "Conectando WiFi..........."

**Soluciones:**

1. **Verificar SSID y contraseña:**
   - Resetear con botón (3 seg)
   - Reconfigurar desde portal

2. **Verificar banda WiFi:**
   - ESP32 solo soporta **2.4 GHz**
   - No funciona con 5 GHz

3. **Borrar configuración manualmente:**
   ```cpp
   // En setup(), temporalmente agregar:
   clearWiFiConfig();
   ESP.restart();
   ```

4. **Ver debug en Serial:**
   ```
   Conectando a WiFi: MiWiFi
   . . . . . . (20 puntos = timeout)
   ```

---

### **Problema: MQTT no conecta**

**Síntomas:**
- WiFi OK pero "MQTT: NO"
- Serial: "❌ falló, rc=-2"

**Soluciones:**

1. **Verificar servidor:**
   ```cpp
   // conexiones.cpp
   const char* mqtt_server = "mqtt.ispciot.org";  // ¿Correcto?
   const uint16_t mqtt_port = 80;                 // ¿Correcto?
   ```

2. **Verificar firewall:**
   - Puerto 80 debe estar abierto
   - Servidor debe aceptar WebSocket

3. **Probar con mosquitto local:**
   ```bash
   # En tu PC
   mosquitto -p 1883 -v
   
   # Cambiar en código:
   const char* mqtt_server = "192.168.x.x";  // IP de tu PC
   const uint16_t mqtt_port = 1883;
   ```

---

### **Problema: Botón no responde**

**Síntomas:**
- Presionar botón no hace nada
- LCD no cambia

**Soluciones:**

1. **Verificar pin:**
   ```cpp
   // conexiones.cpp
   const int RESET_BUTTON_PIN = 34;  // ¿Conectado aquí?
   ```

2. **Probar otro pin:**
   ```cpp
   const int RESET_BUTTON_PIN = 0;  // Botón BOOT integrado
   ```

3. **Verificar conexión:**
   ```
   GPIO 34 ─── [Botón] ─── GND
   ```

4. **Test manual:**
   ```cpp
   void setup() {
       pinMode(34, INPUT_PULLUP);
       Serial.begin(115200);
   }
   
   void loop() {
       Serial.println(digitalRead(34));  // 1 = no presionado, 0 = presionado
       delay(100);
   }
   ```

---

### **Problema: Compilación falla**

**Errores comunes:**

1. **"'getLCD' was not declared"**
   ```cpp
   // Asegurarse de incluir en main.cpp:
   #include "lcdplus.h"
   ```

2. **"WebSocketClient.h: No such file"**
   - Borrar `#include "WebSocketClient.h"`
   - Ese archivo ya no se usa

3. **"PubSubClient.h: No such file"**
   - Remover de platformio.ini:
     ```ini
     knolleary/PubSubClient@^2.8  ← Borrar esta línea
     ```

---

### **Problema: Se reinicia constantemente**

**Causas:**

1. **Watchdog Timer:**
   ```cpp
   // Agregar en loop() cada función pesada:
   delay(10);  // Da tiempo al watchdog
   ```

2. **Falta de memoria:**
   ```cpp
   // Ver uso en compilación:
   RAM:   [=         ]  14.8% ← Debe ser < 80%
   Flash: [=======   ]  74.6% ← Debe ser < 95%
   ```

---

## 📚 Recursos Adicionales

### **Documentación Oficial:**
- ESP32: https://docs.espressif.com/
- PlatformIO: https://docs.platformio.org/
- MQTT Protocol: https://mqtt.org/

### **Bibliotecas Usadas:**
- WebSockets: https://github.com/Links2004/arduinoWebSockets
- LiquidCrystal I2C: https://github.com/johnrickman/LiquidCrystal_I2C
- ArduinoJson: https://arduinojson.org/

### **Herramientas Útiles:**
- MQTT Explorer: https://mqtt-explorer.com/
- Serial Monitor: PlatformIO integrado
- I2C Scanner: (código en Paso 6)

---

## ✅ Checklist Final

Antes de dar por terminado:

- [ ] LCD muestra información correcta
- [ ] WiFi se conecta automáticamente después de configurar
- [ ] MQTT publica cada 30 segundos
- [ ] Botón reset funciona (3 segundos)
- [ ] Portal web accesible en modo AP
- [ ] Configuración sobrevive a reinicios
- [ ] Serial monitor muestra mensajes claros
- [ ] Sin errores de compilación
- [ ] Memoria < 80% RAM y < 95% Flash

---

**¡Proyecto completado! 🎉**

¿Tienes dudas sobre algún paso específico?
