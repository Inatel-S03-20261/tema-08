/**
 * @file sd_card.cpp
 * @brief SD Card source file.
 *
 * @authors Gustavo Pivoto Ambrósio
 * @date 2026-06-13
 */

/* Includes ----------------------------------------------------------------- */
#include "sd_card.h"

#include <SPI.h>
#include <FS.h>
#include <SD.h>
#include <TFT_eSPI.h>
#include <JPEGDecoder.h>

/* Private defines ---------------------------------------------------------- */

/* Private macros ----------------------------------------------------------- */

/* Private typedefs --------------------------------------------------------- */

/* Private constants -------------------------------------------------------- */

/* Private variables -------------------------------------------------------- */
char pokemonFolders[MAX_POKEMONS][32];
int pokemonCount = 0;

/* Private function prototypes ---------------------------------------------- */
static bool parse_line(const String &line, const char *key, char *out, int outLen);

/* Public functions --------------------------------------------------------- */
bool SD_init() {
    return SD.begin(SD_CS);
}

void scan_pokemonsOnSD() {
    pokemonCount = 0;
    
    File root = SD.open("/pokemons");
    if(!root || !root.isDirectory()) {
        Serial.println("/pokemons não encontrado!");
        return;
    }

    File entry = root.openNextFile();
    while(entry && pokemonCount < MAX_POKEMONS) {
        if(entry.isDirectory()) {
            String name = String(entry.name());
            if(name.startsWith("/pokemons/")) name = name.substring(10);
            if(name.startsWith("pokemons/"))  name = name.substring(9);
            name.toCharArray(pokemonFolders[pokemonCount], 32);
            Serial.printf("  Encontrado: %s\n", pokemonFolders[pokemonCount]);
            pokemonCount++;
        }
        
        entry = root.openNextFile();
    }
    
    root.close();
    Serial.printf("Total: %d pokémons\n", pokemonCount);
}

bool load_pokemon(const char *folder, Pokemon &p) {
    char path[80];
  
    snprintf(path, sizeof(path), "/pokemons/%s/data.txt", folder);

    File f = SD.open(path);
    if(!f) return false;

    memset(&p, 0, sizeof(p));

    while(f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();

        if(parse_line(line, "NAME:",  p.name,  sizeof(p.name))) {}
        else if(parse_line(line, "TYPE1:", p.type1, sizeof(p.type1))) {}
        else if (parse_line(line, "TYPE2:", p.type2, sizeof(p.type2))) {}
        else if (line.startsWith("HP:")) p.hp = line.substring(3).toInt();
        else if (line.startsWith("ATK:")) p.atk = line.substring(4).toInt();
        else if (line.startsWith("DEF:")) p.def = line.substring(4).toInt();
        else if (line.startsWith("SDEF:")) p.sdef = line.substring(5).toInt();
        else if (line.startsWith("SPD:")) p.spd = line.substring(4).toInt();
        else {
            for(int i = 0; i < 4; i++) {
                char key[8];
                snprintf(key, sizeof(key), "MOVE%d:", i + 1);
                if(line.startsWith(key)) {
                    String val = line.substring(strlen(key));
                    int sep1 = val.indexOf('|');
                    int sep2 = val.lastIndexOf('|');
                    val.substring(0, sep1).toCharArray(p.moves[i].name, sizeof(p.moves[i].name));
                    p.moves[i].power = val.substring(sep1 + 1, sep2).toInt();
                    val.substring(sep2 + 1).toCharArray(p.moves[i].category, sizeof(p.moves[i].category));
                }
            }
        }
    }

    f.close();
    return true;
}

void draw_jpegFromSD(const char *path, int x, int y) {
    File f = SD.open(path);
    if(!f) {
        Serial.printf("Erro ao abrir: %s\n", path);
        return;
    }

    size_t size = f.size();
    uint8_t* buf = (uint8_t*)malloc(size);
    if(!buf) {
        Serial.println("Sem memória para JPEG!");
        f.close();
        return;
    }

    f.read(buf, size);
    f.close();

    if(JpegDec.decodeArray(buf, size)) {
        uint16_t* pImg;
        int mcu_w = JpegDec.MCUWidth;
        int mcu_h = JpegDec.MCUHeight;

        while(JpegDec.read()) {
            pImg = JpegDec.pImage;
            int mcu_x = JpegDec.MCUx * mcu_w + x;
            int mcu_y = JpegDec.MCUy * mcu_h + y;
            int pixelCount = mcu_w * mcu_h;

            for(int i = 0; i < pixelCount; i++) {
                uint16_t px = pImg[i];
                pImg[i] = ((px & 0xFF) << 8) | ((px >> 8) & 0xFF);
            }

            if((mcu_x + mcu_w) <= tft.width() && (mcu_y + mcu_h) <= tft.height()) {
                tft.pushImage(mcu_x, mcu_y, mcu_w, mcu_h, pImg);
            }
        }
    }

  free(buf);
}

/* Private functions -------------------------------------------------------- */
static bool parse_line(const String &line, const char *key, char *out, int outLen) {
    if(line.startsWith(key)) {
        line.substring(strlen(key)).toCharArray(out, outLen);
        return true;
    }
    
    return false;
}
