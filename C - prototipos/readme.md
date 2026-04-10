# Gateway MQTT con ESP32

Sistema de gateway MQTT sobre WebSocket para ESP32, con configuración WiFi mediante portal web y visualización de estado en pantalla LCD.

## 📋 Características

- ✅ Conexión MQTT sobre WebSocket (puerto 80)
- ✅ Portal web de configuración WiFi (modo Access Point)
- ✅ Pantalla LCD I2C para visualización de estado
- ✅ Reconexión automática WiFi y MQTT
- ✅ Publicación periódica de estado del gateway
- ✅ Sincronización horaria con NTP (zona horaria Argentina)

## 🔧 Hardware Requerido

- **ESP32 DOIT DevKit V1** (o compatible)
- **Pantalla LCD I2C 16x2** (dirección 0x27 o 0x3F)
- Cable USB para programación
- Fuente de alimentación 5V

### Conexiones LCD I2C

| LCD I2C | ESP32 |
|---------|-------|
| VCC     | 3.3V  |
| GND     | GND   |
| SDA     | GPIO21|
| SCL     | GPIO22|

## 📦 Dependencias

Las siguientes librerías se instalan automáticamente con PlatformIO:

```ini
- ArduinoJson @ ^7.4.2
- LiquidCrystal_I2C @ ^1.1.4
- WebSockets @ ^2.7.1
- PubSubClient @ ^2.8
- WiFi (incluida en ESP32)
- Preferences (incluida en ESP32)
- WebServer (incluida en ESP32)
```

## 🚀 Instalación

### 1. Clonar el repositorio

```bash
git clone <url-repositorio>
cd gateway_mqtt
```

### 2. Compilar y cargar

Con PlatformIO:

```bash
pio run --target upload
```

Con PlatformIO IDE:
- Abrir el proyecto en VSCode
- Hacer clic en el botón "Upload" (→) en la barra inferior

### 3. Monitor Serial (opcional)

```bash
pio device monitor
```

Velocidad: **115200 baudios**

## 📱 Configuración WiFi (Primera vez)

### Paso 1: Modo Access Point
Cuando el ESP32 no tiene WiFi configurado:
- Crea un punto de acceso llamado: `GatewayMQTT_Gat_01`
- El modo AP permanece activo por **3 minutos**
- La pantalla LCD muestra: `Modo AP activo` y `192.168.4.1`

### Paso 2: Conectarse al AP
1. Desde tu celular o PC, busca la red WiFi `GatewayMQTT_Gat_01`
2. Conéctate (no requiere contraseña)

### Paso 3: Configurar WiFi
1. Abre el navegador web
2. Navega a: `http://192.168.4.1`
3. Ingresa tu **SSID** (nombre de red WiFi)
4. Ingresa tu **contraseña** WiFi
5. Haz clic en "Guardar y Conectar"

### Paso 4: Reinicio automático
- El ESP32 guarda la configuración en memoria permanente
- Se reinicia automáticamente
- Se conecta al WiFi configurado
- Inicia conexión MQTT

## 🌐 Funcionamiento Normal

### Conexión WiFi
- El ESP32 se conecta automáticamente al WiFi guardado
- Si pierde conexión, reintenta cada **10 segundos**
- La IP asignada se muestra en el monitor serial

### Conexión MQTT
- **Servidor:** `mqtt.ispciot.org`
- **Puerto:** 80 (WebSocket)
- **Path:** `/mqtt`
- **Client ID:** `Gat_01`
- Reconexión automática cada **5 segundos** si se pierde

### Publicación de Estado
Cada **30 segundos**, el gateway publica en:

**Tópico:** `gateway/Gat_01/status`

**Formato JSON:**
```json
{
  "gateway_id": "Gat_01",
  "rssi": -45,
  "quality": "Excelente",
  "timestamp": "14:30 16/10"
}
```

### Calidad de Señal WiFi
- **Excelente:** RSSI ≥ -50 dBm
- **Buena:** RSSI ≥ -60 dBm
- **Regular:** RSSI ≥ -70 dBm
- **Débil:** RSSI < -70 dBm

### Suscripción MQTT
El gateway se suscribe al tópico:
- `gateway/Gat_01/cmd` (para recibir comandos)

## 📺 Pantalla LCD

La pantalla LCD muestra información en tiempo real (actualización cada 5 segundos):

### Modo AP
```
Modo AP activo
192.168.4.1
```

### WiFi Conectado
```
WiFi: OK
MQTT: OK
```

### WiFi Desconectado
```
WiFi: Descon.
Reintentando...
```

### Sin Configuración
```
Sin config WiFi
```

## 🔄 Intervalos de Tiempo

| Acción | Intervalo |
|--------|-----------|
| Publicación MQTT | 30 segundos |
| Actualización LCD | 5 segundos |
| Verificación MQTT | 5 segundos |
| Reintentos WiFi | 10 segundos |
| Duración modo AP | 3 minutos |

## 📁 Estructura del Proyecto

```
gateway_mqtt/
├── src/
│   ├── main.cpp              # Código principal
│   ├── conexiones.h          # Declaraciones WiFi/MQTT
│   ├── conexiones.cpp        # Implementación WiFi/MQTT
│   ├── lcdplus.h            # Declaraciones LCD
│   ├── lcdplus.cpp          # Implementación LCD
│   ├── web_portal.h         # Declaraciones portal web
│   ├── web_portal.cpp       # Implementación portal web
│   └── WebSocketClient.h    # Wrapper WebSocket-Cliente
├── platformio.ini           # Configuración PlatformIO
└── README.md               # Este archivo
```

## ⚙️ Personalización

### Cambiar ID del Gateway
En `conexiones.cpp`, línea 7:
```cpp
const char* gatewayId = "Gat_01";  // Cambiar aquí
```

### Cambiar Servidor MQTT
En `conexiones.cpp`, líneas 8-10:
```cpp
const char* mqtt_server = "mqtt.ispciot.org";
const uint16_t mqtt_port = 80;
const char* mqtt_path = "/mqtt";
```

### Cambiar Dirección LCD I2C
En `lcdplus.cpp`, línea 7:
```cpp
static LiquidCrystal_I2C lcd(0x27, 16, 2);  // 0x27 o 0x3F
```

### Ajustar Intervalos
En `main.cpp`, líneas 7-12:
```cpp
const unsigned long apDuration = 180000;      // 3 minutos AP
const unsigned long publishInterval = 30000;   // 30s publicación
const unsigned long lcdInterval = 5000;        // 5s actualización LCD
const unsigned long mqttCheckInterval = 5000;  // 5s verificación MQTT
```

## 🐛 Solución de Problemas

### LCD no muestra nada
- Verificar conexiones I2C (SDA/SCL)
- Probar dirección alternativa: cambiar `0x27` por `0x3F` en `lcdplus.cpp`
- Ajustar contraste del LCD (potenciómetro en el módulo I2C)

### No se conecta al WiFi
- Verificar SSID y contraseña correctos
- Asegurarse de que el router está en 2.4GHz (ESP32 no soporta 5GHz)
- Revisar monitor serial para mensajes de error

### No se conecta a MQTT
- Verificar que el servidor `mqtt.ispciot.org` está accesible
- Comprobar que el puerto 80 está abierto
- Revisar monitor serial: buscar mensajes "MQTT conectado" o errores

### El modo AP no aparece
- Borrar configuración WiFi guardada:
  ```cpp
  preferences.begin("wifi_cfg", false);
  preferences.clear();
  preferences.end();
  ESP.restart();
  ```

### Memoria Flash llena
- Revisar uso: 74.6% es normal
- Si necesitas espacio, puedes reducir tamaño de buffer MQTT
- Considerar particiones personalizadas

## 📊 Uso de Recursos

- **RAM:** 14.8% (48,372 bytes / 327,680 bytes)
- **Flash:** 74.6% (978,449 bytes / 1,310,720 bytes)

## 🔐 Seguridad

⚠️ **Consideraciones importantes:**
- El portal web no usa HTTPS (limitación del ESP32)
- Las credenciales WiFi se guardan en texto plano en la memoria flash
- El modo AP no tiene contraseña (solo activo 3 minutos)
- Para producción, considera implementar:
  - Contraseña en el AP
  - Cifrado de credenciales
  - Autenticación MQTT

## 📝 Registro de Cambios

### Versión 1.0.0
- ✅ Implementación inicial
- ✅ Soporte MQTT sobre WebSocket
- ✅ Portal web de configuración
- ✅ Pantalla LCD I2C
- ✅ Reconexión automática
- ✅ Sincronización NTP

## 📄 Licencia

[Especificar tu licencia aquí]

## 👥 Contribuciones

Las contribuciones son bienvenidas. Por favor:
1. Fork del repositorio
2. Crear rama para tu feature (`git checkout -b feature/NuevaFuncionalidad`)
3. Commit de cambios (`git commit -m 'Agregar nueva funcionalidad'`)
4. Push a la rama (`git push origin feature/NuevaFuncionalidad`)
5. Abrir Pull Request

## 📧 Contacto

[Tu información de contacto]

## 🙏 Agradecimientos

- Biblioteca WebSockets de Links2004
- Biblioteca PubSubClient de knolleary
- Biblioteca LiquidCrystal_I2C de marcoschwartz
- Comunidad de PlatformIO y ESP32

---

**Nota:** Este proyecto fue desarrollado para el curso/proyecto de IoT con MQTT.
