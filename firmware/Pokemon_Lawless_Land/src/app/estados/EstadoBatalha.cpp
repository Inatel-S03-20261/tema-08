#include "app/estados/EstadoBatalha.h"

#include <cstdio>

#include "app/ControladorAplicacao.h"
#include "dominio/Ataque.h"
#include "dominio/Batalha.h"
#include "dominio/Jogador.h"
#include "infra/buttons.h"
#include "infra/sd_card.h"

namespace {

int contarVivos(IJogador& jogador) {
  int n = 0;
  for (int i = 0; i < jogador.getQuantidadeCartas(); ++i) {
    IPokemon* c = jogador.getCarta(i);
    if (c != nullptr && c->estaVivo()) ++n;
  }
  return n;
}

IPokemon* vivoNoIndice(IJogador& jogador, int indice) {
  int n = 0;
  for (int i = 0; i < jogador.getQuantidadeCartas(); ++i) {
    IPokemon* c = jogador.getCarta(i);
    if (c != nullptr && c->estaVivo()) {
      if (n == indice) return c;
      ++n;
    }
  }
  return nullptr;
}

void desenharBarra(TFT_eSPI& tft, int x, int y, int w, int h, int valor,
                   int maximo, uint16_t cor) {
  if (maximo < 1) maximo = 1;
  if (valor < 0) valor = 0;
  int fill = (valor * (w - 2)) / maximo;
  tft.drawRect(x, y, w, h, TFT_WHITE);
  tft.fillRect(x + 1, y + 1, w - 2, h - 2, TFT_DARKGREY);
  if (fill > 0) tft.fillRect(x + 1, y + 1, fill, h - 2, cor);
}

uint16_t corVida(int vida, int maximo) {
  if (maximo < 1) maximo = 1;
  int pct = (vida * 100) / maximo;
  if (pct > 50) return TFT_GREEN;
  if (pct > 25) return TFT_YELLOW;
  return TFT_RED;
}

void desenharStatus(TFT_eSPI& tft, int x, int y, IJogador& jog, IPokemon& p,
                    bool destaque) {
  char buf[40];
  tft.fillRect(x, y, 156, 40, TFT_BLACK);

  snprintf(buf, sizeof(buf), "%s%s - %s", destaque ? ">" : "", jog.getNome(),
           p.getNome());
  tft.setTextColor(destaque ? TFT_GREEN : TFT_WHITE);
  tft.drawString(buf, x, y, 1);

  desenharBarra(tft, x, y + 12, 150, 11, p.getVida(), p.getVidaMaxima(),
                corVida(p.getVida(), p.getVidaMaxima()));
  snprintf(buf, sizeof(buf), "%d/%d", p.getVida(), p.getVidaMaxima());
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString(buf, x + 75, y + 14, 1);

  desenharBarra(tft, x, y + 25, 150, 11, p.getEnergia(), p.getEnergiaMaxima(),
                TFT_BLUE);
  snprintf(buf, sizeof(buf), "EN %d/%d", p.getEnergia(), p.getEnergiaMaxima());
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString(buf, x + 75, y + 27, 1);
}

void desenharSprite(TFT_eSPI& tft, IPokemon& p, bool costas, int x, int y) {
  if (!restore_backgroundRegion(x, y, 96, 96))
    tft.fillRect(x, y, 96, 96, TFT_BLACK);
  draw_pokemonImage(p.getPastaAssets(), costas, x, y);
}

void desenharLinhaSelecao(TFT_eSPI& tft, IJogador& jog, int aliveIdx, bool sel) {
  IPokemon* c = vivoNoIndice(jog, aliveIdx);
  if (c == nullptr) return;
  int y = 44 + aliveIdx * 24;
  char buf[48];
  tft.fillRect(20, y - 2, 280, 20, sel ? TFT_NAVY : TFT_BLACK);
  snprintf(buf, sizeof(buf), "%c %s  HP %d/%d", sel ? '>' : ' ', c->getNome(),
           c->getVida(), c->getVidaMaxima());
  tft.setTextColor(TFT_WHITE);
  tft.drawString(buf, 28, y, 2);
}

}  // namespace

void EstadoBatalha::atualizarStatus(TFT_eSPI& tft, int x, int y, IJogador& jog,
                                    IPokemon& p, bool destaque,
                                    StatusCache& cache, bool forcar) {
  if (!forcar && cache.valido && cache.poke == &p && cache.jog == &jog &&
      cache.vida == p.getVida() && cache.energia == p.getEnergia() &&
      cache.destaque == destaque) {
    return;
  }
  desenharStatus(tft, x, y, jog, p, destaque);
  cache.poke = &p;
  cache.jog = &jog;
  cache.vida = p.getVida();
  cache.energia = p.getEnergia();
  cache.destaque = destaque;
  cache.valido = true;
}

void EstadoBatalha::aoAbrir() {
  sujo_ = true;
  batalha_ = (controlador_ != nullptr) ? controlador_->getBatalha() : nullptr;
  cursor_ = 0;
  cursorPokemon_ = 0;
  semEnergia_ = false;
  redesenhoTotal_ = true;
  ultFrameSelecao_ = false;
  selAtiva_ = false;
  ultCursorSel_ = -1;
  ultCima_ = nullptr;
  ultBaixo_ = nullptr;
  ultLogDesenhado_ = LogTurno();  // sentinela
  cacheCima_ = StatusCache();
  cacheBaixo_ = StatusCache();
}

void EstadoBatalha::tratarEvento() {
  if (batalha_ == nullptr) return;

  if (batalha_->batalhaTerminou()) {
    if (controlador_ != nullptr)
      controlador_->trocarEstado(IdEstado::RESULTADO);
    return;
  }

  bool houve = button_leftClicked() || button_rightClicked() ||
               button_selectClicked() || button_backClicked();
  if (!houve) return;

  if (batalha_->aguardandoSelecao()) {
    IJogador* jog = batalha_->jogadorParaSelecionar();
    int n = (jog != nullptr) ? contarVivos(*jog) : 0;
    if (button_leftClicked() && n > 0)
      cursorPokemon_ = (cursorPokemon_ - 1 + n) % n;
    if (button_rightClicked() && n > 0)
      cursorPokemon_ = (cursorPokemon_ + 1) % n;
    if (button_selectClicked() && jog != nullptr) {
      IPokemon* escolhido = vivoNoIndice(*jog, cursorPokemon_);
      if (escolhido != nullptr) {
        batalha_->escolherProximoPokemon(*jog, *escolhido);
        cursor_ = 0;
        cursorPokemon_ = 0;
        semEnergia_ = false;
      }
    }
    sujo_ = true;
    return;
  }

  IPokemon& atacante = batalha_->atacanteAtual();
  int n = atacante.getQuantidadeAtaques();
  if (button_leftClicked() && n > 0) {
    cursor_ = (cursor_ - 1 + n) % n;
    semEnergia_ = false;
  }
  if (button_rightClicked() && n > 0) {
    cursor_ = (cursor_ + 1) % n;
    semEnergia_ = false;
  }
  if (button_backClicked() && !batalha_->algumAtaqueDisponivel(atacante)) {
    batalha_->passarTurno();
    cursor_ = 0;
    semEnergia_ = false;
  }
  if (button_selectClicked()) {
    int dano = batalha_->executarTurno(cursor_);
    if (dano < 0) {
      semEnergia_ = true;
    } else {
      semEnergia_ = false;
      cursor_ = 0;
      if (batalha_->aguardandoSelecao()) cursorPokemon_ = 0;
      if (batalha_->batalhaTerminou() && controlador_ != nullptr) {
        controlador_->trocarEstado(IdEstado::RESULTADO);
        return;
      }
    }
  }
  sujo_ = true;
}

void EstadoBatalha::renderizar(TFT_eSPI& tft) {
  if (!sujo_ || batalha_ == nullptr) return;
  char buf[64];

  if (batalha_->batalhaTerminou()) {
    sujo_ = false;
    return;
  }

  if (batalha_->aguardandoSelecao()) {
    IJogador* jog = batalha_->jogadorParaSelecionar();
    int n = (jog != nullptr) ? contarVivos(*jog) : 0;
    if (!selAtiva_) {
      tft.fillScreen(TFT_BLACK);
      draw_jpegFromSD("/components/background.jpg", 0, 0);
      tft.fillRect(0, 0, 320, 28, TFT_BLACK);
      snprintf(buf, sizeof(buf), "%s: escolha o proximo",
               jog != nullptr ? jog->getNome() : "-");
      tft.setTextColor(TFT_WHITE);
      tft.drawCentreString(buf, 160, 6, 2);
      for (int i = 0; i < n; ++i)
        desenharLinhaSelecao(tft, *jog, i, i == cursorPokemon_);
      selAtiva_ = true;
    } else if (jog != nullptr && ultCursorSel_ != cursorPokemon_) {
      desenharLinhaSelecao(tft, *jog, ultCursorSel_, false);
      desenharLinhaSelecao(tft, *jog, cursorPokemon_, true);
    }
    ultCursorSel_ = cursorPokemon_;
    ultFrameSelecao_ = true;
    sujo_ = false;
    return;
  }

  selAtiva_ = false;
  IJogador* pj1 = controlador_ != nullptr ? controlador_->getJogador1() : nullptr;
  IJogador* pj2 = controlador_ != nullptr ? controlador_->getJogador2() : nullptr;
  if (pj1 == nullptr || pj2 == nullptr) {
    sujo_ = false;
    return;
  }
  bool vezJ1 = batalha_->vezDoJogador1();
  IJogador* jogBaixo = vezJ1 ? pj1 : pj2;
  IJogador* jogCima = vezJ1 ? pj2 : pj1;
  IPokemon* pBaixo = jogBaixo->getPokemonAtual();
  IPokemon* pCima = jogCima->getPokemonAtual();
  if (pBaixo == nullptr || pCima == nullptr) {
    sujo_ = false;
    return;
  }

  bool entrada = redesenhoTotal_ || ultFrameSelecao_;
  ultFrameSelecao_ = false;
  redesenhoTotal_ = false;
  if (entrada) {
    tft.fillScreen(TFT_BLACK);
    draw_jpegFromSD("/components/background.jpg", 0, 0);
    draw_pokemonImage(pCima->getPastaAssets(), false, 210, 6);   // oponente
    draw_pokemonImage(pBaixo->getPastaAssets(), true, 6, 106);   // ativo (costas)
  } else {
    if (pCima != ultCima_) desenharSprite(tft, *pCima, false, 210, 6);
    if (pBaixo != ultBaixo_) desenharSprite(tft, *pBaixo, true, 6, 106);
  }
  ultCima_ = pCima;
  ultBaixo_ = pBaixo;

  atualizarStatus(tft, 4, 4, *jogCima, *pCima, false, cacheCima_, entrada);
  atualizarStatus(tft, 120, 106, *jogBaixo, *pBaixo, true, cacheBaixo_, entrada);

  IPokemon& atk = batalha_->atacanteAtual();
  IJogador& jogVez = batalha_->jogadorDaVez();
  tft.fillRect(118, 150, 202, 54, TFT_BLACK);
  snprintf(buf, sizeof(buf), "Vez: %s", jogVez.getNome());
  tft.setTextColor(TFT_CYAN);
  tft.drawString(buf, 120, 150, 1);
  for (int i = 0; i < atk.getQuantidadeAtaques(); ++i) {
    Ataque* a = atk.getAtaque(i);
    if (a == nullptr) continue;
    bool sel = (i == cursor_);
    snprintf(buf, sizeof(buf), "%s%s (EN %d)", sel ? ">" : " ", a->getNome(),
             a->getCusto());
    tft.setTextColor(sel ? TFT_GREEN : TFT_WHITE);
    tft.drawString(buf, 120, 163 + i * 11, 1);
  }
  if (semEnergia_) {
    tft.setTextColor(TFT_RED);
    tft.drawString("Sem energia!", 120, 163 + 4 * 11, 1);
  }

  const LogTurno& log = batalha_->getUltimoLog();
  bool logMudou = entrada || log.valido != ultLogDesenhado_.valido ||
                  log.nomeAtaque != ultLogDesenhado_.nomeAtaque ||
                  log.nomeAtacante != ultLogDesenhado_.nomeAtacante ||
                  log.dano != ultLogDesenhado_.dano ||
                  log.critico != ultLogDesenhado_.critico ||
                  log.superEfetivo != ultLogDesenhado_.superEfetivo;
  if (logMudou) {
    tft.fillRect(0, 208, 320, 30, TFT_BLACK);
    if (log.valido) {
      snprintf(buf, sizeof(buf), "%s usou %s", log.nomeAtacante, log.nomeAtaque);
      tft.setTextColor(TFT_YELLOW);
      tft.drawString(buf, 4, 210, 1);
      const char* extra = log.critico        ? "FOI CRITICO "
                          : log.superEfetivo ? "FOI SUPER EFETIVO "
                                             : "";
      snprintf(buf, sizeof(buf), "%s(-%d de vida)", extra, log.dano);
      tft.setTextColor(log.critico || log.superEfetivo ? TFT_ORANGE : TFT_WHITE);
      tft.drawString(buf, 4, 222, 1);
    }
    ultLogDesenhado_ = log;
  }

  sujo_ = false;
}
