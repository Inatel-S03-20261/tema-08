#pragma once

#include "contratos/IPokemon.h"

struct ResultadoAtaque {
  int dano = 0;
  bool critico = false;
  bool superEfetivo = false;
};

class Ataque {
 public:
  Ataque(const char* nome, const char* tipo, int custo, int poder);
  virtual ~Ataque() {}

  const char* getNome() const { return nome_; }
  const char* getTipo() const { return tipo_; }
  int getCusto() const { return custo_; }

  ResultadoAtaque executar(IPokemon& atacante, IPokemon& defensor) const;

  virtual ResultadoAtaque calcular(const IPokemon& atacante,
                                   const IPokemon& defensor) const = 0;

  int calcularDano(const IPokemon& atacante, const IPokemon& defensor) const {
    return calcular(atacante, defensor).dano;
  }

 protected:
  static int aplicarDefesa(int bruto, const IPokemon& defensor);

  const char* nome_;
  const char* tipo_;
  int custo_;
  int poder_;
};
