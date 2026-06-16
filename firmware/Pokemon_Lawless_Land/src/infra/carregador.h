#ifndef INFRA_CARREGADOR_H_
#define INFRA_CARREGADOR_H_

class Pokemon;
class Jogador;
class Aleatorio;
struct PokemonData;

void montarPokemon(const PokemonData& dados, const char* folder,
                   Pokemon& destino, Aleatorio& rng);

bool carregarPokemonDoSD(const char* folder, Pokemon& destino, Aleatorio& rng);

int carregarPoolDoSD(Pokemon* pool, int maxPool, Aleatorio& rng);

void distribuirAleatorio(Jogador& j1, Jogador& j2, Pokemon* pool, int n,
                         int qtdCada, Aleatorio& rng);

#endif  // INFRA_CARREGADOR_H_
