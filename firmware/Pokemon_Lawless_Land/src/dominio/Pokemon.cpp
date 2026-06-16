#include "dominio/Pokemon.h"

#include <cstring>

namespace {
void copiar(char* destino, const char* origem, int tamanho) {
  if (origem == nullptr) {
    destino[0] = '\0';
    return;
  }
  strncpy(destino, origem, tamanho - 1);
  destino[tamanho - 1] = '\0';
}
}  // namespace

Pokemon::Pokemon()
    : qtdTipos_(0),
      vida_(1),
      vidaMax_(1),
      defesa_(0),
      energia_(0),
      energiaMax_(0),
      qtdAtaques_(0) {
  nome_[0] = '\0';
  pastaAssets_[0] = '\0';
}

void Pokemon::configurar(const char* nome, const char* pastaAssets,
                         const char* tipo0, const char* tipo1, int vidaMaxima,
                         int defesa, int energiaMaxima) {
  copiar(nome_, nome, sizeof(nome_));
  copiar(pastaAssets_, pastaAssets, sizeof(pastaAssets_));

  qtdTipos_ = 0;
  if (tipo0 != nullptr && tipo0[0] != '\0') {
    copiar(tipos_[0], tipo0, sizeof(tipos_[0]));
    qtdTipos_ = 1;
  }
  if (tipo1 != nullptr && tipo1[0] != '\0' && qtdTipos_ < kMaxTipos) {
    copiar(tipos_[qtdTipos_], tipo1, sizeof(tipos_[qtdTipos_]));
    qtdTipos_++;
  }

  vidaMax_ = vidaMaxima < 1 ? 1 : vidaMaxima;
  vida_ = vidaMax_;
  defesa_ = defesa;
  energiaMax_ = energiaMaxima < 0 ? 0 : energiaMaxima;
  energia_ = energiaMax_;
  qtdAtaques_ = 0;
}

void Pokemon::adicionarAtaque(Ataque* ataque) {
  if (ataque != nullptr && qtdAtaques_ < kMaxAtaques) {
    ataques_[qtdAtaques_++] = ataque;
  }
}

void Pokemon::restaurar() {
  vida_ = vidaMax_;
  energia_ = energiaMax_;
}
