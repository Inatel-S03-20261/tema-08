#include "dominio/Batalha.h"

Batalha::Batalha(IJogador& jogador1, IJogador& jogador2, Aleatorio& rng)
    : jogador1_(jogador1),
      jogador2_(jogador2),
      rng_(rng),
      iniciada_(false),
      vezDoJogador1_(true),
      batalhaTerminou_(false),
      rodadaTrocou_(false),
      aguardandoSelecao_(false),
      jogadorSelecao_(nullptr),
      vencedor_(nullptr),
      numeroRodada_(0),
      qtdRodadas_(0) {}

void Batalha::iniciar() {
  if (jogador1_.getPokemonAtual() == nullptr && jogador1_.getQuantidadeCartas() > 0) {
    jogador1_.setPokemonAtual(jogador1_.getCarta(0));
  }
  if (jogador2_.getPokemonAtual() == nullptr && jogador2_.getQuantidadeCartas() > 0) {
    jogador2_.setPokemonAtual(jogador2_.getCarta(0));
  }

  vencedor_ = nullptr;
  batalhaTerminou_ = false;
  rodadaTrocou_ = false;
  aguardandoSelecao_ = false;
  jogadorSelecao_ = nullptr;
  ultimoLog_ = LogTurno();
  qtdRodadas_ = 0;
  numeroRodada_ = 1;

  vezDoJogador1_ = (rng_.proximo(2) == 0);
  iniciada_ = true;

  iniciarTurno();
}

IJogador& Batalha::jogadorDaVez() const {
  return vezDoJogador1_ ? jogador1_ : jogador2_;
}

IJogador& Batalha::oponenteDaVez() const {
  return vezDoJogador1_ ? jogador2_ : jogador1_;
}

IPokemon& Batalha::atacanteAtual() const {
  return *jogadorDaVez().getPokemonAtual();
}

IPokemon& Batalha::defensorAtual() const {
  return *oponenteDaVez().getPokemonAtual();
}

bool Batalha::algumAtaqueDisponivel(const IPokemon& pokemon) const {
  for (int i = 0; i < pokemon.getQuantidadeAtaques(); ++i) {
    const Ataque* a = pokemon.getAtaque(i);
    if (a != nullptr && pokemon.getEnergia() >= a->getCusto()) return true;
  }
  return false;
}

int Batalha::executarTurno(int indiceAtaque) {
  rodadaTrocou_ = false;
  if (!iniciada_ || batalhaTerminou_) return -1;
  if (aguardandoSelecao_) return -1;
  if (jogadorDaVez().getPokemonAtual() == nullptr ||
      oponenteDaVez().getPokemonAtual() == nullptr) {
    return -1;
  }

  IPokemon& atacante = atacanteAtual();
  if (indiceAtaque < 0 || indiceAtaque >= atacante.getQuantidadeAtaques()) {
    return -1;
  }

  Ataque* ataque = atacante.getAtaque(indiceAtaque);
  if (ataque == nullptr || !podeUsar(*ataque, atacante)) {
    return -1;
  }

  IPokemon& defensor = defensorAtual();
  ResultadoAtaque r = ataque->executar(atacante, defensor);

  ultimoLog_.valido = true;
  ultimoLog_.nomeAtacante = atacante.getNome();
  ultimoLog_.nomeAtaque = ataque->getNome();
  ultimoLog_.dano = r.dano;
  ultimoLog_.critico = r.critico;
  ultimoLog_.superEfetivo = r.superEfetivo;

  if (!defensor.estaVivo()) {
    finalizarRodada();
  } else {
    passarVez();
  }
  return r.dano;
}

void Batalha::passarTurno() {
  if (!iniciada_ || batalhaTerminou_) return;
  passarVez();
}

void Batalha::escolherProximoPokemon(IJogador& jogador, IPokemon& novo) {
  jogador.setPokemonAtual(&novo);
  if (aguardandoSelecao_ && &jogador == jogadorSelecao_) {
    aguardandoSelecao_ = false;
    jogadorSelecao_ = nullptr;
    vezDoJogador1_ = (&jogador == &jogador1_);
    iniciarTurno();
  }
}

void Batalha::iniciarTurno() {
  IPokemon* atacante = jogadorDaVez().getPokemonAtual();
  if (atacante != nullptr) regenerarEnergia(*atacante);
}

void Batalha::passarVez() {
  vezDoJogador1_ = !vezDoJogador1_;
  iniciarTurno();
}

void Batalha::regenerarEnergia(IPokemon& pokemon) {
  int nova = pokemon.getEnergia() + kEnergiaRegenPorTurno;
  int maximo = pokemon.getEnergiaMaxima();
  if (nova > maximo) nova = maximo;
  pokemon.setEnergia(nova);
}

void Batalha::finalizarRodada() {
  if (qtdRodadas_ < kMaxRodadas) {
    Rodada& r = historico_[qtdRodadas_++];
    r.numero = numeroRodada_;
    r.pokemonJogador1 = jogador1_.getPokemonAtual();
    r.pokemonJogador2 = jogador2_.getPokemonAtual();
    r.pokemonVencedor = jogadorDaVez().getPokemonAtual();  // atacante venceu
  }

  IJogador& perdedor = oponenteDaVez();  // dono do Pokemon que caiu

  if (!perdedor.temPokemonVivo()) {
    batalhaTerminou_ = true;
    vencedor_ = &jogadorDaVez();
    return;
  }

  numeroRodada_++;
  rodadaTrocou_ = true;
  vezDoJogador1_ = (&perdedor == &jogador1_);
  aguardandoSelecao_ = true;
  jogadorSelecao_ = &perdedor;
}
