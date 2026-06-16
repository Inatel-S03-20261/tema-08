#pragma once

#include "contratos/IJogador.h"
#include "contratos/IPokemon.h"

// Jogador falso para os testes.
class FakeJogador : public IJogador {
 public:
  explicit FakeJogador(const char* nome)
      : nome_(nome), qtdCartas_(0), atual_(nullptr) {}

  void adicionarCarta(IPokemon* p) {
    if (qtdCartas_ < kMaxCartas) cartas_[qtdCartas_++] = p;
  }

  const char* getNome() const override { return nome_; }
  int getQuantidadeCartas() const override { return qtdCartas_; }
  IPokemon* getCarta(int i) const override { return cartas_[i]; }
  IPokemon* getPokemonAtual() const override { return atual_; }
  void setPokemonAtual(IPokemon* p) override { atual_ = p; }

 private:
  static const int kMaxCartas = 6;
  const char* nome_;
  IPokemon* cartas_[kMaxCartas];
  int qtdCartas_;
  IPokemon* atual_;
};
