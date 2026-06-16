#pragma once

#include <TFT_eSPI.h>

#include "app/Estado.h"
#include "app/IdEstado.h"

class Batalha;
class Jogador;

class ControladorAplicacao {
 public:
  static ControladorAplicacao& instancia();

  void registrarEstado(IdEstado id, Estado* estado);
  void iniciar(TFT_eSPI* tft,
               IdEstado inicial = IdEstado::LOGIN);
  void trocarEstado(IdEstado id);

  void tratarEvento();
  void atualizar();
  void renderizar();

  TFT_eSPI* getTft() const { return tft_; }
  IdEstado getEstadoAtualId() const { return idAtual_; }

  void setBatalha(Batalha* batalha) { batalha_ = batalha; }
  Batalha* getBatalha() const { return batalha_; }
  void setJogadores(Jogador* j1, Jogador* j2) {
    jogador1_ = j1;
    jogador2_ = j2;
  }
  Jogador* getJogador1() const { return jogador1_; }
  Jogador* getJogador2() const { return jogador2_; }

 private:
  ControladorAplicacao() = default;
  ControladorAplicacao(const ControladorAplicacao&) = delete;
  ControladorAplicacao& operator=(const ControladorAplicacao&) = delete;

  static const int kNumEstados =
      static_cast<int>(IdEstado::NUM_ESTADOS);

  Estado* registro_[kNumEstados] = {};
  Estado* atual_ = nullptr;
  IdEstado idAtual_ = IdEstado::LOGIN;
  TFT_eSPI* tft_ = nullptr;

  Batalha* batalha_ = nullptr;
  Jogador* jogador1_ = nullptr;
  Jogador* jogador2_ = nullptr;
};
