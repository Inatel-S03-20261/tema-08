#include "app/ControladorAplicacao.h"

ControladorAplicacao& ControladorAplicacao::instancia() {
  static ControladorAplicacao unica;
  return unica;
}

void ControladorAplicacao::registrarEstado(IdEstado id,
                                            Estado* estado) {
  int i = static_cast<int>(id);
  if (i < 0 || i >= kNumEstados) return;
  registro_[i] = estado;
  if (estado != nullptr) estado->setControlador(this);
}

void ControladorAplicacao::iniciar(TFT_eSPI* tft, IdEstado inicial) {
  tft_ = tft;
  int i = static_cast<int>(inicial);
  if (i < 0 || i >= kNumEstados) return;
  idAtual_ = inicial;
  atual_ = registro_[i];
  if (atual_ != nullptr) atual_->aoAbrir();
}

void ControladorAplicacao::trocarEstado(IdEstado id) {
  int i = static_cast<int>(id);
  if (i < 0 || i >= kNumEstados) return;
  Estado* proximo = registro_[i];
  if (proximo == nullptr) return;

  if (atual_ != nullptr) atual_->aoFechar();
  atual_ = proximo;
  idAtual_ = id;
  atual_->aoAbrir();
}

void ControladorAplicacao::tratarEvento() {
  if (atual_ != nullptr) atual_->tratarEvento();
}

void ControladorAplicacao::atualizar() {
  if (atual_ != nullptr) atual_->atualizar();
}

void ControladorAplicacao::renderizar() {
  if (atual_ != nullptr && tft_ != nullptr) atual_->renderizar(*tft_);
}
