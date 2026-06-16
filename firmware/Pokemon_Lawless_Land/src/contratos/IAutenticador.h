#pragma once

struct ResultadoLogin {
  bool sucesso = false;
  char token[512] = {0};  // JWT quando sucesso
};

class IAutenticador {
 public:
  virtual ~IAutenticador() {}
  virtual ResultadoLogin autenticar(const char* usuario, const char* senha) = 0;
};
