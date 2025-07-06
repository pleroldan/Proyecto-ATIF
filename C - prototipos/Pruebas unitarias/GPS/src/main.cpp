#include <WiFi.h>
#include <HTTPClient.h>
#include <TinyGPSPlus.h>

#define GPS_RX 16
#define GPS_TX 17
const char* ssid = "ssid";
const char* password = "pass@";
const char* apiKey = "apikey"; // Reemplaza con tu API Key de GeoLinker

HardwareSerial gpsSerial(1);
TinyGPSPlus gps;

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" conectado!");
}

void loop() {
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  static double lastLat = 0.0, lastLng = 0.0;
  static unsigned long lastSend = 0;
  const char* device_id = "ESP32_GPS_01"; // Cambia este ID si lo deseas

  if (gps.location.isValid()) {
    double lat = gps.location.lat();
    double lng = gps.location.lng();

    // Cambios significativos: 0.0001° en lat/lng
    bool changed = (fabs(lat - lastLat) > 0.0001) || (fabs(lng - lastLng) > 0.0001);

    if (changed && WiFi.status() == WL_CONNECTED) {
      lastLat = lat;
      lastLng = lng;
      // Timestamp UTC actual
      char timestamp[20] = "";
      if (gps.date.isValid() && gps.time.isValid()) {
        snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02d %02d:%02d:%02d",
          gps.date.year(), gps.date.month(), gps.date.day(),
          gps.time.hour(), gps.time.minute(), gps.time.second());
      } else {
        strcpy(timestamp, "1970-01-01 00:00:00");
      }

      String payload = "{";
      payload += "\"device_id\":\"" + String(device_id) + "\",";
      payload += "\"timestamp\":[\"" + String(timestamp) + "\"],";
      payload += "\"lat\": [" + String(lat, 6) + "],";
      payload += "\"long\": [" + String(lng, 6) + "]";
      payload += "}";

      HTTPClient http;
      http.begin("https://www.circuitdigest.cloud/geolinker");
      http.addHeader("Content-Type", "application/json");
      http.addHeader("Authorization", apiKey);

      int httpResponseCode = http.POST(payload);
      Serial.print("📡 Enviado a GeoLinker: ");
      Serial.println(httpResponseCode);
      http.end();

      lastSend = millis();
    }
  }
  delay(1000); // Pequeña espera para no saturar el loop
}
