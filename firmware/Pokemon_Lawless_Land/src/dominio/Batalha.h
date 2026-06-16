#pragma once

#include "contratos/IJogador.h"
#include "dominio/Aleatorio.h"
#include "dominio/Ataque.h"
#include "dominio/Rodada.h"

struct LogTurno {
  bool valido = false;
  const char* nomeAtacante = nullptr;
  const char* nomeAtaque = nullptr;
  int dano = 0;
  bool critico = false;
  bool superEfetivo = false;
};

class Batalha {
 public:
  static const int kMaxRodadas = 16;
  static const int kEnergiaRegenPorTurno = 10;

  Batalha(IJogador& jogador1, IJogador& jogador2, Aleatorio& rng);

  void iniciar();

  bool vezDoJogador1() const { return vezDoJogador1_; }
  IJogador& jogadorDaVez() const;
  IJogador& oponenteDaVez() const;
  IPokemon& atacanteAtual() const;
  IPokemon& defensorAtual() const;

  bool podeUsar(const Ataque& ataque, const IPokemon& atacante) const {
    return atacante.getEnergia() >= ataque.getCusto();
  }
  bool algumAtaqueDisponivel(const IPokemon& pokemon) const;

  int executarTurno(int indiceAtaque);

  void passarTurno();

  void escolherProximoPokemon(IJogador& jogador, IPokemon& novo);

  bool batalhaTerminou() const { return batalhaTerminou_; }
  IJogador* getVencedor() const { return vencedor_; }
  bool rodadaTrocouNoUltimoTurno() const { return rodadaTrocou_; }

  bool aguardandoSelecao() const { return aguardandoSelecao_; }
  IJogador* jogadorParaSelecionar() const { return jogadorSelecao_; }

  const LogTurno& getUltimoLog() const { return ultimoLog_; }

  int getNumeroRodadaAtual() const { return numeroRodada_; }
  int getQuantidadeRodadas() const { return qtdRodadas_; }
  const Rodada& getRodada(int indice) const { return historico_[indice]; }

 private:
  void iniciarTurno();
  void passarVez();
  void regenerarEnergia(IPokemon& pokemon);
  void finalizarRodada();

  IJogador& jogador1_;
  IJogador& jogador2_;
  Aleatorio& rng_;

  bool iniciada_;
  bool vezDoJogador1_;
  bool batalhaTerminou_;
  bool rodadaTrocou_;
  bool aguardandoSelecao_;
  IJogador* jogadorSelecao_;
  IJogador* vencedor_;
  int numeroRodada_;

  Rodada historico_[kMaxRodadas];
  int qtdRodadas_;
  LogTurno ultimoLog_;
};
