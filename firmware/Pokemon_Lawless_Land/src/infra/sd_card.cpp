#include "infra/sd_card.h"

#include <FS.h>
#include <JPEGDecoder.h>
#include <PNGdec.h>
#include <SD.h>
#include <SPI.h>

char pokemonFolders[MAX_POKEMONS][32];
int pokemonCount = 0;

static bool parse_line(const String& line, const char* key, char* out,
                       int outLen);
static bool draw_pngFromSD(const char* path, int x, int y);

static PNG pngDecoder;
static File pngFile;
static int pngDrawX = 0;
static int pngDrawY = 0;

static void* pngOpen(const char* filename, int32_t* size) {
  pngFile = SD.open(filename);
  if (!pngFile) return nullptr;
  *size = pngFile.size();
  return &pngFile;
}

static void pngClose(void* handle) {
  (void)handle;
  if (pngFile) pngFile.close();
}

static int32_t pngRead(PNGFILE* handle, uint8_t* buffer, int32_t length) {
  (void)handle;
  return pngFile ? pngFile.read(buffer, length) : 0;
}

static int32_t pngSeek(PNGFILE* handle, int32_t position) {
  (void)handle;
  return pngFile ? pngFile.seek(position) : 0;
}

static int pngDraw(PNGDRAW* draw) {
  static uint16_t pixels[320];
  static uint8_t alphaMask[40];
  int width = min(draw->iWidth, 320);
  int screenY = pngDrawY + draw->y;
  if (width <= 0 || screenY < 0 || screenY >= tft.height()) return 1;

  pngDecoder.getLineAsRGB565(draw, pixels, PNG_RGB565_BIG_ENDIAN, 0xFFFFFFFF);
  pngDecoder.getAlphaMask(draw, alphaMask, 128);

  int runStart = -1;
  for (int i = 0; i <= width; i++) {
    bool opaque =
        i < width && (alphaMask[i >> 3] & (0x80 >> (i & 7))) != 0;

    if (opaque && runStart < 0) {
      runStart = i;
    } else if (!opaque && runStart >= 0) {
      int runWidth = i - runStart;
      int screenX = pngDrawX + runStart;
      if (screenX < tft.width() && screenX + runWidth > 0) {
        int sourceOffset = 0;
        if (screenX < 0) {
          sourceOffset = -screenX;
          runWidth -= sourceOffset;
          screenX = 0;
        }
        runWidth = min(runWidth, tft.width() - screenX);
        if (runWidth > 0) {
          tft.pushImage(screenX, screenY, runWidth, 1,
                        pixels + runStart + sourceOffset);
        }
      }
      runStart = -1;
    }
  }
  return 1;
}

bool SD_init() {
  if (!SD.begin(SD_CS)) {
    Serial.println("Falha ao montar o cartao SD");
    return false;
  }

  Serial.printf("SD montado. Capacidade: %llu MB\n",
                SD.cardSize() / (1024ULL * 1024ULL));
  if (!SD.exists("/components")) {
    Serial.println("Aviso: pasta /components nao existe no SD");
  }
  if (!SD.exists("/pokemons")) {
    Serial.println("Aviso: pasta /pokemons nao existe no SD");
  }
  return true;
}

void scan_pokemonsOnSD() {
  pokemonCount = 0;
  File root = SD.open("/pokemons");
  if (!root || !root.isDirectory()) {
    Serial.println("Pasta /pokemons nao encontrada");
    return;
  }

  File entry = root.openNextFile();
  while (entry && pokemonCount < MAX_POKEMONS) {
    if (entry.isDirectory()) {
      String name = String(entry.name());
      if (name.startsWith("/pokemons/")) name = name.substring(10);
      if (name.startsWith("pokemons/")) name = name.substring(9);
      name.toCharArray(pokemonFolders[pokemonCount],
                       sizeof(pokemonFolders[pokemonCount]));
      Serial.printf("Pokemon encontrado: %s\n", pokemonFolders[pokemonCount]);
      pokemonCount++;
    }
    entry.close();
    entry = root.openNextFile();
  }

  root.close();
  Serial.printf("Total de pokemons: %d\n", pokemonCount);
}

bool load_pokemon(const char* folder, PokemonData& pokemon) {
  char path[80];
  snprintf(path, sizeof(path), "/pokemons/%s/data.txt", folder);

  File file = SD.open(path);
  if (!file) {
    Serial.printf("Dados do pokemon nao encontrados: %s\n", path);
    return false;
  }

  memset(&pokemon, 0, sizeof(pokemon));
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();

    if (parse_line(line, "NAME:", pokemon.name, sizeof(pokemon.name))) {
    } else if (parse_line(line, "TYPE1:", pokemon.type1,
                          sizeof(pokemon.type1))) {
    } else if (parse_line(line, "TYPE2:", pokemon.type2,
                          sizeof(pokemon.type2))) {
    } else if (line.startsWith("HP:")) {
      pokemon.hp = line.substring(3).toInt();
    } else if (line.startsWith("ATK:")) {
      pokemon.atk = line.substring(4).toInt();
    } else if (line.startsWith("DEF:")) {
      pokemon.def = line.substring(4).toInt();
    } else if (line.startsWith("SDEF:")) {
      pokemon.sdef = line.substring(5).toInt();
    } else if (line.startsWith("SPD:")) {
      pokemon.spd = line.substring(4).toInt();
    } else {
      for (int i = 0; i < 4; i++) {
        char key[8];
        snprintf(key, sizeof(key), "MOVE%d:", i + 1);
        if (!line.startsWith(key)) continue;

        String value = line.substring(strlen(key));
        int firstSeparator = value.indexOf('|');
        int lastSeparator = value.lastIndexOf('|');
        if (firstSeparator < 0 || lastSeparator <= firstSeparator) continue;

        value.substring(0, firstSeparator)
            .toCharArray(pokemon.moves[i].name, sizeof(pokemon.moves[i].name));
        pokemon.moves[i].power =
            value.substring(firstSeparator + 1, lastSeparator).toInt();
        value.substring(lastSeparator + 1)
            .toCharArray(pokemon.moves[i].category,
                         sizeof(pokemon.moves[i].category));
      }
    }
  }

  file.close();
  return true;
}

static bool drawJpegClipped(const char* path, int x, int y, int clipX,
                            int clipY, int clipWidth, int clipHeight) {
  File file = SD.open(path);
  if (!file) {
    Serial.printf("Imagem nao encontrada: %s\n", path);
    return false;
  }

  size_t size = file.size();
  if (size == 0) {
    Serial.printf("Imagem vazia: %s\n", path);
    file.close();
    return false;
  }

  uint8_t* buffer = static_cast<uint8_t*>(malloc(size));
  if (buffer == nullptr) {
    Serial.printf("Sem memoria para imagem (%u bytes): %s\n",
                  static_cast<unsigned int>(size), path);
    file.close();
    return false;
  }

  size_t bytesRead = file.read(buffer, size);
  file.close();

  bool drawn = false;
  if (bytesRead == size && JpegDec.decodeArray(buffer, size)) {
    int mcuWidth = JpegDec.MCUWidth;
    int mcuHeight = JpegDec.MCUHeight;

    while (JpegDec.read()) {
      uint16_t* image = JpegDec.pImage;
      int imageX = JpegDec.MCUx * mcuWidth + x;
      int imageY = JpegDec.MCUy * mcuHeight + y;
      int pixelCount = mcuWidth * mcuHeight;

      for (int i = 0; i < pixelCount; i++) {
        uint16_t pixel = image[i];
        image[i] = ((pixel & 0x00FF) << 8) | ((pixel & 0xFF00) >> 8);
      }

      int left = max(imageX, clipX);
      int top = max(imageY, clipY);
      int right = min(min(imageX + mcuWidth, clipX + clipWidth),
                      static_cast<int>(tft.width()));
      int bottom = min(min(imageY + mcuHeight, clipY + clipHeight),
                       static_cast<int>(tft.height()));

      if (right > left && bottom > top) {
        int sourceX = left - imageX;
        int sourceY = top - imageY;
        int drawWidth = right - left;
        int drawHeight = bottom - top;

        for (int row = 0; row < drawHeight; row++) {
          tft.pushImage(left, top + row, drawWidth, 1,
                        image + (sourceY + row) * mcuWidth + sourceX);
        }
        drawn = true;
      }
    }
  } else {
    Serial.printf("JPEG invalido ou leitura incompleta: %s\n", path);
  }

  free(buffer);
  if (drawn) Serial.printf("Imagem carregada: %s\n", path);
  return drawn;
}

bool draw_jpegFromSD(const char* path, int x, int y) {
  return drawJpegClipped(path, x, y, 0, 0, tft.width(), tft.height());
}

bool restore_backgroundRegion(int x, int y, int width, int height) {
  return restore_jpegRegion("/components/background.jpg", x, y, width, height);
}

bool restore_jpegRegion(const char* path, int x, int y, int width, int height) {
  if (width <= 0 || height <= 0) return false;
  return drawJpegClipped(path, 0, 0, x, y, width, height);
}

bool draw_pokemonImage(const char* pastaAssets, bool usarCostas, int x, int y) {
  if (pastaAssets == nullptr || strlen(pastaAssets) == 0) {
    Serial.println("Pokemon sem pasta de imagens");
    return false;
  }

  char path[96];
  snprintf(path, sizeof(path), "/pokemons/%s/%s.png", pastaAssets,
           usarCostas ? "back" : "front");
  if (draw_pngFromSD(path, x, y)) return true;

  if (usarCostas) {
    snprintf(path, sizeof(path), "/pokemons/%s/front.png", pastaAssets);
    if (draw_pngFromSD(path, x, y)) return true;
  }

  snprintf(path, sizeof(path), "/pokemons/%s/%s.jpg", pastaAssets,
           usarCostas ? "back" : "front");
  if (draw_jpegFromSD(path, x, y)) return true;

  if (usarCostas) {
    snprintf(path, sizeof(path), "/pokemons/%s/front.jpg", pastaAssets);
    return draw_jpegFromSD(path, x, y);
  }
  return false;
}

static bool draw_pngFromSD(const char* path, int x, int y) {
  if (!SD.exists(path)) {
    Serial.printf("Imagem PNG nao encontrada: %s\n", path);
    return false;
  }

  pngDrawX = x;
  pngDrawY = y;
  int result =
      pngDecoder.open(path, pngOpen, pngClose, pngRead, pngSeek, pngDraw);
  if (result != PNG_SUCCESS) {
    Serial.printf("Falha ao abrir PNG (%d): %s\n", result, path);
    return false;
  }

  result = pngDecoder.decode(nullptr, 0);
  pngDecoder.close();
  if (result != PNG_SUCCESS) {
    Serial.printf("Falha ao decodificar PNG (%d): %s\n", result, path);
    return false;
  }

  Serial.printf("Imagem PNG carregada: %s\n", path);
  return true;
}

static bool parse_line(const String& line, const char* key, char* out,
                       int outLen) {
  if (!line.startsWith(key)) return false;
  line.substring(strlen(key)).toCharArray(out, outLen);
  return true;
}
