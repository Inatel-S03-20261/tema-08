#include "infra/dados_demo.h"

#include "dominio/Pokemon.h"
#include "infra/carregador.h"
#include "infra/sd_card.h"  // PokemonData / Move

namespace {

const PokemonData kDemo[] = {
    {"bulbasaur", "grass", "poison", 45, 49, 49, 0, 0,
     {{"Tackle", 40, "weak"}, {"Vine Whip", 45, "medium"},
      {"Take Down", 90, "strong"}, {"Solar Beam", 120, "special"}}},
    {"charmander", "fire", "", 39, 52, 43, 0, 0,
     {{"Scratch", 40, "weak"}, {"Slash", 70, "medium"},
      {"Flare Blitz", 120, "strong"}, {"Ember", 40, "special"}}},
    {"squirtle", "water", "", 44, 48, 65, 0, 0,
     {{"Tackle", 40, "weak"}, {"Headbutt", 70, "medium"},
      {"Skull Bash", 130, "strong"}, {"Water Gun", 40, "special"}}},
    {"pikachu", "electric", "", 35, 55, 40, 0, 0,
     {{"Double Kick", 30, "weak"}, {"Slam", 80, "medium"},
      {"Iron Tail", 100, "strong"}, {"Thunder Shock", 40, "special"}}},
    {"vulpix", "fire", "", 38, 41, 40, 0, 0,
     {{"Tackle", 40, "weak"}, {"Flame Wheel", 60, "medium"},
      {"Body Slam", 85, "strong"}, {"Ember", 40, "special"}}},
    {"oddish", "grass", "poison", 45, 50, 55, 0, 0,
     {{"Rage", 20, "weak"}, {"Razor Leaf", 55, "medium"},
      {"Take Down", 90, "strong"}, {"Acid", 40, "special"}}},
    {"psyduck", "water", "", 50, 52, 48, 0, 0,
     {{"Scratch", 40, "weak"}, {"Zen Headbutt", 80, "medium"},
      {"Aqua Tail", 90, "strong"}, {"Water Gun", 40, "special"}}},
    {"growlithe", "fire", "", 55, 70, 45, 0, 0,
     {{"Double Kick", 30, "weak"}, {"Bite", 60, "medium"},
      {"Take Down", 90, "strong"}, {"Ember", 40, "special"}}},
    {"machop", "fighting", "", 70, 80, 50, 0, 0,
     {{"Tackle", 40, "weak"}, {"Karate Chop", 50, "medium"},
      {"Double Edge", 120, "strong"}, {"Flamethrower", 90, "special"}}},
    {"geodude", "rock", "ground", 40, 80, 100, 0, 0,
     {{"Tackle", 40, "weak"}, {"Rock Throw", 50, "medium"},
      {"Take Down", 90, "strong"}, {"Mud Slap", 20, "special"}}},
};

const char* kDemoFolders[] = {
    "001_bulbasaur", "004_charmander", "007_squirtle", "025_pikachu",
    "037_vulpix",    "043_oddish",     "054_psyduck",  "058_growlithe",
    "066_machop",    "074_geodude",
};

}  // namespace

int carregarPoolDemo(Pokemon* pool, int maxPool, Aleatorio& rng) {
  int total = sizeof(kDemo) / sizeof(kDemo[0]);
  int n = 0;
  for (int i = 0; i < total && n < maxPool; ++i) {
    montarPokemon(kDemo[i], kDemoFolders[i], pool[n], rng);
    n++;
  }
  return n;
}
