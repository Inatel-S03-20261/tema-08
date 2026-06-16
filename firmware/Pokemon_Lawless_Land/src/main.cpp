#include <Arduino.h>
#include <TFT_eSPI.h>

#include "app/ControladorAplicacao.h"
#include "app/estados/EstadoBatalha.h"
#include "app/estados/EstadoLogin.h"
#include "app/estados/EstadoResultado.h"
#include "app/estados/EstadoSelecao.h"
#include "dominio/Batalha.h"
#include "dominio/Jogador.h"
#include "dominio/Pokemon.h"
#include "dominio/Aleatorio.h"
#include "infra/buttons.h"
#include "infra/carregador.h"
#include "infra/dados_demo.h"
#include "infra/sd_card.h"
#include "infra/AutenticadorHttp.h"
#include "infra/PublicadorMqtt.h"

TFT_eSPI tft = TFT_eSPI();

static Aleatorio rng;

static const int kPoolMax = 12;
static Pokemon pool[kPoolMax];
static Jogador jogador1;
static Jogador jogador2;
static Batalha* batalha = nullptr;

static EstadoLogin estadoLogin;
static EstadoSelecao estadoSelecao;
static EstadoBatalha estadoBatalha;
static EstadoResultado estadoResultado;
static PublicadorMqtt publicador;

static void erroFatal(const char* mensagem) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawCentreString(mensagem, 160, 110, 2);
  while (true) delay(1000);
}

void setup() {
  Serial.begin(115200);
  delay(300);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("Iniciando...", 160, 110, 2);

  if (!SD_init()) erroFatal("Erro no cartao SD");
  buttons_init();
  scan_pokemonsOnSD();

  rng.semear(esp_random());  // semente de hardware p/ o gerador aleatorio

  int n = carregarPoolDoSD(pool, kPoolMax, rng);
  if (n < 2) {
    Serial.println("SD sem dados; usando Pokemon de exemplo (flash).");
    n = carregarPoolDemo(pool, kPoolMax, rng);
  }
  if (n < 2) erroFatal("Sem pokemons");

  estadoLogin.configurarRecursos(pool, n, &rng);

  // Login real (Tema-01) via Adapter: descomente e ajuste o IP do auth-service.
  // O mock continua funcionando como fallback (nao bloqueia o demo offline).
  // static AutenticadorHttp autenticador("http://192.168.0.10:8080");
  // estadoLogin.configurarAutenticador(&autenticador);

  static Batalha batalhaObj(jogador1, jogador2, rng);
  batalha = &batalhaObj;

  ControladorAplicacao& c = ControladorAplicacao::instancia();
  c.registrarEstado(IdEstado::LOGIN, &estadoLogin);
  c.registrarEstado(IdEstado::SELECAO, &estadoSelecao);
  c.registrarEstado(IdEstado::BATALHA, &estadoBatalha);
  c.registrarEstado(IdEstado::RESULTADO, &estadoResultado);
  c.setJogadores(&jogador1, &jogador2);
  c.setBatalha(batalha);

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("Conectando WiFi...", 160, 110, 2);
  publicador.iniciar();
  estadoResultado.configurarPublicador(&publicador);

  c.iniciar(&tft, IdEstado::LOGIN);

  Serial.println("Pronto. Login: digite nick e senha no terminal + ENTER.");
}

void loop() {
  ControladorAplicacao& c = ControladorAplicacao::instancia();
  if (c.getEstadoAtualId() != IdEstado::LOGIN) {
    buttons_update();
  }
  c.tratarEvento();
  c.atualizar();
  c.renderizar();
}
