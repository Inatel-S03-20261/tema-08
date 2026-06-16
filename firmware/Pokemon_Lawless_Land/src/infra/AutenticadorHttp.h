#pragma once

#include "contratos/IAutenticador.h"

class AutenticadorHttp : public IAutenticador {
 public:
  explicit AutenticadorHttp(const char* baseUrl);  // ex.: "http://192.168.0.10:8080"
  ResultadoLogin autenticar(const char* usuario, const char* senha) override;

 private:
  const char* baseUrl_;
};
