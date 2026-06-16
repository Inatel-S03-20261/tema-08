#pragma once

#include "app/Estado.h"

class Pokemon;
class Aleatorio;
class IAutenticador;

class EstadoLogin : public Estado {
 public:
  void configurarRecursos(Pokemon* pool, int n, Aleatorio* rng);
  // Opcional: liga o login real (Tema-01). Se nao configurado, mock segue.
  void configurarAutenticador(IAutenticador* autenticador);

  void aoAbrir() override;
  void atualizar() override;
  void tratarEvento() override {}
  void renderizar(TFT_eSPI& tft) override;

 private:
  void finalizar();
  void tentarAutenticar(const char* usuario, const char* senha);

  IAutenticador* autenticador_ = nullptr;

  enum Etapa { NICK1 = 0, SENHA1, NICK2, SENHA2 };
  Etapa etapa_ = NICK1;
  char buffer_[24] = {0};
  int len_ = 0;
  char nick1_[24] = {0};
  char nick2_[24] = {0};

  Pokemon* pool_ = nullptr;
  int poolN_ = 0;
  Aleatorio* rng_ = nullptr;

  bool redesenhoTotal_ = true;
  int ultTrainer_ = -1;
};
