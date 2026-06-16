#include "dominio/AtaqueNormal.h"

AtaqueNormal::AtaqueNormal(const char* nome, const char* tipo, int custo,
                           int poder)
    : Ataque(nome, tipo, custo, poder) {}

ResultadoAtaque AtaqueNormal::calcular(const IPokemon& /*atacante*/,
                                       const IPokemon& defensor) const {
  ResultadoAtaque r;
  r.dano = aplicarDefesa(poder_, defensor);
  return r;
}
