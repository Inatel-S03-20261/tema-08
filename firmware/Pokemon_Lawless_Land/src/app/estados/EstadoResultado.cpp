#include "app/estados/EstadoResultado.h"

#include <cstdio>

#include "app/ControladorAplicacao.h"
#include "dominio/Batalha.h"
#include "dominio/Jogador.h"
#include "dominio/Rodada.h"
#include "contratos/IPublicadorResultado.h"
#include "infra/buttons.h"
#include "infra/sd_card.h"

namespace {
const char* kFundo = "/components/results.jpg";  // fundo claro (Battle Results)
const char* nomePoke(IPokemon* p) { return p != nullptr ? p->getNome() : "-"; }

void limparFaixa(TFT_eSPI& tft, int x, int y, int w, int h) {
  if (!restore_jpegRegion(kFundo, x, y, w, h)) tft.fillRect(x, y, w, h, TFT_WHITE);
}
}  // namespace

void EstadoResultado::configurarPublicador(IPublicadorResultado* publicador) {
  publicador_ = publicador;
}

void EstadoResultado::aoAbrir() {
  sujo_ = true;
  pendente_ = false;
  renderizado_ = false;
  tentouEnviar_ = false;
  envioOk_ = false;
  avisoMostrado_ = -1;
  payload_[0] = '\0';

  Batalha* b = (controlador_ != nullptr) ? controlador_->getBatalha() : nullptr;
  Jogador* j1 = (controlador_ != nullptr) ? controlador_->getJogador1() : nullptr;
  Jogador* j2 = (controlador_ != nullptr) ? controlador_->getJogador2() : nullptr;
  if (b == nullptr || j1 == nullptr || j2 == nullptr) return;

  IJogador* v = b->getVencedor();
  const char* nomeJ1 = j1->getNome();
  const char* nomeJ2 = j2->getNome();

  int off = 0;
  off += snprintf(payload_ + off, sizeof(payload_) - off,
                  "{\"vencedor\":\"%s\",\"jogador1\":\"%s\",\"jogador2\":\"%s\","
                  "\"rodadas\":[",
                  v != nullptr ? v->getNome() : "-", nomeJ1, nomeJ2);
  for (int i = 0; i < b->getQuantidadeRodadas(); ++i) {
    const Rodada& r = b->getRodada(i);
    bool j1Venceu = (r.pokemonVencedor == r.pokemonJogador1);
    off += snprintf(
        payload_ + off, sizeof(payload_) - off,
        "%s{\"n\":%d,\"%s\":\"%s\",\"%s\":\"%s\",\"vencedor\":\"%s\","
        "\"vencedorJogador\":\"%s\"}",
        i > 0 ? "," : "", r.numero, nomeJ1, nomePoke(r.pokemonJogador1), nomeJ2,
        nomePoke(r.pokemonJogador2), nomePoke(r.pokemonVencedor),
        j1Venceu ? nomeJ1 : nomeJ2);
    if (off >= static_cast<int>(sizeof(payload_)) - 64) break;  // seguranca
  }
  off += snprintf(payload_ + off, sizeof(payload_) - off, "]}");
  pendente_ = true;
}

void EstadoResultado::tratarEvento() {
  if (button_selectClicked() && controlador_ != nullptr) {
    controlador_->trocarEstado(IdEstado::LOGIN);
  }
}

void EstadoResultado::atualizar() {
  if (pendente_ && renderizado_) {
    tentouEnviar_ = true;
    envioOk_ = (publicador_ != nullptr) && publicador_->publicar(payload_);
    pendente_ = false;
  }
}

void EstadoResultado::desenharAvisoEnvio(TFT_eSPI& tft, Envio estado) {
  limparFaixa(tft, 0, 184, 320, 16);  // restaura o fundo (sem caixa preta)
  if (estado == Envio::ENVIANDO) {
    tft.setTextColor(TFT_RED);
    tft.drawCentreString("Realizando envio do resultado...", 160, 186, 1);
  } else if (estado == Envio::FALHOU) {
    tft.setTextColor(TFT_RED);
    tft.drawCentreString("Falha ao enviar resultado", 160, 186, 1);
  }
}

void EstadoResultado::renderizar(TFT_eSPI& tft) {
  if (sujo_) {
    tft.fillScreen(TFT_BLACK);
    draw_jpegFromSD(kFundo, 0, 0);

    Batalha* b = (controlador_ != nullptr) ? controlador_->getBatalha() : nullptr;
    Jogador* j1 = (controlador_ != nullptr) ? controlador_->getJogador1() : nullptr;
    Jogador* j2 = (controlador_ != nullptr) ? controlador_->getJogador2() : nullptr;
    IJogador* v = (b != nullptr) ? b->getVencedor() : nullptr;
    const char* nomeJ1 = j1 != nullptr ? j1->getNome() : "J1";
    const char* nomeJ2 = j2 != nullptr ? j2->getNome() : "J2";

    char buf[64];
    snprintf(buf, sizeof(buf), "Vencedor: %s", v != nullptr ? v->getNome() : "-");
    tft.setTextColor(TFT_RED);
    tft.drawCentreString(buf, 160, 48, 4);

    snprintf(buf, sizeof(buf), "Historico  (esq=%s  dir=%s)", nomeJ1, nomeJ2);
    tft.setTextColor(TFT_BLUE);
    tft.drawString(buf, 14, 78, 1);

    if (b != nullptr) {
      int y = 90;
      for (int i = 0; i < b->getQuantidadeRodadas() && y < 182; ++i) {
        const Rodada& r = b->getRodada(i);
        snprintf(buf, sizeof(buf), "R%d: %s x %s  ->  %s", r.numero,
                 nomePoke(r.pokemonJogador1), nomePoke(r.pokemonJogador2),
                 nomePoke(r.pokemonVencedor));
        tft.setTextColor(TFT_BLACK);
        tft.drawString(buf, 16, y, 1);
        y += 11;
      }
    }

    sujo_ = false;
    avisoMostrado_ = -1;
  }

  Envio desejado;
  if (pendente_) {
    desejado = Envio::ENVIANDO;
  } else if (!tentouEnviar_ || envioOk_) {
    desejado = Envio::CONCLUIDO;
  } else {
    desejado = Envio::FALHOU;
  }
  if (static_cast<int>(desejado) != avisoMostrado_) {
    desenharAvisoEnvio(tft, desejado);
    avisoMostrado_ = static_cast<int>(desejado);
  }

  renderizado_ = true;
}
