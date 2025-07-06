# Proyecto-ATIF
en este repositorio se avanza sobre un sistema iot diseñado para generar alertas y eventos en pos de minizar daños probocados por incendios forestales
## Características del sistema ATIF

- **A-Sensor**: Dispositivo de campo encargado de recolectar datos ambientales críticos:
    - Temperatura
    - Humedad
    - Presión atmosférica
    - Detección de partículas (cenizas)
    - Niveles de CO₂ y CO
    - Posición GPS
    - Estado de batería y panel solar
    - Otros sensores adicionales según necesidad
    - Comunicación mediante LoRa hacia el concentrador

- **B-Gate**: Dispositivo concentrador de datos:
    - Recibe información de múltiples A-Sensor vía LoRa
    - Conectividad a internet mediante WiFi, LTE, LTE-M o 3G
    - Envía los datos de todos los sensores conectados a un broker MQTT
    - Monitorea la conectividad de los A-Sensor y genera alertas inmediatas si alguno pierde conexión

- **Infraestructura de backend**:
    - Broker MQTT encapsulado en un contenedor Docker
    - Base de datos para almacenamiento de eventos y datos históricos
    - Node-RED para procesamiento, triangulación de posiciones y detección de patrones de incendio

- **Alertas y eventos**:
    - Generación de alertas tempranas ante detección de condiciones de riesgo
    - Notificación inmediata ante desconexión de cualquier A-Sensor

- **Escalabilidad y modularidad**:
    - Soporte para múltiples sensores y gateways
    - Arquitectura basada en contenedores para facilitar despliegue y mantenimiento