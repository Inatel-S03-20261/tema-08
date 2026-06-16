#pragma once

class IPokemon;

struct Rodada {
  int numero = 0;
  IPokemon* pokemonJogador1 = nullptr;
  IPokemon* pokemonJogador2 = nullptr;
  IPokemon* pokemonVencedor = nullptr;
};
