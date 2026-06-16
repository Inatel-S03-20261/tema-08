#pragma once

#include "app/Estado.h"
#include "dominio/Batalha.h"

class IPokemon;
class IJogador;

class EstadoBatalha : public Estado {
 public:
  void aoAbrir() override;
  void tratarEvento() override;
  void renderizar(TFT_eSPI& tft) override;

 private:
  struct StatusCache {
    IPokemon* poke = nullptr;
    IJogador* jog = nullptr;
    int vida = -1;
    int energia = -1;
    bool destaque = false;
    bool valido = false;
  };
  void atualizarStatus(TFT_eSPI& tft, int x, int y, IJogador& jog, IPokemon& p,
                       bool destaque, StatusCache& cache, bool forcar);

  Batalha* batalha_ = nullptr;
  int cursor_ = 0;
  int cursorPokemon_ = 0;
  bool semEnergia_ = false;
  bool redesenhoTotal_ = true;
  bool ultFrameSelecao_ = false;
  bool selAtiva_ = false;
  int ultCursorSel_ = -1;
  IPokemon* ultCima_ = nullptr;
  IPokemon* ultBaixo_ = nullptr;
  LogTurno ultLogDesenhado_;
  StatusCache cacheCima_;
  StatusCache cacheBaixo_;
};
