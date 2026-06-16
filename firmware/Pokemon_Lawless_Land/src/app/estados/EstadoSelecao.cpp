#include "app/estados/EstadoSelecao.h"

#include <cstdio>

#include "app/ControladorAplicacao.h"
#include "dominio/Batalha.h"
#include "dominio/Jogador.h"
#include "dominio/Pokemon.h"
#include "infra/buttons.h"
#include "infra/sd_card.h"

namespace {

const int kSpriteX = 112;
const int kSpriteY = 56;
const int kSpriteTam = 96;

void restaurarDeck(Jogador* j) {
  if (j == nullptr) return;
  for (int i = 0; i < j->getQuantidadeCartas(); ++i) {
    j->getCartaConcreta(i)->restaurar();
  }
  j->setPokemonAtual(nullptr);
}

Jogador* jogadorDe(ControladorAplicacao* c, int n) {
  return n == 1 ? c->getJogador1() : c->getJogador2();
}

}  // namespace

void EstadoSelecao::aoAbrir() {
  sujo_ = true;
  jogadorAtual_ = 1;
  cursor_ = 0;
  redesenhoTotal_ = true;
  ultJogadorDesenhado_ = 0;
  if (controlador_ != nullptr) {
    restaurarDeck(controlador_->getJogador1());
    restaurarDeck(controlador_->getJogador2());
  }
}

void EstadoSelecao::tratarEvento() {
  if (controlador_ == nullptr) return;
  Jogador* jog = jogadorDe(controlador_, jogadorAtual_);
  if (jog == nullptr) return;
  int n = jog->getQuantidadeCartas();
  if (n <= 0) return;

  if (button_leftClicked()) {
    cursor_ = (cursor_ - 1 + n) % n;
    sujo_ = true;
  }
  if (button_rightClicked()) {
    cursor_ = (cursor_ + 1) % n;
    sujo_ = true;
  }
  if (button_backClicked()) {
    if (jogadorAtual_ == 2) {
      jogadorAtual_ = 1;
      cursor_ = 0;
      sujo_ = true;
    } else {
      controlador_->trocarEstado(IdEstado::LOGIN);
    }
  }
  if (button_selectClicked()) {
    jog->setPokemonAtual(jog->getCarta(cursor_));
    if (jogadorAtual_ == 1) {
      jogadorAtual_ = 2;
      cursor_ = 0;
      sujo_ = true;
    } else {
      Batalha* b = controlador_->getBatalha();
      if (b != nullptr) b->iniciar();
      controlador_->trocarEstado(IdEstado::BATALHA);
    }
  }
}

void EstadoSelecao::renderizar(TFT_eSPI& tft) {
  if (!sujo_) return;
  Jogador* jog =
      controlador_ != nullptr ? jogadorDe(controlador_, jogadorAtual_) : nullptr;
  if (jog == nullptr || jog->getQuantidadeCartas() <= 0) {
    sujo_ = false;
    return;
  }

  bool completo = redesenhoTotal_ || jogadorAtual_ != ultJogadorDesenhado_;
  redesenhoTotal_ = false;
  ultJogadorDesenhado_ = jogadorAtual_;

  const char* kFundo = "/components/pick.jpg";  // fundo claro
  if (completo) {
    tft.fillScreen(TFT_BLACK);
    draw_jpegFromSD(kFundo, 0, 0);
    tft.setTextColor(TFT_NAVY);
    tft.drawCentreString("a/d move   s confirma   w volta", 160, 30, 1);
  }

  Pokemon* p = jog->getCartaConcreta(cursor_);

  if (!restore_jpegRegion(kFundo, kSpriteX, kSpriteY, kSpriteTam, kSpriteTam)) {
    tft.fillRect(kSpriteX, kSpriteY, kSpriteTam, kSpriteTam, TFT_WHITE);
  }
  draw_pokemonImage(p->getPastaAssets(), false, kSpriteX, kSpriteY);

  char buf[48];
  if (!restore_jpegRegion(kFundo, 0, 6, 320, 20))
    tft.fillRect(0, 6, 320, 20, TFT_WHITE);
  snprintf(buf, sizeof(buf), "%s: escolha (%d/%d)", jog->getNome(), cursor_ + 1,
           jog->getQuantidadeCartas());
  tft.setTextColor(TFT_BLUE);
  tft.drawCentreString(buf, 160, 8, 2);

  if (!restore_jpegRegion(kFundo, 0, 158, 320, 22))
    tft.fillRect(0, 158, 320, 22, TFT_WHITE);
  snprintf(buf, sizeof(buf), "%s  HP %d  [%s]", p->getNome(), p->getVidaMaxima(),
           p->getQuantidadeTipos() > 0 ? p->getTipo(0) : "-");
  tft.setTextColor(TFT_BLACK);
  tft.drawCentreString(buf, 160, 160, 2);

  sujo_ = false;
}
