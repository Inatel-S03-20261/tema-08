#include "infra/carregador.h"

#include <cstring>

#include "dominio/Aleatorio.h"
#include "dominio/AtaqueCritico.h"
#include "dominio/AtaqueNormal.h"
#include "dominio/AtaqueSuperEfetivo.h"
#include "dominio/Jogador.h"
#include "dominio/Pokemon.h"
#include "infra/sd_card.h"

namespace {

const int kMaxNomes = 96;
char nomesPool[kMaxNomes][32];
int nomeCount = 0;

const char* persistirNome(const char* s) {
  if (nomeCount >= kMaxNomes) return s;
  strncpy(nomesPool[nomeCount], s, 31);
  nomesPool[nomeCount][31] = '\0';
  return nomesPool[nomeCount++];
}

const int kCustoSpecial = 50;
const int kCustoStrong = 40;
const int kCustoMedium = 30;
const int kCustoWeak = 20;
const float kMultSuperEfetivo = 1.5f;
const float kMultCritico = 2.0f;
const int kEscalaVida = 2;
const int kVidaMinima = 50;
const int kEnergiaMaxima = 100;

Ataque* criarAtaque(const Move& m, const char* tipo, Aleatorio& rng,
                    int custoForcado) {
  const char* nome = persistirNome(m.name);
  int poder = m.power;
  if (strcmp(m.category, "special") == 0) {
    int custo = custoForcado >= 0 ? custoForcado : kCustoSpecial;
    return new AtaqueSuperEfetivo(nome, tipo, custo, poder, kMultSuperEfetivo);
  }
  if (strcmp(m.category, "strong") == 0) {
    int custo = custoForcado >= 0 ? custoForcado : kCustoStrong;
    return new AtaqueCritico(nome, tipo, custo, poder, kMultCritico, rng);
  }
  int custo = custoForcado >= 0
                  ? custoForcado
                  : ((strcmp(m.category, "medium") == 0) ? kCustoMedium : kCustoWeak);
  return new AtaqueNormal(nome, tipo, custo, poder);
}

}  // namespace

void montarPokemon(const PokemonData& dados, const char* folder,
                   Pokemon& destino, Aleatorio& rng) {
  int vidaMax = dados.hp * kEscalaVida;
  if (vidaMax < kVidaMinima) vidaMax = kVidaMinima;
  int defesa = dados.def < 1 ? 1 : dados.def;
  destino.configurar(dados.name, folder, dados.type1, dados.type2, vidaMax,
                     defesa, kEnergiaMaxima);

  const char* tipoGolpe =
      destino.getQuantidadeTipos() > 0 ? destino.getTipo(0) : "normal";
  const int kMaxGolpes = sizeof(dados.moves) / sizeof(dados.moves[0]);
  for (int i = 0; i < kMaxGolpes; i++) {
    if (dados.moves[i].name[0] == '\0') continue;
    int custoForcado = (i == 0) ? 0 : -1;
    destino.adicionarAtaque(
        criarAtaque(dados.moves[i], tipoGolpe, rng, custoForcado));
  }
}

bool carregarPokemonDoSD(const char* folder, Pokemon& destino, Aleatorio& rng) {
  PokemonData dados;
  if (!load_pokemon(folder, dados)) return false;
  montarPokemon(dados, folder, destino, rng);
  return true;
}

int carregarPoolDoSD(Pokemon* pool, int maxPool, Aleatorio& rng) {
  int n = 0;
  for (int i = 0; i < pokemonCount && n < maxPool; i++) {
    if (carregarPokemonDoSD(pokemonFolders[i], pool[n], rng)) n++;
  }
  return n;
}

void distribuirAleatorio(Jogador& j1, Jogador& j2, Pokemon* pool, int n,
                         int qtdCada, Aleatorio& rng) {
  const int kMax = 64;
  if (n > kMax) n = kMax;
  int idx[kMax];
  for (int i = 0; i < n; ++i) idx[i] = i;
  for (int i = n - 1; i > 0; --i) {  // Fisher-Yates
    int k = rng.proximo(i + 1);
    int t = idx[i];
    idx[i] = idx[k];
    idx[k] = t;
  }
  j1.limparCartas();
  j2.limparCartas();
  int pos = 0;
  for (int c = 0; c < qtdCada && pos < n; ++c, ++pos) {
    pool[idx[pos]].restaurar();
    j1.adicionarCarta(&pool[idx[pos]]);
  }
  for (int c = 0; c < qtdCada && pos < n; ++c, ++pos) {
    pool[idx[pos]].restaurar();
    j2.adicionarCarta(&pool[idx[pos]]);
  }
}
