#pragma once

#include "contratos/IJogador.h"
#include "dominio/Pokemon.h"

class Jogador : public IJogador {
 public:
  static const int kMaxCartas = 6;

  Jogador();

  void configurar(const char* nome);
  void limparCartas();
  void adicionarCarta(Pokemon* pokemon);
  Pokemon* getCartaConcreta(int i) const { return cartas_[i]; }

  const char* getNome() const override { return nome_; }
  int getQuantidadeCartas() const override { return qtdCartas_; }
  IPokemon* getCarta(int i) const override { return cartas_[i]; }
  IPokemon* getPokemonAtual() const override { return atual_; }
  void setPokemonAtual(IPokemon* pokemon) override { atual_ = pokemon; }

 private:
  char nome_[24];
  Pokemon* cartas_[kMaxCartas];
  int qtdCartas_;
  IPokemon* atual_;
};
