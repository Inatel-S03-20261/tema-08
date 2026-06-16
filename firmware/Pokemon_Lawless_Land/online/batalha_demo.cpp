// =============================================================================
// batalha_demo.cpp  -  DEMO AUTOCONTIDA da engine de batalha (1 unico arquivo)
//
// Objetivo: rodar a logica de batalha em QUALQUER compilador C++ ONLINE, sem
// PlatformIO/Arduino. Cole este arquivo em um destes e clique "Run":
//   - OnlineGDB:  https://www.onlinegdb.com/  (linguagem: C++)
//   - Wandbox:    https://wandbox.org/         (gcc, -std=c++17)
//   - Godbolt:    https://godbolt.org/         (Execute the code)
//
// Local (com g++):  g++ -std=c++17 batalha_demo.cpp -o demo && ./demo
//
// ATENCAO: este e um ESPELHO condensado da engine para fins de demonstracao.
// A FONTE DE VERDADE e o codigo em src/ (dominio/Ataque, dominio/Batalha,
// dominio/TabelaTipos, contratos/*). As regras sao as mesmas; aqui as classes
// estao reunidas num arquivo so e ha um "AI" simples que joga a partida.
// =============================================================================
#include <cstdint>
#include <cstdio>

// ----------------------------------------------------------------------------
// IRandom (aleatoriedade abstrata) + um LCG deterministico para reproducao.
// ----------------------------------------------------------------------------
struct IRandom {
  virtual ~IRandom() {}
  virtual int proximo(int limite) = 0;
  bool chance(int porcentagem) { return proximo(100) < porcentagem; }
};
struct RngLCG : IRandom {
  uint32_t s;
  explicit RngLCG(uint32_t seed) : s(seed) {}
  int proximo(int limite) override {
    s = s * 1103515245u + 12345u;
    return limite > 0 ? static_cast<int>((s >> 16) % static_cast<uint32_t>(limite)) : 0;
  }
};

// ----------------------------------------------------------------------------
// Contratos (interfaces) — espelham contratos/IPokemon.h e IJogador.h
// ----------------------------------------------------------------------------
class Ataque;

struct IPokemon {
  virtual ~IPokemon() {}
  virtual const char* getNome() const = 0;
  virtual int getVida() const = 0;
  virtual void setVida(int v) = 0;
  virtual int getVidaMaxima() const = 0;
  virtual int getDefesa() const = 0;
  virtual int getEnergia() const = 0;
  virtual void setEnergia(int e) = 0;
  virtual int getEnergiaMaxima() const = 0;
  virtual int getQuantidadeTipos() const = 0;
  virtual const char* getTipo(int i) const = 0;
  virtual int getQuantidadeAtaques() const = 0;
  virtual Ataque* getAtaque(int i) const = 0;
  bool estaVivo() const { return getVida() > 0; }
};

struct IJogador {
  virtual ~IJogador() {}
  virtual const char* getNome() const = 0;
  virtual int getQuantidadeCartas() const = 0;
  virtual IPokemon* getCarta(int i) const = 0;
  virtual IPokemon* getPokemonAtual() const = 0;
  virtual void setPokemonAtual(IPokemon* p) = 0;
  bool temPokemonVivo() const {
    for (int i = 0; i < getQuantidadeCartas(); ++i) {
      IPokemon* c = getCarta(i);
      if (c != nullptr && c->estaVivo()) return true;
    }
    return false;
  }
};

// ----------------------------------------------------------------------------
// TabelaTipos — espelha dominio/TabelaTipos.cpp (tabela oficial Gen 6+)
// ----------------------------------------------------------------------------
namespace TabelaTipos {
enum class Tipo {
  NORMAL, FIRE, WATER, ELECTRIC, GRASS, ICE, FIGHTING, POISON, GROUND,
  FLYING, PSYCHIC, BUG, ROCK, GHOST, DRAGON, DARK, STEEL, FAIRY, DESCONHECIDO
};
static bool igualaSemCase(const char* a, const char* b) {
  if (!a || !b) return false;
  while (*a && *b) {
    char ca = *a, cb = *b;
    if (ca >= 'A' && ca <= 'Z') ca = char(ca - 'A' + 'a');
    if (cb >= 'A' && cb <= 'Z') cb = char(cb - 'A' + 'a');
    if (ca != cb) return false;
    ++a; ++b;
  }
  return *a == *b;
}
static Tipo tipoDeTexto(const char* slug) {
  struct Par { const char* n; Tipo t; };
  static const Par M[] = {
    {"normal",Tipo::NORMAL},{"fire",Tipo::FIRE},{"water",Tipo::WATER},
    {"electric",Tipo::ELECTRIC},{"grass",Tipo::GRASS},{"ice",Tipo::ICE},
    {"fighting",Tipo::FIGHTING},{"poison",Tipo::POISON},{"ground",Tipo::GROUND},
    {"flying",Tipo::FLYING},{"psychic",Tipo::PSYCHIC},{"bug",Tipo::BUG},
    {"rock",Tipo::ROCK},{"ghost",Tipo::GHOST},{"dragon",Tipo::DRAGON},
    {"dark",Tipo::DARK},{"steel",Tipo::STEEL},{"fairy",Tipo::FAIRY}};
  for (const Par& p : M) if (igualaSemCase(slug, p.n)) return p.t;
  return Tipo::DESCONHECIDO;
}
static bool ehSuperEfetivoTipo(Tipo a, Tipo d) {
  using T = Tipo;
  switch (a) {
    case T::NORMAL:   return false;
    case T::FIRE:     return d==T::GRASS||d==T::ICE||d==T::BUG||d==T::STEEL;
    case T::WATER:    return d==T::FIRE||d==T::GROUND||d==T::ROCK;
    case T::ELECTRIC: return d==T::WATER||d==T::FLYING;
    case T::GRASS:    return d==T::WATER||d==T::GROUND||d==T::ROCK;
    case T::ICE:      return d==T::GRASS||d==T::GROUND||d==T::FLYING||d==T::DRAGON;
    case T::FIGHTING: return d==T::NORMAL||d==T::ICE||d==T::ROCK||d==T::DARK||d==T::STEEL;
    case T::POISON:   return d==T::GRASS||d==T::FAIRY;
    case T::GROUND:   return d==T::FIRE||d==T::ELECTRIC||d==T::POISON||d==T::ROCK||d==T::STEEL;
    case T::FLYING:   return d==T::GRASS||d==T::FIGHTING||d==T::BUG;
    case T::PSYCHIC:  return d==T::FIGHTING||d==T::POISON;
    case T::BUG:      return d==T::GRASS||d==T::PSYCHIC||d==T::DARK;
    case T::ROCK:     return d==T::FIRE||d==T::ICE||d==T::FLYING||d==T::BUG;
    case T::GHOST:    return d==T::PSYCHIC||d==T::GHOST;
    case T::DRAGON:   return d==T::DRAGON;
    case T::DARK:     return d==T::PSYCHIC||d==T::GHOST;
    case T::STEEL:    return d==T::ICE||d==T::ROCK||d==T::FAIRY;
    case T::FAIRY:    return d==T::FIGHTING||d==T::DRAGON||d==T::DARK;
    default:          return false;
  }
}
static bool ehSuperEfetivo(const char* tipoAtaque, const IPokemon& def) {
  Tipo a = tipoDeTexto(tipoAtaque);
  if (a == Tipo::DESCONHECIDO) return false;
  for (int i = 0; i < def.getQuantidadeTipos(); ++i)
    if (ehSuperEfetivoTipo(a, tipoDeTexto(def.getTipo(i)))) return true;
  return false;
}
}  // namespace TabelaTipos

// ----------------------------------------------------------------------------
// Ataque (Template Method + polimorfismo) — espelha dominio/Ataque.*
// ----------------------------------------------------------------------------
class Ataque {
 public:
  Ataque(const char* n, const char* t, int custo, int poder)
      : nome_(n), tipo_(t), custo_(custo), poder_(poder) {}
  virtual ~Ataque() {}
  const char* getNome() const { return nome_; }
  const char* getTipo() const { return tipo_; }
  int getCusto() const { return custo_; }
  int getPoder() const { return poder_; }
  int executar(IPokemon& atk, IPokemon& def) const {
    int dano = calcularDano(atk, def);
    int en = atk.getEnergia() - custo_;
    atk.setEnergia(en < 0 ? 0 : en);
    int v = def.getVida() - dano;
    def.setVida(v < 0 ? 0 : v);
    return dano;
  }
  virtual int calcularDano(const IPokemon& atk, const IPokemon& def) const = 0;

 protected:
  static int aplicarDefesa(int bruto, const IPokemon& def) {
    int d = bruto - def.getDefesa();
    return d < 1 ? 1 : d;
  }
  const char* nome_; const char* tipo_; int custo_; int poder_;
};
class AtaqueNormal : public Ataque {
 public:
  AtaqueNormal(const char* n, const char* t, int c, int p) : Ataque(n, t, c, p) {}
  int calcularDano(const IPokemon&, const IPokemon& def) const override {
    return aplicarDefesa(poder_, def);
  }
};
class AtaqueCritico : public Ataque {
 public:
  static const int kChance = 20;
  AtaqueCritico(const char* n, const char* t, int c, int p, float mult, IRandom& rng)
      : Ataque(n, t, c, p), mult_(mult), rng_(rng) {}
  int calcularDano(const IPokemon&, const IPokemon& def) const override {
    int bruto = poder_;
    if (rng_.chance(kChance)) bruto = static_cast<int>(poder_ * mult_);
    return aplicarDefesa(bruto, def);
  }
 private:
  float mult_; IRandom& rng_;
};
class AtaqueSuperEfetivo : public Ataque {
 public:
  AtaqueSuperEfetivo(const char* n, const char* t, int c, int p, float mult)
      : Ataque(n, t, c, p), mult_(mult) {}
  int calcularDano(const IPokemon&, const IPokemon& def) const override {
    int bruto = poder_;
    if (TabelaTipos::ehSuperEfetivo(tipo_, def)) bruto = static_cast<int>(poder_ * mult_);
    return aplicarDefesa(bruto, def);
  }
 private:
  float mult_;
};

// ----------------------------------------------------------------------------
// Pokemon/Jogador concretos (papel do cartao SD) — espelha infra/DemoDados.h
// ----------------------------------------------------------------------------
class PokemonC : public IPokemon {
 public:
  PokemonC(const char* nome, int vida, int defesa, int energia,
           const char* t0, const char* t1 = nullptr)
      : nome_(nome), vida_(vida), vidaMax_(vida), defesa_(defesa),
        energia_(energia), energiaMax_(energia),
        qtdTipos_(t1 ? 2 : 1), qtdAtaques_(0) {
    tipos_[0] = t0; tipos_[1] = t1;
  }
  void add(Ataque* a) { if (qtdAtaques_ < 4) ataques_[qtdAtaques_++] = a; }
  const char* getNome() const override { return nome_; }
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
  const char* nome_; int vida_, vidaMax_, defesa_, energia_, energiaMax_;
  int qtdTipos_; const char* tipos_[2]; Ataque* ataques_[4]; int qtdAtaques_;
};
class JogadorC : public IJogador {
 public:
  explicit JogadorC(const char* nome) : nome_(nome), qtd_(0), atual_(nullptr) {}
  void add(IPokemon* p) { if (qtd_ < 6) cartas_[qtd_++] = p; }
  const char* getNome() const override { return nome_; }
  int getQuantidadeCartas() const override { return qtd_; }
  IPokemon* getCarta(int i) const override { return cartas_[i]; }
  IPokemon* getPokemonAtual() const override { return atual_; }
  void setPokemonAtual(IPokemon* p) override { atual_ = p; }
 private:
  const char* nome_; IPokemon* cartas_[6]; int qtd_; IPokemon* atual_;
};

// ----------------------------------------------------------------------------
// Batalha (engine) — espelha dominio/Batalha.*
// ----------------------------------------------------------------------------
class Batalha {
 public:
  static const int kMaxRodadas = 16;
  static const int kEnergiaRegenPorTurno = 1;
  Batalha(IJogador& j1, IJogador& j2, IRandom& rng)
      : j1_(j1), j2_(j2), rng_(rng), iniciada_(false), vezJ1_(true),
        terminou_(false), trocou_(false), vencedor_(nullptr),
        numRodada_(0), qtdRodadas_(0) {}
  void iniciar() {
    if (!j1_.getPokemonAtual() && j1_.getQuantidadeCartas() > 0) j1_.setPokemonAtual(j1_.getCarta(0));
    if (!j2_.getPokemonAtual() && j2_.getQuantidadeCartas() > 0) j2_.setPokemonAtual(j2_.getCarta(0));
    vencedor_ = nullptr; terminou_ = false; trocou_ = false;
    qtdRodadas_ = 0; numRodada_ = 1;
    vezJ1_ = (rng_.proximo(2) == 0);
    iniciada_ = true;
    iniciarTurno();
  }
  bool jogadorAtualEhJogador1() const { return vezJ1_; }
  IJogador& jogadorDaVez() const { return vezJ1_ ? j1_ : j2_; }
  IJogador& oponenteDaVez() const { return vezJ1_ ? j2_ : j1_; }
  IPokemon& atacanteAtual() const { return *jogadorDaVez().getPokemonAtual(); }
  IPokemon& defensorAtual() const { return *oponenteDaVez().getPokemonAtual(); }
  bool podeUsar(const Ataque& a, const IPokemon& atk) const { return atk.getEnergia() >= a.getCusto(); }
  bool algumAtaqueDisponivel(const IPokemon& p) const {
    for (int i = 0; i < p.getQuantidadeAtaques(); ++i) {
      const Ataque* a = p.getAtaque(i);
      if (a && p.getEnergia() >= a->getCusto()) return true;
    }
    return false;
  }
  int executarTurno(int idx) {
    trocou_ = false;
    if (!iniciada_ || terminou_) return -1;
    if (!jogadorDaVez().getPokemonAtual() || !oponenteDaVez().getPokemonAtual()) return -1;
    IPokemon& atk = atacanteAtual();
    if (idx < 0 || idx >= atk.getQuantidadeAtaques()) return -1;
    Ataque* a = atk.getAtaque(idx);
    if (!a || !podeUsar(*a, atk)) return -1;
    IPokemon& def = defensorAtual();
    int dano = a->executar(atk, def);
    if (!def.estaVivo()) finalizarRodada();
    else passarVez();
    return dano;
  }
  void passarTurno() { if (iniciada_ && !terminou_) passarVez(); }
  bool batalhaTerminou() const { return terminou_; }
  IJogador* getVencedor() const { return vencedor_; }
  bool rodadaTrocouNoUltimoTurno() const { return trocou_; }
  int getNumeroRodadaAtual() const { return numRodada_; }
  int getQuantidadeRodadas() const { return qtdRodadas_; }

 private:
  void iniciarTurno() {
    IPokemon* atk = jogadorDaVez().getPokemonAtual();
    if (atk) regenerar(*atk);
  }
  void passarVez() { vezJ1_ = !vezJ1_; iniciarTurno(); }
  void regenerar(IPokemon& p) {
    int nova = p.getEnergia() + kEnergiaRegenPorTurno;
    if (nova > p.getEnergiaMaxima()) nova = p.getEnergiaMaxima();
    p.setEnergia(nova);
  }
  void finalizarRodada() {
    if (qtdRodadas_ < kMaxRodadas) qtdRodadas_++;
    IJogador& perdedor = oponenteDaVez();
    if (!perdedor.temPokemonVivo()) { terminou_ = true; vencedor_ = &jogadorDaVez(); return; }
    numRodada_++; trocou_ = true;
    vezJ1_ = (&perdedor == &j1_);
    for (int i = 0; i < perdedor.getQuantidadeCartas(); ++i) {
      IPokemon* c = perdedor.getCarta(i);
      if (c && c->estaVivo()) { perdedor.setPokemonAtual(c); break; }
    }
    iniciarTurno();
  }
  IJogador& j1_; IJogador& j2_; IRandom& rng_;
  bool iniciada_, vezJ1_, terminou_, trocou_;
  IJogador* vencedor_;
  int numRodada_, qtdRodadas_;
};

// ----------------------------------------------------------------------------
// Demonstracao: monta duas equipes e joga a partida com "AI" simples
// (cada turno escolhe o golpe pagavel de maior poder), narrando o que acontece.
// ----------------------------------------------------------------------------
int main() {
  RngLCG rng(2024);

  AtaqueNormal       atkBrasa("Brasa", "fire", 1, 8);
  AtaqueSuperEfetivo atkChamas("Chamas", "fire", 2, 11, 1.5f);
  AtaqueCritico      atkMordida("Mordida", "normal", 2, 9, 2.0f, rng);
  AtaqueNormal       atkJato("Jato", "water", 1, 8);
  AtaqueSuperEfetivo atkHidro("Hidrojato", "water", 2, 11, 1.5f);
  AtaqueNormal       atkFolha("Folha", "grass", 1, 8);
  AtaqueSuperEfetivo atkChicote("Chicote", "grass", 2, 11, 1.5f);

  PokemonC charmander("Charmander", 30, 3, 4, "fire");
  charmander.add(&atkBrasa); charmander.add(&atkChamas); charmander.add(&atkMordida);
  PokemonC bulbasaur("Bulbasaur", 32, 4, 4, "grass");
  bulbasaur.add(&atkFolha); bulbasaur.add(&atkChicote); bulbasaur.add(&atkMordida);
  PokemonC squirtle("Squirtle", 34, 5, 4, "water");
  squirtle.add(&atkJato); squirtle.add(&atkHidro); squirtle.add(&atkMordida);
  PokemonC poliwag("Poliwag", 30, 3, 4, "water");
  poliwag.add(&atkJato); poliwag.add(&atkMordida);

  JogadorC ash("Ash");   ash.add(&charmander); ash.add(&bulbasaur);
  JogadorC gary("Gary"); gary.add(&squirtle);  gary.add(&poliwag);

  Batalha b(ash, gary, rng);
  b.iniciar();
  printf("=== BATALHA: %s vs %s ===\n", ash.getNome(), gary.getNome());
  printf("Sorteio: comeca %s\n\n", b.jogadorDaVez().getNome());

  int passos = 0;
  while (!b.batalhaTerminou() && passos < 300) {
    IJogador& atkJog = b.jogadorDaVez();
    IPokemon& pa = b.atacanteAtual();
    IPokemon& pd = b.defensorAtual();
    int rodada = b.getNumeroRodadaAtual();

    // escolhe o golpe pagavel de maior poder
    int idx = -1, melhor = -1;
    for (int i = 0; i < pa.getQuantidadeAtaques(); ++i) {
      Ataque* a = pa.getAtaque(i);
      if (pa.getEnergia() >= a->getCusto() && a->getPoder() > melhor) { melhor = a->getPoder(); idx = i; }
    }
    if (idx < 0) {
      printf("[R%d] %s(%s) sem energia -> passa a vez\n", rodada, atkJog.getNome(), pa.getNome());
      b.passarTurno(); passos++; continue;
    }

    Ataque* a = pa.getAtaque(idx);
    int hpAntes = pd.getVida();
    int dano = b.executarTurno(idx);
    int hpDepois = hpAntes - dano; if (hpDepois < 0) hpDepois = 0;
    printf("[R%d] %s: %s usa %-9s (%-6s c%d) em %-10s -> dano %2d  | HP %2d->%2d  EN(atk)=%d%s\n",
           rodada, atkJog.getNome(), pa.getNome(), a->getNome(), a->getTipo(), a->getCusto(),
           pd.getNome(), dano, hpAntes, hpDepois, pa.getEnergia(),
           (hpDepois == 0 ? "  *** KO ***" : ""));
    if (b.rodadaTrocouNoUltimoTurno())
      printf("    --- fim da rodada: troca de Pokemon; comeca a rodada %d ---\n", b.getNumeroRodadaAtual());
    passos++;
  }

  printf("\n=== VENCEDOR: %s  (em %d rodadas) ===\n",
         b.getVencedor() ? b.getVencedor()->getNome() : "(nenhum)", b.getQuantidadeRodadas());

  // Sanidade (smoke test): a batalha precisa concluir com um vencedor.
  if (!b.batalhaTerminou() || b.getVencedor() == nullptr) {
    printf("FALHA: a batalha nao concluiu corretamente.\n");
    return 1;
  }
  printf("OK: batalha concluida.\n");
  return 0;
}
