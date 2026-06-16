#pragma once

#include <cstdint>

class Aleatorio {
 public:
  explicit Aleatorio(uint32_t semente = 1);
  void semear(uint32_t semente);
  int proximo(int limite);  // [0, limite)
  bool chance(int porcentagem) { return proximo(100) < porcentagem; }

 private:
  uint32_t proximoBruto();
  uint32_t estado_;
};
