#pragma once

#include "dominio/Ataque.h"

class AtaqueSuperEfetivo : public Ataque {
 public:
  AtaqueSuperEfetivo(const char* nome, const char* tipo, int custo, int poder,
                     float multiplicadorTipo);
  ResultadoAtaque calcular(const IPokemon& atacante,
                           const IPokemon& defensor) const override;

 private:
  float multiplicadorTipo_;
};
