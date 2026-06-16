#include "dominio/Jogador.h"

#include <cstring>

Jogador::Jogador() : qtdCartas_(0), atual_(nullptr) { nome_[0] = '\0'; }

void Jogador::configurar(const char* nome) {
  if (nome == nullptr) {
    nome_[0] = '\0';
  } else {
    strncpy(nome_, nome, sizeof(nome_) - 1);
    nome_[sizeof(nome_) - 1] = '\0';
  }
  qtdCartas_ = 0;
  atual_ = nullptr;
}

void Jogador::limparCartas() {
  qtdCartas_ = 0;
  atual_ = nullptr;
}

void Jogador::adicionarCarta(Pokemon* pokemon) {
  if (pokemon != nullptr && qtdCartas_ < kMaxCartas) {
    cartas_[qtdCartas_++] = pokemon;
  }
}
