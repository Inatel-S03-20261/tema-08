#pragma once

#include "app/Estado.h"

class EstadoSelecao : public Estado {
 public:
  void aoAbrir() override;
  void tratarEvento() override;
  void renderizar(TFT_eSPI& tft) override;

 private:
  int jogadorAtual_ = 1;  // 1 ou 2
  int cursor_ = 0;
  bool redesenhoTotal_ = true;
  int ultJogadorDesenhado_ = 0;  // 0 = nenhum
};
