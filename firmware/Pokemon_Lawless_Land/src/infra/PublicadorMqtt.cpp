#include "infra/PublicadorMqtt.h"

#include <Arduino.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASS ""
#define MQTT_BROKER "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_TOPICO "LawlessLandResults"

PublicadorMqtt::PublicadorMqtt() : mqtt_(wifiClient_) {}

void PublicadorMqtt::iniciar() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Conectando ao WiFi");
  unsigned long inicio = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - inicio < 10000) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi conectado: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi falhou");
  }
  mqtt_.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt_.setBufferSize(1024);
}

bool PublicadorMqtt::publicar(const char* payload) {
  if (WiFi.status() != WL_CONNECTED) return false;

  if (!mqtt_.connected()) {
    char clientId[40];
    snprintf(clientId, sizeof(clientId), "lawlessland-%06lX",
             static_cast<unsigned long>(ESP.getEfuseMac() & 0xFFFFFF));
    if (!mqtt_.connect(clientId)) {
      Serial.printf("MQTT connect falhou (estado %d)\n", mqtt_.state());
      return false;
    }
  }

  bool ok = mqtt_.publish(MQTT_TOPICO, payload);
  mqtt_.loop();
  Serial.printf("MQTT publish em %s: %s -> %s\n", MQTT_TOPICO,
                ok ? "OK" : "FALHOU", payload);
  return ok;
}
