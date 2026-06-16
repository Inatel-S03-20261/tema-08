#include "dominio/AtaqueCritico.h"

AtaqueCritico::AtaqueCritico(const char* nome, const char* tipo, int custo,
                             int poder, float multiplicadorCritico, Aleatorio& rng)
    : Ataque(nome, tipo, custo, poder),
      multiplicadorCritico_(multiplicadorCritico),
      rng_(rng) {}

ResultadoAtaque AtaqueCritico::calcular(const IPokemon& /*atacante*/,
                                        const IPokemon& defensor) const {
  ResultadoAtaque r;
  int bruto = poder_;
  if (rng_.chance(kChanceCriticoPorcento)) {
    bruto = static_cast<int>(poder_ * multiplicadorCritico_);
    r.critico = true;
  }
  r.dano = aplicarDefesa(bruto, defensor);
  return r;
}
