#include "dominio/AtaqueSuperEfetivo.h"

#include "dominio/TabelaTipos.h"

AtaqueSuperEfetivo::AtaqueSuperEfetivo(const char* nome, const char* tipo,
                                       int custo, int poder,
                                       float multiplicadorTipo)
    : Ataque(nome, tipo, custo, poder), multiplicadorTipo_(multiplicadorTipo) {}

ResultadoAtaque AtaqueSuperEfetivo::calcular(const IPokemon& /*atacante*/,
                                             const IPokemon& defensor) const {
  ResultadoAtaque r;
  int bruto = poder_;
  if (TabelaTipos::superEfetivoContra(tipo_, defensor)) {
    bruto = static_cast<int>(poder_ * multiplicadorTipo_);
    r.superEfetivo = true;
  }
  r.dano = aplicarDefesa(bruto, defensor);
  return r;
}
