#include "dominio/Aleatorio.h"

Aleatorio::Aleatorio(uint32_t semente) { semear(semente); }

void Aleatorio::semear(uint32_t semente) {
  estado_ = semente != 0 ? semente : 1;  // xorshift nao pode comecar em 0
}

uint32_t Aleatorio::proximoBruto() {
  uint32_t x = estado_;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  estado_ = x;
  return x;
}

int Aleatorio::proximo(int limite) {
  if (limite <= 0) return 0;
  return static_cast<int>(proximoBruto() % static_cast<uint32_t>(limite));
}
