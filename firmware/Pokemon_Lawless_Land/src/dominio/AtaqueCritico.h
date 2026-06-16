#pragma once

#include "dominio/Aleatorio.h"
#include "dominio/Ataque.h"

class AtaqueCritico : public Ataque {
 public:
  static const int kChanceCriticoPorcento = 20;

  AtaqueCritico(const char* nome, const char* tipo, int custo, int poder,
                float multiplicadorCritico, Aleatorio& rng);
  ResultadoAtaque calcular(const IPokemon& atacante,
                           const IPokemon& defensor) const override;

 private:
  float multiplicadorCritico_;
  Aleatorio& rng_;
};
