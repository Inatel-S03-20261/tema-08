#include "dominio/Ataque.h"

Ataque::Ataque(const char* nome, const char* tipo, int custo, int poder)
    : nome_(nome), tipo_(tipo), custo_(custo), poder_(poder) {}

int Ataque::aplicarDefesa(int bruto, const IPokemon& defensor) {
  int defesa = defensor.getDefesa();
  if (defesa < 0) defesa = 0;
  int dano = (bruto * 100) / (100 + defesa);
  return dano < 1 ? 1 : dano;  // piso de 1
}

ResultadoAtaque Ataque::executar(IPokemon& atacante, IPokemon& defensor) const {
  ResultadoAtaque r = calcular(atacante, defensor);

  int energia = atacante.getEnergia() - custo_;
  atacante.setEnergia(energia < 0 ? 0 : energia);

  int vida = defensor.getVida() - r.dano;
  defensor.setVida(vida < 0 ? 0 : vida);

  return r;
}
