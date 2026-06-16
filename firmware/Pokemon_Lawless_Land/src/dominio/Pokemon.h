#pragma once

#include "contratos/IPokemon.h"

class Ataque;

class Pokemon : public IPokemon {
 public:
  static const int kMaxAtaques = 4;
  static const int kMaxTipos = 2;

  Pokemon();

  void configurar(const char* nome, const char* pastaAssets, const char* tipo0,
                  const char* tipo1, int vidaMaxima, int defesa,
                  int energiaMaxima);
  void adicionarAtaque(Ataque* ataque);
  void restaurar();

  const char* getNome() const override { return nome_; }
  const char* getPastaAssets() const override { return pastaAssets_; }
  int getVida() const override { return vida_; }
  void setVida(int v) override { vida_ = v; }
  int getVidaMaxima() const override { return vidaMax_; }
  int getDefesa() const override { return defesa_; }
  int getEnergia() const override { return energia_; }
  void setEnergia(int e) override { energia_ = e; }
  int getEnergiaMaxima() const override { return energiaMax_; }
  int getQuantidadeTipos() const override { return qtdTipos_; }
  const char* getTipo(int i) const override { return tipos_[i]; }
  int getQuantidadeAtaques() const override { return qtdAtaques_; }
  Ataque* getAtaque(int i) const override { return ataques_[i]; }

 private:
  char nome_[32];
  char pastaAssets_[32];
  char tipos_[kMaxTipos][16];
  int qtdTipos_;
  int vida_, vidaMax_, defesa_, energia_, energiaMax_;
  Ataque* ataques_[kMaxAtaques];
  int qtdAtaques_;
};
