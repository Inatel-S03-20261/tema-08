/**
 * @file sd_card.h
 * @brief SD Card header file.
 *
 * @author Gustavo Pivoto Ambrósio
 * @date 2026-06-13
 */

#ifndef SD_CARD_H_
#define SD_CARD_H_

/* Includes ----------------------------------------------------------------- */
#include <Arduino.h>
#include <TFT_eSPI.h>

/* Defines ------------------------------------------------------------------ */
#define SD_CS 5
#define MAX_POKEMONS 64

/* Macros ------------------------------------------------------------------- */

/* Typedefs ----------------------------------------------------------------- */
struct Move {
    char name[32];
    int power;
    char category[10];
};

struct Pokemon {
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

/* Constants ---------------------------------------------------------------- */
extern char pokemonFolders[MAX_POKEMONS][32];
extern int pokemonCount;
extern TFT_eSPI tft;

/* Public function prototypes ----------------------------------------------- */
bool SD_init();

void scan_pokemonsOnSD();

bool load_pokemon(const char *folder, Pokemon &p);

void draw_jpegFromSD(const char *path, int x, int y);

#endif /* SD_CARD_H_ */
