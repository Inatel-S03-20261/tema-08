#pragma once

class IPokemon;

namespace TabelaTipos {

enum class Tipo {
  NORMAL, FIRE, WATER, ELECTRIC, GRASS, ICE, FIGHTING, POISON, GROUND,
  FLYING, PSYCHIC, BUG, ROCK, GHOST, DRAGON, DARK, STEEL, FAIRY,
  DESCONHECIDO
};

Tipo tipoDeTexto(const char* slug);

bool superEfetivoContra(Tipo ataque, Tipo defensor);

bool superEfetivoContra(const char* tipoAtaque, const IPokemon& defensor);

}  // namespace TabelaTipos
