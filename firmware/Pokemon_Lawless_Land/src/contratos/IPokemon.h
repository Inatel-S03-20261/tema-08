#pragma once

class Ataque;

class IPokemon {
 public:
  virtual ~IPokemon() {}

  virtual const char* getNome() const = 0;

  virtual const char* getPastaAssets() const = 0;

  virtual int getVida() const = 0;
  virtual void setVida(int vida) = 0;
  virtual int getVidaMaxima() const = 0;

  virtual int getDefesa() const = 0;

  virtual int getEnergia() const = 0;
  virtual void setEnergia(int energia) = 0;
  virtual int getEnergiaMaxima() const = 0;

  virtual int getQuantidadeTipos() const = 0;
  virtual const char* getTipo(int indice) const = 0;

  virtual int getQuantidadeAtaques() const = 0;
  virtual Ataque* getAtaque(int indice) const = 0;

  bool estaVivo() const { return getVida() > 0; }
};
