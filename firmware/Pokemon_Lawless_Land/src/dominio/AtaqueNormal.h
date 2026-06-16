#pragma once

#include "dominio/Ataque.h"

class AtaqueNormal : public Ataque {
 public:
  AtaqueNormal(const char* nome, const char* tipo, int custo, int poder);
  ResultadoAtaque calcular(const IPokemon& atacante,
                           const IPokemon& defensor) const override;
};
