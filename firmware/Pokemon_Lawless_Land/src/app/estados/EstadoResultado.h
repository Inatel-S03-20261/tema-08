#pragma once

#include "app/Estado.h"

class IPublicadorResultado;

class EstadoResultado : public Estado {
 public:
  void configurarPublicador(IPublicadorResultado* publicador);

  void aoAbrir() override;
  void tratarEvento() override;
  void atualizar() override;
  void renderizar(TFT_eSPI& tft) override;

 private:
  enum class Envio { ENVIANDO = 0, CONCLUIDO = 1, FALHOU = 2 };
  void desenharAvisoEnvio(TFT_eSPI& tft, Envio estado);

  IPublicadorResultado* publicador_ = nullptr;
  char payload_[1024];
  bool pendente_ = false;
  bool renderizado_ = false;
  bool tentouEnviar_ = false;
  bool envioOk_ = false;
  int avisoMostrado_ = -1;
};
