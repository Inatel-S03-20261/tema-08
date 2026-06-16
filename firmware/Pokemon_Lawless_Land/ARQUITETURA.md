# Arquitetura — Pokémon Lawless Land (ESP32)

Documento de referência da estrutura do código e dos padrões aplicados. Toda a
explicação que antes vivia em comentários de bloco foi centralizada aqui, para
ter uma única fonte de verdade (comentários inline ficam só como marcadores
curtos de detalhes locais).

---

## 1. Visão geral

Jogo de batalha Pokémon embarcado num **ESP32** com display **TFT ILI9341
320×240** e cartão **microSD** (sprites, golpes e atributos). A entrada é por
terminal/serial (`a`/`d`/`s`/`w`); o resultado da batalha é publicado por
**MQTT** num broker HiveMQ.

O código é dividido em **4 camadas** com dependências de fora para dentro:

```
        contratos/   (interfaces puras: IPokemon, IJogador)
            ▲
        dominio/     (engine de batalha — NÃO conhece hardware)
            ▲
   ┌────────┴─────────┐
 infra/              app/        main.cpp
(adapta hardware)  (SPA + estados)  (setup/loop)
```

Princípio central: **a engine (`dominio/`) é pura** — depende só de `contratos/`,
nunca de Arduino/TFT/SD. Por isso ela roda e é testada no PC (ambiente `native`
do PlatformIO, 9 testes Unity) sem nenhum hardware.

---

## 2. Estrutura de pastas e papel de cada arquivo

### `contratos/` — interfaces (desacoplam o domínio do mundo concreto)
| Arquivo | Para que serve |
|---|---|
| `IPokemon.h` | Contrato de um Pokémon: vida, energia, tipos, ataques. |
| `IJogador.h` | Contrato de um jogador: deck de cartas e Pokémon ativo. |
| `IPublicadorResultado.h` | Contrato de saída do resultado (Target do Adapter MQTT). |
| `IAutenticador.h` | Contrato de login (Target do Adapter HTTP do Tema-01). |

### `dominio/` — engine pura (testável sem hardware)
| Arquivo | Para que serve |
|---|---|
| `Ataque.h/.cpp` | Classe base **abstrata** dos golpes; `executar()` é um **Template Method**. |
| `AtaqueNormal.*` | Golpe comum (dano base mitigado pela defesa). |
| `AtaqueCritico.*` | Golpe que pode dar crítico (chance via `Aleatorio`). |
| `Aleatorio.h/.cpp` | PRNG `xorshift32` puro (semeável); usado pela batalha. |
| `AtaqueSuperEfetivo.*` | Golpe com bônus por vantagem de tipo. |
| `Batalha.h/.cpp` | **Motor da batalha**: turnos, energia, fim de rodada/partida, vencedor e histórico de rodadas. |
| `Pokemon.h/.cpp` | Implementação concreta de `IPokemon`. |
| `Jogador.h/.cpp` | Implementação concreta de `IJogador`. |
| `Rodada.h` | Struct de uma rodada do histórico (pokémon de cada lado + vencedor). |
| `TabelaTipos.h/.cpp` | Tabela de efetividade de tipos (quem é super efetivo contra quem). |

### `infra/` — adaptadores de hardware/bibliotecas
| Arquivo | Para que serve |
|---|---|
| `sd_card.h/.cpp` | Monta o SD e desenha JPEG/PNG no TFT (com recorte de região). |
| `carregador.h/.cpp` | Constrói `Pokemon` a partir dos dados do SD e distribui as cartas. |
| `dados_demo.h/.cpp` | Pokémon de exemplo na flash (fallback quando o SD não tem dados). |
| `buttons.h/.cpp` | Entrada do usuário (lê `a`/`d`/`s`/`w` da serial). |
| `PublicadorMqtt.h/.cpp` | **Adapter** de `IPublicadorResultado`: envolve `PubSubClient`/WiFi e publica em `LawlessLandResults`. |
| `AutenticadorHttp.h/.cpp` | **Adapter** de `IAutenticador`: `POST /auth/login` no auth-service (Tema-01) via `HTTPClient`. Scaffold opcional; o mock é o padrão. |

### `app/` — a aplicação SPA (controla a única tela)
| Arquivo | Para que serve |
|---|---|
| `IdEstado.h` | `enum` de identificadores de estado (chave do registro). |
| `Estado.h` | Interface **State** (participante *State* do GoF). |
| `ControladorAplicacao.h/.cpp` | **Context + Singleton**: registra os estados, faz a transição e delega o comportamento. |
| `estados/EstadoLogin.*` | Login dos dois jogadores (nick + senha, mock). |
| `estados/EstadoSelecao.*` | Cada jogador escolhe seu Pokémon ativo. |
| `estados/EstadoBatalha.*` | A batalha em si (sprites, barras, menu de golpes). |
| `estados/EstadoResultado.*` | Mostra o vencedor + histórico e publica via MQTT. |

### Raiz
| Arquivo | Para que serve |
|---|---|
| `main.cpp` | `setup()`/`loop()` do Arduino: monta o pool, registra os estados e roda o laço `tratarEvento → atualizar → renderizar`. |

---

## 3. Padrões aplicados (e onde)

### 3.1 SPA — *Single Page Application* (arquitetura escolhida)
Entre **MOM**, **MVC** e **SPA**, escolhemos **SPA**: o dispositivo tem **uma
única "página"** (a tela TFT). Não há navegação entre páginas separadas — o
`ControladorAplicacao` mantém **um componente ativo** e o **troca em tempo de
execução**, redesenhando a mesma tela. Cada "componente" é uma `Estado`
(Login → Seleção → Batalha → Resultado). *(É por isso que esses estados se
chamavam "Componente" originalmente.)*

- **Onde:** `ControladorAplicacao` (raiz da SPA) + o laço de `main.cpp`.
- **Como reforçamos a ideia:** *atualização parcial* — cada estado redesenha só
  a parte que mudou (status, sprite, log), análogo a atualizar nós de um DOM em
  vez de recarregar a página.

```
loop():  controlador.tratarEvento();  controlador.atualizar();  controlador.renderizar();
         └────────── tudo delegado ao componente/estado ATIVO ──────────┘
```

### 3.2 State (GoF)
É o mecanismo que implementa a SPA: cada tela é um estado com comportamento
próprio e decide a própria transição.

| Papel no GoF | No projeto |
|---|---|
| **Context** | `ControladorAplicacao` (guarda `atual_`, faz `trocarEstado`, delega) |
| **State** (interface) | `Estado` (`aoAbrir`/`aoFechar`/`tratarEvento`/`atualizar`/`renderizar`) |
| **ConcreteState** | `EstadoLogin`, `EstadoSelecao`, `EstadoBatalha`, `EstadoResultado` |
| Chave de registro | `enum IdEstado` |

Transição (um estado dispara a próxima):
`controlador_->trocarEstado(IdEstado::BATALHA);`

### 3.3 Singleton (GoF)
- **Onde:** `ControladorAplicacao::instancia()` — instância única, construtor
  privado. Garante um único orquestrador acessível de qualquer estado, sem
  passar o ponteiro manualmente por toda parte.

### 3.4 Adapter (GoF)
O projeto usa Adapter sempre que precisa ligar uma **biblioteca/serviço externo**
a uma **interface do nosso domínio** (`contratos/`). Dois exemplos concretos:

**a) Envio do resultado pelo broker** — o Adapter completo:

| Papel no GoF | No projeto |
|---|---|
| **Target** (o que o app espera) | `IPublicadorResultado` (`publicar(payload)`) |
| **Adaptee** (lib externa) | `PubSubClient` (cliente MQTT) + WiFi |
| **Adapter** | `PublicadorMqtt` (tem um `PubSubClient` e o traduz) |
| **Client** | `EstadoResultado` (depende só de `IPublicadorResultado`) |

O `EstadoResultado` chama `publicador_->publicar(...)` **sem saber que é MQTT** —
trocar por HTTP, por exemplo, seria só outro Adapter implementando a mesma
interface. A injeção acontece no `main.cpp` (`configurarPublicador`).

**b) Login (Tema-01)** — `AutenticadorHttp` implementa `IAutenticador` chamando
`POST /auth/login` do auth-service (Spring Boot) via `HTTPClient`. É um *scaffold
opcional*: o `EstadoLogin` só usa o login real se `configurarAutenticador()` for
chamado; por padrão segue o mock (não depende de rede no demo).

> **Pendente — distribuição de cartas (Tema-02):** o grupo ainda **não expõe um
> endpoint HTTP** (o código é só classes Python + demo) e os dados por carta são
> apenas `{id, nome, tipos}` — sem hp/defesa/golpes que a batalha precisa. Falta
> alinhar o contrato; quando definido, o Adapter `IDistribuidorCartas` segue o
> mesmo molde.

### 3.5 Template Method (GoF) — presente na hierarquia de Ataque
- **Onde:** `Ataque::executar()` é o **template concreto** que fixa o esqueleto
  do ataque — *calcular o dano* (passo polimórfico) → *aplicar no defensor* →
  *gastar energia do atacante*. As subclasses só variam o passo `calcular()`:
  - `AtaqueNormal` → dano direto mitigado pela defesa;
  - `AtaqueCritico` → chance de multiplicador crítico;
  - `AtaqueSuperEfetivo` → bônus por vantagem de tipo.

```
Ataque::executar()  [esqueleto fixo]
   └── calcular()   [primitivo — cada subclasse implementa o seu]
```

---

## 4. Decisões de projeto (e o porquê)

Pontos para defender na apresentação:

1. **Programar contra interfaces (Inversão de Dependência).** A engine
   (`dominio/`) depende só de `contratos/` (`IPokemon`, `IJogador`),
   nunca das implementações concretas. Ganhos: (a) cada integrante trabalha em
   paralelo; (b) a batalha é **testável no PC** com objetos falsos; (c) trocar
   hardware (display, RNG) não toca na lógica.

2. **State em vez de `switch(estado)`.** Cada tela encapsula seu próprio
   comportamento e decide a própria transição (`controlador_->trocarEstado(...)`).
   Não existe um `switch` gigante de telas — justamente o anti-exemplo que o
   State resolve.

3. **Singleton em `ControladorAplicacao`.** Há um único orquestrador da
   aplicação: `instancia()` no estilo Meyers (objeto `static` local), construtor
   privado, cópia/movimento `= delete`.

4. **`dominio/` e `app/` são "puros" (sem Arduino).** Só `infra/` e `main.cpp`
   incluem `Arduino.h`. É isso que permite **compilar e testar a lógica no PC**
   (ambiente `native`), separando regra de negócio de hardware.

5. **Sem heap na lógica de batalha** (boa prática em microcontrolador): o
   histórico de rodadas é **array fixo**; nomes/tipos são `const char*` (sem
   alocar `String`); a `Batalha` não faz `new`/`delete` — só **usa** objetos cuja
   posse é externa (ver seção 5).

6. **Aleatoriedade concreta e semeável (`Aleatorio`).** PRNG `xorshift32` em C++
   puro (sem Arduino): a produção semeia com `esp_random()`; os testes usam uma
   **semente fixa** → o crítico e o sorteio de quem começa ficam **determinísticos,
   testáveis sem flakiness**. (Não é mais uma interface — só uma classe do domínio.)

---

## 5. Ponteiros, referências e posse (ownership)

A parte que mais gera dúvida — o raciocínio em uma olhada:

- **Referência (`T&`)** = apelido de um objeto que **sempre existe e não troca**.
  Usada em parâmetros/membros obrigatórios: `Batalha(IJogador&, IJogador&,
  Aleatorio&)`, `executar(IPokemon& atacante, IPokemon& defensor)`. Lê melhor e
  dispensa checar `nullptr`.
- **Ponteiro (`T*`)** = pode ser **nulo** ("ainda não tem") e pode **mudar** de
  alvo. Usado onde isso acontece: `pokemonAtual` (nulo antes da seleção),
  `vencedor` (nulo até a batalha acabar), a troca de Pokémon após uma derrota.

Por que **ponteiros no modelo** (`IPokemon*`, `Ataque*`):
1. **Polimorfismo** — chamar um método `virtual` (ex.: `calcular()` de
   `AtaqueNormal/Critico/SuperEfetivo`) exige ponteiro/referência à base; objeto
   "por valor" perderia o tipo real (*slicing*).
2. **Identidade e compartilhamento** — o **mesmo** Pokémon aparece em vários
   lugares (deck do jogador, `pokemonAtual`, `pokemonVencedor` da `Rodada`,
   atacante/defensor). Todos precisam ver **a mesma instância**: vida e energia
   mudam durante a luta, e cópia quebraria isso.
3. **Nulidade legítima** — referência não representa "ainda não existe"; ponteiro
   sim.

Por que **não** `new`/`delete`/smart pointers na batalha:
- **A posse é externa.** Quem cria os `Pokemon`/`Jogador` é a carga do SD
  (`carregador`, sobre o pool estático no `main.cpp`); a `Batalha` só **usa**
  esses objetos (ponteiros não-donos), então nunca dá `delete`.
- **Embarcado evita heap** — `new`/`delete` repetidos fragmentam a RAM do ESP32.
  Os objetos vivem como globais/estáticos, com tempo de vida do programa.

Convenções de apoio:
- `executarTurno()` retorna **`-1`** como sentinela de "golpe inválido" (sem
  energia / índice fora / batalha encerrada) — a tela então reescolhe.
- Métodos que não alteram o objeto são `const`.

---

## 6. Fluxo da aplicação

```
LOGIN ──(2 jogadores logam, recebem 5 Pokémon aleatórios)──▶ SELECAO
SELECAO ──(cada um escolhe o Pokémon ativo)──▶ BATALHA
BATALHA ──(alguém zera o time do outro)──▶ RESULTADO
RESULTADO ──(SELECT)──▶ LOGIN   (publica o resultado no MQTT)
```

Regras da batalha (na engine `Batalha`):
- Turnos alternados; a 1ª rodada começa por sorteio, depois começa quem perdeu a
  rodada anterior.
- Energia é **por Pokémon**, persiste entre turnos e regenera a cada turno.
- Dano usa **mitigação proporcional** pela defesa (não subtração), com piso de 1.
- Vence quem derrotar todos os Pokémon do adversário. Sem empate.

---

## 7. Camadas, testes e build

- **Dependências:** `dominio/` depende **apenas** de `contratos/`. `infra/` e
  `app/` dependem do domínio e do hardware. Essa direção é o que torna a engine
  testável isoladamente.
- **Testes (PC):** `pio test -e native` — 9 testes Unity sobre a engine pura
  (ataques, tabela de tipos, energia, fluxo de rodadas, vencedor). O build
  `native` exclui `main.cpp`, `infra/` e `app/` (código Arduino).
- **Firmware (ESP32):** `pio run -e esp32dev` (compila) e `-t upload` (grava).

---

## 8. Resumo dos padrões (cola para a apresentação)

| Padrão | Tipo | Onde | Em uma frase |
|---|---|---|---|
| **SPA** | Arquitetura | `ControladorAplicacao` + `main.cpp` | Uma tela só; troca o componente/estado ativo no lugar. |
| **State** | GoF comportamental | `Estado` + `EstadoX` + `ControladorAplicacao` | Cada tela encapsula seu comportamento e suas transições. |
| **Singleton** | GoF criacional | `ControladorAplicacao::instancia()` | Um único orquestrador global. |
| **Adapter** | GoF estrutural | `PublicadorMqtt`→`IPublicadorResultado`; `AutenticadorHttp`→`IAutenticador` | Envolve serviço externo (MQTT, login HTTP) atrás de um contrato de I/O. |
| **Template Method** | GoF comportamental | `Ataque::executar()` + subclasses | Esqueleto fixo do ataque; subclasses variam só o cálculo do dano. |
