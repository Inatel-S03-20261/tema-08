#pragma once

#include <TFT_eSPI.h>

#include "app/IdEstado.h"

class ControladorAplicacao;

class Estado {
 public:
  virtual ~Estado() {}

  void setControlador(ControladorAplicacao* controlador) {
    controlador_ = controlador;
  }
  void marcarSujo() { sujo_ = true; }

  virtual void aoAbrir() { sujo_ = true; }
  virtual void aoFechar() {}
  virtual void tratarEvento() = 0;
  virtual void atualizar() {}
  virtual void renderizar(TFT_eSPI& tft) = 0;

 protected:
  ControladorAplicacao* controlador_ = nullptr;
  bool sujo_ = true;  // precisa redesenhar?
};
