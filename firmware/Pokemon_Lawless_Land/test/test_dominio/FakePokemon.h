#pragma once

#include "contratos/IPokemon.h"
#include "dominio/Ataque.h"

// Pokemon falso para os testes (energiaMax separada da energia inicial).
class FakePokemon : public IPokemon {
 public:
  FakePokemon(const char* nome, int vida, int defesa, int energia,
              int energiaMax, const char* tipo0, const char* tipo1 = nullptr)
      : nome_(nome),
        vida_(vida),
        vidaMax_(vida),
        defesa_(defesa),
        energia_(energia),
        energiaMax_(energiaMax),
        qtdTipos_(tipo1 != nullptr ? 2 : 1),
        qtdAtaques_(0) {
    tipos_[0] = tipo0;
    tipos_[1] = tipo1;
  }

  void adicionarAtaque(Ataque* a) {
    if (qtdAtaques_ < kMaxAtaques) ataques_[qtdAtaques_++] = a;
  }

  const char* getNome() const override { return nome_; }
  const char* getPastaAssets() const override { return ""; }
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
  static const int kMaxAtaques = 4;
  const char* nome_;
  int vida_, vidaMax_, defesa_, energia_, energiaMax_;
  int qtdTipos_;
  const char* tipos_[2];
  Ataque* ataques_[kMaxAtaques];
  int qtdAtaques_;
};
