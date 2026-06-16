#include <unity.h>

#include "FakeJogador.h"
#include "FakePokemon.h"
#include "dominio/Aleatorio.h"
#include "dominio/AtaqueCritico.h"
#include "dominio/AtaqueNormal.h"
#include "dominio/AtaqueSuperEfetivo.h"
#include "dominio/Batalha.h"
#include "dominio/TabelaTipos.h"

void setUp(void) {}
void tearDown(void) {}

// --- Ataques ---

void test_ataque_normal_dano(void) {
  FakePokemon atk("A", 30, 0, 5, 5, "normal");
  FakePokemon def("D", 30, 2, 5, 5, "normal");
  AtaqueNormal a("Tackle", "normal", 1, 10);
  // dano = 10 * 100 / (100 + 2) = 9
  TEST_ASSERT_EQUAL_INT(9, a.calcularDano(atk, def));
}

void test_ataque_critico_deterministico(void) {
  FakePokemon atk("A", 30, 0, 5, 5, "normal");
  FakePokemon def("D", 30, 0, 5, 5, "normal");

  Aleatorio rngCritico(3);  // semente cuja 1a chance(20) e true
  AtaqueCritico crit("Bite", "normal", 1, 10, 2.0f, rngCritico);
  TEST_ASSERT_EQUAL_INT(20, crit.calcularDano(atk, def));  // 10 * 2.0

  Aleatorio rngSemCritico(2);  // semente cuja 1a chance(20) e false
  AtaqueCritico semCrit("Bite", "normal", 1, 10, 2.0f, rngSemCritico);
  TEST_ASSERT_EQUAL_INT(10, semCrit.calcularDano(atk, def));
}

void test_ataque_super_efetivo(void) {
  FakePokemon atk("A", 30, 0, 5, 5, "fire");
  FakePokemon grass("G", 30, 0, 5, 5, "grass");
  FakePokemon water("W", 30, 0, 5, 5, "water");
  AtaqueSuperEfetivo s("Ember", "fire", 1, 10, 1.5f);
  // fire super efetivo contra grass -> 10 * 1.5 = 15
  TEST_ASSERT_EQUAL_INT(15, s.calcularDano(atk, grass));
  // fire nao super contra water -> poder normal
  TEST_ASSERT_EQUAL_INT(10, s.calcularDano(atk, water));
}

// --- Tabela de tipos ---

void test_tabela_tipos(void) {
  using namespace TabelaTipos;
  TEST_ASSERT_TRUE(superEfetivoContra(Tipo::FIRE, Tipo::GRASS));
  TEST_ASSERT_TRUE(superEfetivoContra(Tipo::WATER, Tipo::FIRE));
  TEST_ASSERT_TRUE(superEfetivoContra(Tipo::ELECTRIC, Tipo::WATER));
  TEST_ASSERT_FALSE(superEfetivoContra(Tipo::FIRE, Tipo::WATER));
  TEST_ASSERT_FALSE(superEfetivoContra(Tipo::NORMAL, Tipo::NORMAL));
  // case-insensitive
  TEST_ASSERT_EQUAL_INT(static_cast<int>(Tipo::FIRE),
                        static_cast<int>(tipoDeTexto("Fire")));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(Tipo::DESCONHECIDO),
                        static_cast<int>(tipoDeTexto("xyz")));
}

// --- Energia ---

void test_energia_gasta_no_executar(void) {
  FakePokemon atk("A", 30, 0, 5, 5, "normal");
  FakePokemon def("D", 30, 0, 5, 5, "normal");
  AtaqueNormal a("Tackle", "normal", 2, 10);
  ResultadoAtaque r = a.executar(atk, def);
  TEST_ASSERT_EQUAL_INT(10, r.dano);
  TEST_ASSERT_EQUAL_INT(3, atk.getEnergia());  // 5 - 2
  TEST_ASSERT_EQUAL_INT(20, def.getVida());    // 30 - 10
}

void test_executar_reporta_flags(void) {
  Aleatorio rng(3);  // semente cuja 1a chance(20) e true (critico)
  FakePokemon atk("A", 30, 0, 9, 9, "fire");
  FakePokemon grass("G", 60, 0, 5, 5, "grass");

  AtaqueCritico crit("Bite", "normal", 1, 10, 2.0f, rng);
  TEST_ASSERT_TRUE(crit.executar(atk, grass).critico);

  AtaqueSuperEfetivo sup("Ember", "fire", 1, 10, 1.5f);
  TEST_ASSERT_TRUE(sup.executar(atk, grass).superEfetivo);  // fire vs grass

  AtaqueNormal nrm("Tackle", "normal", 1, 10);
  ResultadoAtaque r = nrm.executar(atk, grass);
  TEST_ASSERT_FALSE(r.critico);
  TEST_ASSERT_FALSE(r.superEfetivo);
}

void test_batalha_energia_insuficiente(void) {
  Aleatorio rng(2);  // proximo(2)=0 -> J1 comeca
  // p1 com energia 1 e maxima 1 (regen nao ajuda) e golpe de custo 2.
  AtaqueNormal caro("Caro", "normal", 2, 10);
  FakePokemon p1("P1", 10, 0, 1, 1, "normal");
  p1.adicionarAtaque(&caro);
  AtaqueNormal barato("Barato", "normal", 1, 10);
  FakePokemon q1("Q1", 10, 0, 5, 5, "normal");
  q1.adicionarAtaque(&barato);
  FakeJogador j1("J1");
  j1.adicionarCarta(&p1);
  FakeJogador j2("J2");
  j2.adicionarCarta(&q1);

  Batalha b(j1, j2, rng);
  b.iniciar();
  TEST_ASSERT_TRUE(b.vezDoJogador1());

  int r = b.executarTurno(0);
  TEST_ASSERT_EQUAL_INT(-1, r);            // golpe rejeitado
  TEST_ASSERT_EQUAL_INT(10, q1.getVida());  // defensor intacto
  TEST_ASSERT_FALSE(b.batalhaTerminou());
  TEST_ASSERT_TRUE(b.vezDoJogador1());  // ainda a vez do J1
}

// --- Fluxo completo da batalha ---

void test_batalha_fluxo_rodadas(void) {
  Aleatorio rng(2);  // proximo(2)=0 -> J1 comeca
  AtaqueNormal letal("Letal", "normal", 1, 10);  // mata em 1 golpe (hp 10)
  FakePokemon p1("P1", 10, 0, 5, 5, "normal");
  p1.adicionarAtaque(&letal);
  FakePokemon p2("P2", 10, 0, 5, 5, "normal");
  p2.adicionarAtaque(&letal);
  FakePokemon q1("Q1", 10, 0, 5, 5, "normal");
  q1.adicionarAtaque(&letal);
  FakePokemon q2("Q2", 10, 0, 5, 5, "normal");
  q2.adicionarAtaque(&letal);
  FakeJogador j1("J1");
  j1.adicionarCarta(&p1);
  j1.adicionarCarta(&p2);
  FakeJogador j2("J2");
  j2.adicionarCarta(&q1);
  j2.adicionarCarta(&q2);

  Batalha b(j1, j2, rng);
  b.iniciar();
  TEST_ASSERT_TRUE(b.vezDoJogador1());

  // T1: p1 mata q1. Fim da rodada 1; J2 (perdedor) deve ESCOLHER o proximo.
  TEST_ASSERT_EQUAL_INT(10, b.executarTurno(0));
  TEST_ASSERT_EQUAL_INT(1, b.getQuantidadeRodadas());
  TEST_ASSERT_TRUE(b.rodadaTrocouNoUltimoTurno());
  TEST_ASSERT_EQUAL_INT(2, b.getNumeroRodadaAtual());
  TEST_ASSERT_TRUE(b.aguardandoSelecao());
  TEST_ASSERT_EQUAL_PTR(&j2, b.jogadorParaSelecionar());
  TEST_ASSERT_FALSE(b.batalhaTerminou());
  TEST_ASSERT_EQUAL_PTR(&p1, b.getRodada(0).pokemonVencedor);
  // Nao da para atacar antes de escolher o proximo Pokemon.
  TEST_ASSERT_EQUAL_INT(-1, b.executarTurno(0));
  // J2 escolhe q2 -> a luta retoma com J2 comecando.
  b.escolherProximoPokemon(j2, q2);
  TEST_ASSERT_FALSE(b.aguardandoSelecao());
  TEST_ASSERT_FALSE(b.vezDoJogador1());

  // T2: q2 mata p1. Fim da rodada 2; J1 deve escolher.
  TEST_ASSERT_EQUAL_INT(10, b.executarTurno(0));
  TEST_ASSERT_EQUAL_INT(2, b.getQuantidadeRodadas());
  TEST_ASSERT_EQUAL_INT(3, b.getNumeroRodadaAtual());
  TEST_ASSERT_TRUE(b.aguardandoSelecao());
  TEST_ASSERT_EQUAL_PTR(&j1, b.jogadorParaSelecionar());
  b.escolherProximoPokemon(j1, p2);
  TEST_ASSERT_TRUE(b.vezDoJogador1());

  // T3: p2 mata q2. J2 sem Pokemons -> batalha encerra, vence J1.
  TEST_ASSERT_EQUAL_INT(10, b.executarTurno(0));
  TEST_ASSERT_TRUE(b.batalhaTerminou());
  TEST_ASSERT_EQUAL_PTR(&j1, b.getVencedor());
  TEST_ASSERT_EQUAL_INT(3, b.getQuantidadeRodadas());
}

void test_batalha_vencedor_mantem_hp(void) {
  Aleatorio rng(2);  // proximo(2)=0 -> J1 comeca
  AtaqueNormal golpe("Golpe", "normal", 1, 6);  // 2 golpes para matar (hp 10)
  FakePokemon p1("P1", 10, 0, 5, 5, "normal");
  p1.adicionarAtaque(&golpe);
  FakePokemon q1("Q1", 10, 0, 5, 5, "normal");
  q1.adicionarAtaque(&golpe);
  FakeJogador j1("J1");
  j1.adicionarCarta(&p1);
  FakeJogador j2("J2");
  j2.adicionarCarta(&q1);

  Batalha b(j1, j2, rng);
  b.iniciar();

  b.executarTurno(0);  // p1 -> q1: 4 de vida
  TEST_ASSERT_EQUAL_INT(4, q1.getVida());
  TEST_ASSERT_FALSE(b.batalhaTerminou());
  TEST_ASSERT_FALSE(b.vezDoJogador1());

  b.executarTurno(0);  // q1 -> p1: 4 de vida
  TEST_ASSERT_EQUAL_INT(4, p1.getVida());
  TEST_ASSERT_TRUE(b.vezDoJogador1());

  b.executarTurno(0);  // p1 -> q1: morre; vence J1, p1 mantem 4 de vida
  TEST_ASSERT_TRUE(b.batalhaTerminou());
  TEST_ASSERT_EQUAL_PTR(&j1, b.getVencedor());
  TEST_ASSERT_EQUAL_INT(4, p1.getVida());
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_ataque_normal_dano);
  RUN_TEST(test_ataque_critico_deterministico);
  RUN_TEST(test_ataque_super_efetivo);
  RUN_TEST(test_tabela_tipos);
  RUN_TEST(test_energia_gasta_no_executar);
  RUN_TEST(test_executar_reporta_flags);
  RUN_TEST(test_batalha_energia_insuficiente);
  RUN_TEST(test_batalha_fluxo_rodadas);
  RUN_TEST(test_batalha_vencedor_mantem_hp);
  return UNITY_END();
}
