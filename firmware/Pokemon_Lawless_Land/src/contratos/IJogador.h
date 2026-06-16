#pragma once

#include "contratos/IPokemon.h"

class IJogador {
 public:
  virtual ~IJogador() {}

  virtual const char* getNome() const = 0;

  virtual int getQuantidadeCartas() const = 0;
  virtual IPokemon* getCarta(int indice) const = 0;

  virtual IPokemon* getPokemonAtual() const = 0;
  virtual void setPokemonAtual(IPokemon* pokemon) = 0;

  bool temPokemonVivo() const {
    for (int i = 0; i < getQuantidadeCartas(); ++i) {
      IPokemon* c = getCarta(i);
      if (c != nullptr && c->estaVivo()) return true;
    }
    return false;
  }
};
