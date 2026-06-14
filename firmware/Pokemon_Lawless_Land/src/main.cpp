/**
 * @file main.cpp
 * @brief Main source file.
 *
 * @authors Gustavo Pivoto Ambrósio
 *          Maria Clara Pereira Campos
 *          Patrick Augusto Lins de Oliveira Damião
 *          Pedro Henrique de Paula Andrade
 * @date 2026-06-13
 */

/* Includes ----------------------------------------------------------------- */
#include <Arduino.h>

#include "TFT_eSPI.h"
#include "buttons.h"
#include "sd_card.h"

/* Private defines ---------------------------------------------------------- */
#define LED_BUILTIN  2

#define SPRITE_X     8
#define SPRITE_Y     40
#define SPRITE_W     96
#define SPRITE_H     96

#define INFO_X       82
#define INFO_Y       8

#define STATS_X      8
#define STATS_Y      115

#define MOVES_X      8
#define MOVES_Y      155

#define C_BG         0x0841
#define C_TEXT       0xFFFF
#define C_GOLD       0xFEA0
#define C_BORDER     0x4A69
#define C_PANEL      0x1082
#define C_HP_GREEN   0x07E0
#define C_HP_YELLOW  0xFFE0
#define C_HP_RED     0xF800
#define C_HP_BG      0x2104
#define C_WEAK       0x051F   
#define C_MEDIUM     0x0400
#define C_STRONG     0x6000
#define C_SPECIAL    0x400F

#define DISPLAY_TIME 5000

/* Boot sequence defines */
#define BOOT_LOGIN_TIME    3000
#define BOOT_BG_TIME       2000
#define BOOT_TRAINER_TIME  2000
#define BOOT_LOAD_TIME     2000

/* Private macros ----------------------------------------------------------- */

/* Private typedefs --------------------------------------------------------- */
enum BootState {
  BOOT_LOGIN = 0,
  BOOT_BACKGROUND,
  BOOT_TRAINER,
  BOOT_LOADING,
  BOOT_COMPLETE
};

/* Private constants -------------------------------------------------------- */

/* Private variables -------------------------------------------------------- */
TFT_eSPI tft = TFT_eSPI();

int currentIdx  = 0;
unsigned long lastSwitch = 0;

/* Boot sequence variables */
BootState bootState = BOOT_LOGIN;
unsigned long bootStateTime = 0;
bool bootComplete = false;


/* Private function prototypes ---------------------------------------------- */
uint16_t type_color(const char *type);
uint16_t move_color(const char *category);
void draw_statBar(int x, int y, int w, int h, int val, int maxVal, uint16_t color, const char *label);
void draw_typeBadge(int x, int y, const char *type);
void display_pokemon(const char *folder, Pokemon &p);

/* Boot sequence function prototypes */
void show_bootScreen();
void update_bootSequence();

/* Public functions --------------------------------------------------------- */
void setup() {
  Serial.begin(115200);
  delay(500);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(C_GOLD);
  tft.drawString("Iniciando...", 100, 115, 2);

  if(!SD_init()) {
    tft.setTextColor(TFT_RED);
    tft.drawString("Erro no SD!", 100, 140, 2);
    Serial.println("SD falhou!");
    while(true) delay(1000);
  }

  scan_pokemonsOnSD();

  if(pokemonCount == 0) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_RED);
    tft.drawString("Nenhum pokemon", 50, 100, 2);
    tft.drawString("encontrado no SD!", 50, 125, 2);
    while (true) delay(1000);
  }

  /* Initialize boot sequence */
  bootState = BOOT_LOGIN;
  bootStateTime = millis();
  bootComplete = false;
}

void loop() {
  /* Handle boot sequence */
  if(!bootComplete) {
    update_bootSequence();
    return;
  }

  /* Normal gameplay loop */
  if(millis() - lastSwitch >= DISPLAY_TIME || lastSwitch == 0) {
    lastSwitch = millis();

    Pokemon p;
    if(load_pokemon(pokemonFolders[currentIdx], p)) {
      display_pokemon(pokemonFolders[currentIdx], p);
      Serial.printf("Exibindo: %s (%d/%d)\n", p.name, currentIdx + 1, pokemonCount);
    } 
    
    else {
      Serial.printf("Erro ao carregar: %s\n", pokemonFolders[currentIdx]);
    }

    currentIdx = (currentIdx + 1) % pokemonCount;
  }
}

/* Private functions -------------------------------------------------------- */
uint16_t type_color(const char *type) {
  if(strcmp(type, "fire") == 0) return 0xFB00;
  if(strcmp(type, "water") == 0) return 0x045F;
  if(strcmp(type, "grass") == 0) return 0x0760;
  if(strcmp(type, "electric") == 0) return 0xFFE0;
  if(strcmp(type, "psychic") == 0) return 0xF81F;
  if(strcmp(type, "poison") == 0) return 0x8010;
  if(strcmp(type, "normal") == 0) return 0x8C51;
  if(strcmp(type, "flying") == 0) return 0x867F;
  if(strcmp(type, "ghost") == 0) return 0x400F;
  if(strcmp(type, "ice") == 0) return 0xAEFF;
  if(strcmp(type, "dragon") == 0) return 0x600F;
  if(strcmp(type, "fighting") == 0) return 0x8800;
  if(strcmp(type, "rock") == 0) return 0x8C40;
  if(strcmp(type, "ground") == 0) return 0xDBA0;
  if(strcmp(type, "bug") == 0) return 0x4600;
  return 0x4A69;
}

uint16_t move_color(const char *category) {
  if(strcmp(category, "weak") == 0) return C_WEAK;
  if(strcmp(category, "medium") == 0) return C_MEDIUM;
  if(strcmp(category, "strong") == 0) return C_STRONG;
  if(strcmp(category, "special") == 0) return C_SPECIAL;
  return C_PANEL;
}

void draw_statBar(int x, int y, int w, int h, int val, int maxVal, uint16_t color, const char *label) {
  int filled = constrain((int)((float)val / maxVal * w), 0, w);

  tft.setTextColor(C_TEXT);
  tft.drawString(label, x, y, 1);

  int bx = x + 28;
  tft.fillRect(bx, y, w, h, C_HP_BG);
  tft.fillRect(bx, y, filled, h, color);
  tft.drawRect(bx, y, w, h, C_BORDER);

  char numBuf[8];
  snprintf(numBuf, sizeof(numBuf), "%d", val);
  tft.setTextColor(C_TEXT);
  tft.drawString(numBuf, bx + w + 3, y, 1);
}

void draw_typeBadge(int x, int y, const char *type) {
  if(strlen(type) == 0) return;
  int w = strlen(type) * 6 + 6;
  tft.fillRect(x, y, w, 11, type_color(type));
  tft.setTextColor(C_TEXT);
  tft.drawString(type, x + 3, y + 2, 1);
}

void display_pokemon(const char *folder, Pokemon &p) {
  tft.fillScreen(C_BG);

  tft.fillRect(0, 0, 320, 2, C_GOLD);

  char imgPath[80];
  snprintf(imgPath, sizeof(imgPath), "/pokemons/%s/front.jpg", folder);

  tft.fillRect(SPRITE_X - 2, SPRITE_Y - 2, SPRITE_W + 4, SPRITE_H + 4, C_PANEL);
  tft.drawRect(SPRITE_X - 2, SPRITE_Y - 2, SPRITE_W + 4, SPRITE_H + 4, C_BORDER);
  draw_jpegFromSD(imgPath, SPRITE_X, SPRITE_Y);

  char nameBuf[34];
  snprintf(nameBuf, sizeof(nameBuf), "%s", p.name);
  nameBuf[0] = toupper((unsigned char)nameBuf[0]);
  tft.setTextColor(C_GOLD);
  tft.drawString(nameBuf, INFO_X, INFO_Y, 2);

  draw_typeBadge(INFO_X, INFO_Y + 20, p.type1);
  draw_typeBadge(INFO_X + 58, INFO_Y + 20, p.type2);

  draw_statBar(STATS_X, STATS_Y + 0, 110, 7, p.hp, 250, C_HP_GREEN, "HP");
  draw_statBar(STATS_X, STATS_Y + 12, 110, 7, p.atk, 180, 0xFB00, "ATK");
  draw_statBar(STATS_X, STATS_Y + 24, 110, 7, p.def, 180, 0x045F, "DEF");
  draw_statBar(STATS_X, STATS_Y + 36, 110, 7, p.sdef, 180, 0x8010, "SDF");
  draw_statBar(STATS_X, STATS_Y + 48, 110, 7, p.spd, 180, C_HP_YELLOW, "SPD");

  tft.drawLine(0, MOVES_Y - 4, 320, MOVES_Y - 4, C_BORDER);

  int mW = 154, mH = 19, mPad = 3;
  int mBaseY = MOVES_Y;

  for(int i = 0; i < 4; i++) {
    int col = i % 2;
    int row = i / 2;
    int mx = col * 161 + mPad;
    int my = mBaseY + row * (mH + mPad);

    tft.fillRect(mx, my, mW, mH, move_color(p.moves[i].category));
    tft.drawRect(mx, my, mW, mH, C_BORDER);

    tft.setTextColor(C_TEXT);
    tft.drawString(p.moves[i].name, mx + 3, my + 3, 1);

    char pwrBuf[12];
    snprintf(pwrBuf, sizeof(pwrBuf), "%d", p.moves[i].power);
    int pwrX = mx + mW - strlen(pwrBuf) * 6 - 3;
    tft.setTextColor(C_GOLD);
    tft.drawString(pwrBuf, pwrX, my + 3, 1);
  }

  tft.fillRect(0, 238, 320, 2, C_GOLD);
}

/* Boot sequence functions ------------------------------------------------- */
void show_bootScreen() {
  unsigned long elapsed = millis() - bootStateTime;

  switch(bootState) {
    case BOOT_LOGIN: {
      /* Show login screen */
      if(elapsed == 0) {
        tft.fillScreen(TFT_BLACK);
        draw_jpegFromSD("/components/login.jpg", 0, 0);
        Serial.println("Boot: Exibindo tela de login...");
      }
      break;
    }

    case BOOT_BACKGROUND: {
      /* Show background */
      if(elapsed == 0) {
        tft.fillScreen(TFT_BLACK);
        draw_jpegFromSD("/components/background.jpg", 0, 0);
        Serial.println("Boot: Exibindo background...");
      }
      break;
    }

    case BOOT_TRAINER: {
      /* Show trainer selection */
      if(elapsed == 0) {
        tft.fillScreen(TFT_BLACK);
        draw_jpegFromSD("/components/trainer_1.jpg", 0, 0);
        Serial.println("Boot: Exibindo treinador...");
      }
      break;
    }

    case BOOT_LOADING: {
      /* Show loading screen with status */
      if(elapsed == 0) {
        tft.fillScreen(C_BG);
        draw_jpegFromSD("/components/pick.jpg", 0, 0);
        Serial.println("Boot: Carregando pokémons...");
      }
      
      /* Show loading progress */
      tft.setTextColor(C_GOLD);
      tft.drawString("Carregando", 120, 100, 2);
      
      int dots = (elapsed / 300) % 4;
      char loadBuf[20];
      snprintf(loadBuf, sizeof(loadBuf), "%-3s", "...");
      if(dots > 0) tft.drawString(loadBuf, 180, 100, 2);
      
      break;
    }

    case BOOT_COMPLETE: {
      /* Boot complete, no need to draw */
      break;
    }
  }
}

void update_bootSequence() {
  unsigned long elapsed = millis() - bootStateTime;
  unsigned long requiredTime = 0;

  switch(bootState) {
    case BOOT_LOGIN:
      requiredTime = BOOT_LOGIN_TIME;
      if(elapsed < requiredTime) {
        show_bootScreen();
      } else {
        bootState = BOOT_BACKGROUND;
        bootStateTime = millis();
      }
      break;

    case BOOT_BACKGROUND:
      requiredTime = BOOT_BG_TIME;
      if(elapsed < requiredTime) {
        show_bootScreen();
      } else {
        bootState = BOOT_TRAINER;
        bootStateTime = millis();
      }
      break;

    case BOOT_TRAINER:
      requiredTime = BOOT_TRAINER_TIME;
      if(elapsed < requiredTime) {
        show_bootScreen();
      } else {
        bootState = BOOT_LOADING;
        bootStateTime = millis();
      }
      break;

    case BOOT_LOADING:
      requiredTime = BOOT_LOAD_TIME;
      if(elapsed < requiredTime) {
        show_bootScreen();
      } else {
        bootState = BOOT_COMPLETE;
        bootComplete = true;
        currentIdx = 0;
        lastSwitch = millis();
        Serial.println("Boot: Sequencia completa! Iniciando jogo...");
      }
      break;

    case BOOT_COMPLETE:
      bootComplete = true;
      break;
  }
}
