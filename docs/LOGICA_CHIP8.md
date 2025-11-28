# Documentação Detalhada das Funções do Chip8.cpp

Este documento explica detalhadamente a lógica de cada função implementada na classe `Chip8`, responsável pelo núcleo da máquina virtual Chip-8.

---

## Construtor e Inicialização

### `Chip8::Chip8(uint32_t frequency)`
- Inicializa todos os registradores, memória, pilha, timers, display e input.
- Define o PC (Program Counter) para 0x200 (endereço padrão de início das ROMs).
- Carrega o conjunto de fontes (sprites dos dígitos 0-F) na memória.
- Sementeia o gerador de números aleatórios.

### `void initialize()`
- Zera memória, registradores, pilha e variáveis de controle.
- Reinicializa timers, display e input.
- Copia o conjunto de fontes para o início da memória.
- Imprime informações de debug sobre o estado inicial.

### `void load_rom(const char* filename, uint16_t load_address = 0x200)`
- Abre o arquivo ROM e lê seu conteúdo para a memória a partir de `load_address`.
- Verifica tamanho máximo permitido.
- Imprime logs de sucesso ou erro.

---

## Funções de Entrada/Saída e Gráficos

### `void set_key_pressed(uint8_t key_value)`
- Se a VM está aguardando uma tecla (opcode FX0A), armazena o valor pressionado no registrador correto e libera o estado de espera.

### `bool init_display_graphics(uint32_t scale)`
- Inicializa a janela e renderizador SDL com o fator de escala desejado.

### `void render_display()`
- Solicita ao display que desenhe o buffer de pixels na tela.

### `void destroy_display_graphics()`
- Libera recursos gráficos SDL.

---

## Funções de Entrada e Temporizadores

### `void process_input(SDL_Event& event)`
- Encaminha o evento SDL para o componente de input, que atualiza o estado das teclas.

### `void update_timers()`
- Atualiza os timers (Delay e Sound), decrementando-os a 60Hz e controlando o áudio.

---

## Ciclo de Execução

### `uint16_t fetch_opcode()`
- Busca dois bytes da memória no endereço do PC, formando o opcode de 16 bits.
- Incrementa o PC em 2.

### `void cycle()`
- Executa um ciclo da CPU:
  1. Busca o próximo opcode.
  2. Chama `execute_opcode` para decodificar e executar a instrução.
  3. Imprime logs de debug.

---

## Decodificação e Execução de Instruções


### `void execute_opcode(uint16_t opcode)` — Detalhamento Completo

Esta função é o núcleo do interpretador Chip-8. Ela recebe um opcode de 16 bits, decodifica seus campos e executa a instrução correspondente. Abaixo, cada grupo de instruções é detalhado:

#### Decodificação Inicial
- Extrai campos do opcode:
  - `nnn`: endereço de 12 bits (opcode & 0x0FFF)
  - `x`: registrador X (opcode & 0x0F00) >> 8
  - `y`: registrador Y (opcode & 0x00F0) >> 4
  - `nn`: byte de 8 bits (opcode & 0x00FF)
  - `n`: nibble de 4 bits (opcode & 0x000F)

#### Grupo 0x0NNN
- **0x00E0 (CLS):** Limpa a tela chamando `display.clear_screen()`.
- **0x00EE (RET):** Retorna de sub-rotina. Faz pop da pilha e define o PC para o endereço salvo.
- **0x0NNN:** (Chamada de máquina) — ignorado, apenas loga aviso.

#### Grupo 0x1NNN (JP addr)
- Salta para o endereço `nnn` (PC = nnn).

#### Grupo 0x2NNN (CALL addr)
- Chama sub-rotina: salva o PC atual na pilha e salta para `nnn`.

#### Grupo 0x3XNN (SE Vx, byte)
- Pula a próxima instrução se Vx == nn (PC += 2).

#### Grupo 0x4XNN (SNE Vx, byte)
- Pula a próxima instrução se Vx != nn (PC += 2).

#### Grupo 0x5XY0 (SE Vx, Vy)
- Pula a próxima instrução se Vx == Vy (PC += 2).

#### Grupo 0x6XNN (LD Vx, byte)
- Carrega o valor nn em Vx.

#### Grupo 0x7XNN (ADD Vx, byte)
- Soma nn a Vx (sem carry).

#### Grupo 0x8XYN (Operações aritméticas/lógicas)
- **8xy0:** Vx = Vy
- **8xy1:** Vx = Vx | Vy
- **8xy2:** Vx = Vx & Vy
- **8xy3:** Vx = Vx ^ Vy
- **8xy4:** Vx += Vy, VF=1 se overflow
- **8xy5:** Vx -= Vy, VF=1 se não houve borrow
- **8xy6:** Vx >>= 1, VF recebe o bit menos significativo
- **8xy7:** Vx = Vy - Vx, VF=1 se não houve borrow
- **8xyE:** Vx <<= 1, VF recebe o bit mais significativo

#### Grupo 0x9XY0 (SNE Vx, Vy)
- Pula a próxima instrução se Vx != Vy (PC += 2).

#### Grupo 0xANNN (LD I, addr)
- Carrega o valor nnn no registrador I.

#### Grupo 0xBNNN (JP V0, addr)
- Salta para nnn + V0.

#### Grupo 0xCXNN (RND Vx, byte)
- Gera um número aleatório (0-255), faz AND com nn e armazena em Vx.

#### Grupo 0xDXYN (DRW Vx, Vy, nibble)
- Desenha um sprite de N linhas na tela, começando em (Vx, Vy), lendo da memória a partir de I.
- Cada linha do sprite tem 8 bits (1 byte).
- Pixels são desenhados com XOR; se algum pixel foi desligado (1→0), VF=1 (colisão).

#### Grupo 0xEXNN (Teclado)
- **Ex9E (SKP Vx):** Pula se a tecla Vx está pressionada.
- **ExA1 (SKNP Vx):** Pula se a tecla Vx NÃO está pressionada.

#### Grupo 0xFXNN (Timers, memória, teclado)
- **Fx07:** Vx = valor do Delay Timer
- **Fx0A:** Aguarda uma tecla ser pressionada, armazena em Vx
- **Fx15:** Delay Timer = Vx
- **Fx18:** Sound Timer = Vx
- **Fx1E:** I += Vx
- **Fx29:** I = endereço do sprite hexadecimal de Vx
- **Fx33:** Armazena BCD de Vx em memória[I], memória[I+1], memória[I+2]
- **Fx55:** Salva V0 até Vx na memória a partir de I
- **Fx65:** Carrega V0 até Vx da memória a partir de I

#### Tratamento de Erros
- Instruções desconhecidas ou inválidas geram logs de erro e podem encerrar a execução.

#### Exemplo de fluxo (Opcode DXYN — Desenho de Sprite)
1. Lê N bytes da memória a partir de I (cada byte = 1 linha do sprite)
2. Para cada bit do byte, faz XOR com o pixel correspondente do display
3. Se algum pixel foi desligado, VF=1
4. O display é atualizado na próxima chamada de renderização

#### Observações
- O método é chamado a cada ciclo da CPU, garantindo a execução sequencial das instruções da ROM.
- O uso de logs detalhados facilita o debug e a validação do funcionamento.

---

## Observações Gerais
- O código segue fielmente a especificação do Chip-8.
- O método `execute_opcode` é o núcleo da emulação, responsável por interpretar e executar cada instrução da ROM.
- O design modular facilita manutenção e extensão.

---

Para detalhes linha a linha de cada instrução, consulte os comentários no próprio código-fonte ou solicite explicações específicas de cada opcode.
