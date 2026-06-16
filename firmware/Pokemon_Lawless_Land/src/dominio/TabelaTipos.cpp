#include "dominio/TabelaTipos.h"

#include "contratos/IPokemon.h"

namespace TabelaTipos {

namespace {

bool igualaSemCase(const char* a, const char* b) {
  if (a == nullptr || b == nullptr) return false;
  while (*a && *b) {
    char ca = *a, cb = *b;
    if (ca >= 'A' && ca <= 'Z') ca = static_cast<char>(ca - 'A' + 'a');
    if (cb >= 'A' && cb <= 'Z') cb = static_cast<char>(cb - 'A' + 'a');
    if (ca != cb) return false;
    ++a;
    ++b;
  }
  return *a == *b;  // ambos no terminador
}

}  // namespace

Tipo tipoDeTexto(const char* slug) {
  struct Par {
    const char* nome;
    Tipo tipo;
  };
  static const Par kMapa[] = {
      {"normal", Tipo::NORMAL},     {"fire", Tipo::FIRE},
      {"water", Tipo::WATER},       {"electric", Tipo::ELECTRIC},
      {"grass", Tipo::GRASS},       {"ice", Tipo::ICE},
      {"fighting", Tipo::FIGHTING}, {"poison", Tipo::POISON},
      {"ground", Tipo::GROUND},     {"flying", Tipo::FLYING},
      {"psychic", Tipo::PSYCHIC},   {"bug", Tipo::BUG},
      {"rock", Tipo::ROCK},         {"ghost", Tipo::GHOST},
      {"dragon", Tipo::DRAGON},     {"dark", Tipo::DARK},
      {"steel", Tipo::STEEL},       {"fairy", Tipo::FAIRY},
  };
  for (const Par& p : kMapa) {
    if (igualaSemCase(slug, p.nome)) return p.tipo;
  }
  return Tipo::DESCONHECIDO;
}

bool superEfetivoContra(Tipo a, Tipo d) {
  using T = Tipo;
  switch (a) {
    case T::NORMAL:
      return false;
    case T::FIRE:
      return d == T::GRASS || d == T::ICE || d == T::BUG || d == T::STEEL;
    case T::WATER:
      return d == T::FIRE || d == T::GROUND || d == T::ROCK;
    case T::ELECTRIC:
      return d == T::WATER || d == T::FLYING;
    case T::GRASS:
      return d == T::WATER || d == T::GROUND || d == T::ROCK;
    case T::ICE:
      return d == T::GRASS || d == T::GROUND || d == T::FLYING ||
             d == T::DRAGON;
    case T::FIGHTING:
      return d == T::NORMAL || d == T::ICE || d == T::ROCK || d == T::DARK ||
             d == T::STEEL;
    case T::POISON:
      return d == T::GRASS || d == T::FAIRY;
    case T::GROUND:
      return d == T::FIRE || d == T::ELECTRIC || d == T::POISON ||
             d == T::ROCK || d == T::STEEL;
    case T::FLYING:
      return d == T::GRASS || d == T::FIGHTING || d == T::BUG;
    case T::PSYCHIC:
      return d == T::FIGHTING || d == T::POISON;
    case T::BUG:
      return d == T::GRASS || d == T::PSYCHIC || d == T::DARK;
    case T::ROCK:
      return d == T::FIRE || d == T::ICE || d == T::FLYING || d == T::BUG;
    case T::GHOST:
      return d == T::PSYCHIC || d == T::GHOST;
    case T::DRAGON:
      return d == T::DRAGON;
    case T::DARK:
      return d == T::PSYCHIC || d == T::GHOST;
    case T::STEEL:
      return d == T::ICE || d == T::ROCK || d == T::FAIRY;
    case T::FAIRY:
      return d == T::FIGHTING || d == T::DRAGON || d == T::DARK;
    default:
      return false;
  }
}

bool superEfetivoContra(const char* tipoAtaque, const IPokemon& defensor) {
  Tipo ataque = tipoDeTexto(tipoAtaque);
  if (ataque == Tipo::DESCONHECIDO) return false;
  for (int i = 0; i < defensor.getQuantidadeTipos(); ++i) {
    if (superEfetivoContra(ataque, tipoDeTexto(defensor.getTipo(i)))) {
      return true;
    }
  }
  return false;
}

}  // namespace TabelaTipos
