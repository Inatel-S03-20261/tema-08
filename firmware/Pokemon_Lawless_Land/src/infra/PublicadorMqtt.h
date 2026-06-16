#pragma once

#include <PubSubClient.h>
#include <WiFi.h>

#include "contratos/IPublicadorResultado.h"

class PublicadorMqtt : public IPublicadorResultado {
 public:
  PublicadorMqtt();
  void iniciar();
  bool publicar(const char* payload) override;

 private:
  WiFiClient wifiClient_;
  PubSubClient mqtt_;
};
