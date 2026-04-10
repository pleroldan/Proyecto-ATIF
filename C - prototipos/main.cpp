// main.cpp
#include <Arduino.h>
#include "conexiones.h"
#include "lcdplus.h"
#include "web_portal.h"

unsigned long apStartTime = 0;
const unsigned long apDuration = 180000; // 3 min
unsigned long lastPublish = 0;
const unsigned long publishInterval = 30000; // 30s
unsigned long lastLCD = 0;
const unsigned long lcdInterval = 5000; // 5s

// Declarar mqttLoop
void mqttLoop();

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== Gateway MQTT ESP32 ===\n");
  
  initLCD();
  getLCD().clear(); 
  getLCD().print("Iniciando...");
  
  initWiFi();
  
  if (wifiConfigured) {
    apMode = false;
    conectarWifi();
    
    // Esperar conexión WiFi
    Serial.print("Conectando WiFi");
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("✅ WiFi conectado!");
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
      
      // Inicializar MQTT/WebSocket
      initMQTT();
      
      // Esperar un poco para que WebSocket se conecte
      Serial.println("Esperando WebSocket...");
      delay(2000);
      
    } else {
      Serial.println("❌ WiFi falló");
    }
  } else {
    iniciarAP();
    setupWebServer();
    apStartTime = millis();
  }
}

void loop() {
  // Verificar botón de reset en cualquier momento
  checkResetButton();
  
  if (apMode) {
    handleWebRequests();
    if (millis() - apStartTime >= apDuration) {
      WiFi.softAPdisconnect(true);
      apMode = false;
      if (!wifiConfigured) {
        getLCD().clear(); 
        getLCD().print("Sin config WiFi");
      }
    }
  } else {
    handleWiFiReconnect();
    
    if (WiFi.status() == WL_CONNECTED) {
      // Loop de WebSocket
      webSocket.loop();
      
      // Loop MQTT (ping)
      mqttLoop();
      
      // Publicar estado
      if (millis() - lastPublish >= publishInterval) {
        publishStatus();
        lastPublish = millis();
      }
    }
  }
  
  // Actualizar LCD
  if (millis() - lastLCD >= lcdInterval) {
    actualizarLCD();
    lastLCD = millis();
  }
  
  delay(10);
}
