#include "app/estados/EstadoLogin.h"

#include <Arduino.h>

#include <cstdio>
#include <cstring>

#include "app/ControladorAplicacao.h"
#include "contratos/IAutenticador.h"
#include "dominio/Jogador.h"
#include "dominio/Pokemon.h"
#include "infra/carregador.h"
#include "infra/sd_card.h"

void EstadoLogin::configurarRecursos(Pokemon* pool, int n, Aleatorio* rng) {
  pool_ = pool;
  poolN_ = n;
  rng_ = rng;
}

void EstadoLogin::configurarAutenticador(IAutenticador* autenticador) {
  autenticador_ = autenticador;
}

void EstadoLogin::tentarAutenticar(const char* usuario, const char* senha) {
  if (autenticador_ == nullptr) return;  // mock: sem login real
  ResultadoLogin r = autenticador_->autenticar(usuario, senha);
  Serial.printf("[LOGIN] %s: auth real %s\n", usuario,
                r.sucesso ? "OK" : "FALHOU (mock segue)");
}

void EstadoLogin::aoAbrir() {
  sujo_ = true;
  etapa_ = NICK1;
  len_ = 0;
  buffer_[0] = '\0';
  nick1_[0] = '\0';
  nick2_[0] = '\0';
  redesenhoTotal_ = true;
  ultTrainer_ = -1;
  while (Serial.available()) Serial.read();  // limpa o terminal
}

void EstadoLogin::atualizar() {
  while (Serial.available()) {
    char ch = static_cast<char>(Serial.read());

    if (ch == '\n' || ch == '\r') {
      if (len_ == 0) continue;  // ignora ENTER vazio
      buffer_[len_] = '\0';
      Serial.println();
      switch (etapa_) {
        case NICK1:
          strncpy(nick1_, buffer_, sizeof(nick1_) - 1);
          nick1_[sizeof(nick1_) - 1] = '\0';
          etapa_ = SENHA1;
          break;
        case SENHA1:
          tentarAutenticar(nick1_, buffer_);  // login real se configurado
          etapa_ = NICK2;  // senha sempre aceita (mock)
          break;
        case NICK2:
          strncpy(nick2_, buffer_, sizeof(nick2_) - 1);
          nick2_[sizeof(nick2_) - 1] = '\0';
          etapa_ = SENHA2;
          break;
        case SENHA2:
          tentarAutenticar(nick2_, buffer_);  // login real se configurado
          finalizar();
          return;
      }
      len_ = 0;
      buffer_[0] = '\0';
      sujo_ = true;
    } else if (ch == 8 || ch == 127) {  // backspace
      if (len_ > 0) {
        len_--;
        buffer_[len_] = '\0';
        sujo_ = true;
      }
    } else if (ch >= 32 && ch < 127 && len_ < static_cast<int>(sizeof(buffer_)) - 1) {
      buffer_[len_++] = ch;
      buffer_[len_] = '\0';
      Serial.print(ch);  // eco no terminal
      sujo_ = true;
    }
  }
}

void EstadoLogin::finalizar() {
  if (controlador_ == nullptr) return;
  Jogador* j1 = controlador_->getJogador1();
  Jogador* j2 = controlador_->getJogador2();
  if (j1 != nullptr) j1->configurar(nick1_[0] != '\0' ? nick1_ : "Jogador 1");
  if (j2 != nullptr) j2->configurar(nick2_[0] != '\0' ? nick2_ : "Jogador 2");
  if (j1 != nullptr && j2 != nullptr && pool_ != nullptr && rng_ != nullptr) {
    distribuirAleatorio(*j1, *j2, pool_, poolN_, 5, *rng_);
  }
  while (Serial.available()) Serial.read();
  controlador_->trocarEstado(IdEstado::SELECAO);
}

namespace {
const int kNomeX = 120, kNomeY = 86, kNomeW = 138;
const int kSenhaX = 172, kSenhaY = 144, kSenhaW = 88;
const int kCampoH = 18;

void desenharValor(TFT_eSPI& tft, int x, int y, int w, const char* texto,
                   uint16_t cor) {
  tft.fillRect(x, y, w, kCampoH, TFT_WHITE);
  tft.setTextColor(cor);
  tft.drawString(texto, x + 2, y, 2);
}
}  // namespace

void EstadoLogin::renderizar(TFT_eSPI& tft) {
  if (!sujo_) return;

  int trainer = (etapa_ == NICK1 || etapa_ == SENHA1) ? 1 : 2;
  bool campoSenha = (etapa_ == SENHA1 || etapa_ == SENHA2);
  const char* fundo =
      trainer == 1 ? "/components/trainer_1.jpg" : "/components/trainer_2.jpg";

  bool completo = redesenhoTotal_ || trainer != ultTrainer_;
  redesenhoTotal_ = false;
  ultTrainer_ = trainer;

  if (completo) {
    tft.fillScreen(TFT_BLACK);
    draw_jpegFromSD(fundo, 0, 0);
    tft.setTextColor(TFT_NAVY);
    tft.drawCentreString("Digite no terminal + ENTER", 160, 204, 2);
  }

  const char* nickArmazenado = trainer == 1 ? nick1_ : nick2_;
  char linhaNome[28];
  if (!campoSenha) {
    snprintf(linhaNome, sizeof(linhaNome), "%s_", buffer_);
  } else {
    snprintf(linhaNome, sizeof(linhaNome), "%s", nickArmazenado);
  }
  desenharValor(tft, kNomeX, kNomeY, kNomeW, linhaNome,
                campoSenha ? TFT_BLACK : TFT_BLUE);

  char linhaSenha[16];
  if (campoSenha) {
    int i = 0;
    for (; i < len_ && i < 6; ++i) linhaSenha[i] = '*';
    linhaSenha[i] = '\0';
    char comCursor[18];
    snprintf(comCursor, sizeof(comCursor), "%s_", linhaSenha);
    desenharValor(tft, kSenhaX, kSenhaY, kSenhaW, comCursor, TFT_BLUE);
  } else {
    desenharValor(tft, kSenhaX, kSenhaY, kSenhaW, "", TFT_BLACK);
  }

  sujo_ = false;
}
