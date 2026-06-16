#ifndef INFRA_SD_CARD_H_
#define INFRA_SD_CARD_H_

#include <Arduino.h>
#include <TFT_eSPI.h>

#define SD_CS 5
#define MAX_POKEMONS 64

struct Move {
  char name[32];
  int power;
  char category[10];
};

struct PokemonData {
  char name[32];
  char type1[16];
  char type2[16];
  int hp;
  int atk;
  int def;
  int sdef;
  int spd;
  Move moves[4];
};

extern char pokemonFolders[MAX_POKEMONS][32];
extern int pokemonCount;
extern TFT_eSPI tft;

bool SD_init();
void scan_pokemonsOnSD();
bool load_pokemon(const char* folder, PokemonData& p);

bool draw_jpegFromSD(const char* path, int x, int y);
bool restore_backgroundRegion(int x, int y, int width, int height);
bool restore_jpegRegion(const char* path, int x, int y, int width, int height);
bool draw_pokemonImage(const char* pastaAssets, bool usarCostas, int x, int y);

#endif  // INFRA_SD_CARD_H_
