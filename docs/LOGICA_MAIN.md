# 📚 Documentação Detalhada das Funções do main.cpp

Este documento explica detalhadamente a lógica de cada função implementada no arquivo `main.cpp`, responsável pelo controle principal do emulador Chip-8.

---

## 🔧 Constantes e Configurações

### Constantes de Timing
```cpp
constexpr int DEFAULT_CPU_HZ = 500;
constexpr int PERIPHERAL_HZ = 60;
constexpr auto US_PER_60HZ_CYCLE = duration_cast<microseconds>(seconds(1)) / PERIPHERAL_HZ;
```

- **DEFAULT_CPU_HZ**: Frequência padrão da CPU (500Hz = 500 instruções por segundo)
- **PERIPHERAL_HZ**: Frequência dos periféricos (60Hz para timers e renderização)
- **US_PER_60HZ_CYCLE**: Duração de cada ciclo de 60Hz em microssegundos (≈16,667μs)

### Constantes de Display
```cpp
constexpr uint32_t DEFAULT_SCALE = 10;
uint32_t scale_factor = DEFAULT_SCALE;
```

- **DEFAULT_SCALE**: Fator de escala padrão para a janela (10x = 640x320 pixels)
- **scale_factor**: Variável global que armazena o fator de escala atual

---

## 🔍 Análise de Argumentos

### `uint32_t parse_args(int argc, char* argv[], const char** rom_path, uint32_t default_clock)`

**Propósito:** Analisa argumentos da linha de comando e configura parâmetros do emulador.

**Lógica detalhada:**

1. **Inicialização:**
   - Define `clock_hz` com o valor padrão
   - Itera pelos argumentos a partir do índice 1

2. **Processamento de argumentos:**
   
   **`--clock <valor>`:**
   - Converte o próximo argumento para inteiro usando `std::stoul()`
   - Se conversão falhar, captura exceção e usa valor padrão
   - Imprime log de debug com a configuração

   **`--scale <valor>`:**
   - Similar ao clock, mas configura `scale_factor`
   - Controla o zoom da janela do emulador
   - Valor padrão mantido se conversão falhar

   **Arquivo ROM:**
   - Qualquer argumento que não comece com `--` é considerado caminho da ROM
   - Atribui o ponteiro `rom_path` para este argumento

3. **Tratamento de erros:**
   - Usa blocos try-catch para argumentos inválidos
   - Mantém valores padrão em caso de erro
   - Imprime mensagens de erro informativas

**Retorno:** Frequência da CPU configurada (Hz)

**Exemplo de uso:**
```bash
./chip8_emulator --clock 700 --scale 15 roms/pong.ch8
```

---

## 🎯 Função Principal

### `int main(int argc, char* argv[])`

**Propósito:** Ponto de entrada principal do programa. Controla todo o ciclo de vida do emulador.

**Estrutura em 4 seções:**

#### **1. Configuração Inicial e Parse de Argumentos**

```cpp
const char* rom_path = nullptr;
uint32_t clock_hz = parse_args(argc, argv, &rom_path, DEFAULT_CPU_HZ);

if (!rom_path) {
    // Erro: nenhuma ROM especificada
    std::cerr << "ERRO: Forneca o caminho para o arquivo ROM (.ch8) como argumento." << std::endl;
    return 1;
}
```

**Lógica:**
- Chama `parse_args()` para processar argumentos
- Verifica se uma ROM foi especificada
- Se não, imprime mensagem de uso e encerra com código 1

#### **2. Inicialização SDL e Criação da VM**

```cpp
if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    std::cerr << "ERRO SDL: Falha ao inicializar SDL: " << SDL_GetError() << std::endl;
    return 1;
}

Chip8 emulator(clock_hz);
emulator.load_rom(rom_path, 0x200);

if (!emulator.init_display_graphics(scale_factor)) {
    SDL_Quit();
    return 1;
}
```

**Lógica:**
- Inicializa SDL com subsistemas de eventos, vídeo e áudio
- Cria instância do emulador com frequência configurada
- Carrega ROM no endereço 0x200 (padrão Chip-8)
- Inicializa sistema gráfico com fator de escala
- Se qualquer etapa falhar, limpa recursos e encerra

#### **3. Preparação do Loop Principal**

```cpp
long long cycles_executed_total = 0;
auto start_time = high_resolution_clock::now();

const auto CPU_CYCLE_DURATION = duration_cast<microseconds>(seconds(1)) / clock_hz;
auto last_60hz_tick = high_resolution_clock::now();
bool quit = false;
```

**Variáveis de controle:**
- **cycles_executed_total**: Contador para validação de performance
- **start_time**: Marca temporal do início da execução
- **CPU_CYCLE_DURATION**: Duração alvo de cada ciclo da CPU
- **last_60hz_tick**: Última atualização dos periféricos
- **quit**: Flag de controle do loop principal

#### **4. Loop Principal de Execução**

O loop principal executa 4 operações por iteração:

**A. Processamento de Input (Eventos SDL):**
```cpp
SDL_Event event;
while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT) {
        quit = true;
    } else {
        emulator.process_input(event);
    }
}
```

- Processa todos os eventos SDL pendentes
- Detecta solicitação de fechamento da janela
- Encaminha eventos de teclado para o emulador

**B. Ciclo da CPU (Fetch-Decode-Execute):**
```cpp
if (!emulator.is_waiting_for_key()) {
    emulator.cycle();
    cycles_executed_total++;
}
emulator.cycle();
cycles_executed_total++;
```

- Executa um ciclo da CPU (busca e executa instrução)
- Incrementa contador de ciclos para estatísticas
- **Nota:** Há duplicação no código (possível bug)

**C. Controle de Timing da CPU:**
```cpp
auto cpu_end_time = high_resolution_clock::now();
auto elapsed_cpu_time = duration_cast<microseconds>(cpu_end_time - cpu_start_time);

if (elapsed_cpu_time < CPU_CYCLE_DURATION) {
    std::this_thread::sleep_for(CPU_CYCLE_DURATION - elapsed_cpu_time);
}
```

- Mede tempo gasto no ciclo da CPU
- Se o ciclo terminou antes do tempo alvo, dorme pelo tempo restante
- Garante frequência constante da CPU (ex: 500Hz)

**D. Controle de Timing dos Periféricos (60Hz):**
```cpp
auto now = high_resolution_clock::now();
auto elapsed_60hz_time = now - last_60hz_tick;

if (elapsed_60hz_time >= US_PER_60HZ_CYCLE) {
    emulator.update_timers();
    emulator.render_display();
    last_60hz_tick = now;
}
```

- Verifica se passou tempo suficiente desde a última atualização de periféricos
- A cada ~16,667μs (60Hz):
  - Atualiza timers (decrementa Delay e Sound Timer)
  - Renderiza tela
  - Atualiza marca temporal

#### **5. Encerramento e Validação Final**

```cpp
emulator.destroy_display_graphics();

auto end_time = high_resolution_clock::now();
auto total_duration = duration_cast<seconds>(end_time - start_time);

if (total_duration.count() > 0) {
    double average_actual_hz = (double)cycles_executed_total / total_duration.count();
    
    std::cout << "\n=================================================" << std::endl;
    std::cout << "VALIDACAO FINAL (ISSUE 6 - PERFORMANCE):" << std::endl;
    std::cout << "Tempo total de execucao: " << total_duration.count() << " segundos." << std::endl;
    std::cout << "Total de ciclos executados: " << cycles_executed_total << std::endl;
    std::cout << "Frequencia media da CPU: " << std::fixed << std::setprecision(2) 
              << average_actual_hz << " Hz (Alvo: " << clock_hz << " Hz)." << std::endl;
    std::cout << "=================================================" << std::endl;
}

SDL_Quit();
```

**Lógica de encerramento:**
- Libera recursos gráficos
- Calcula estatísticas de performance:
  - Tempo total de execução
  - Ciclos executados
  - Frequência média real vs. alvo
- Imprime relatório de validação
- Encerra SDL
- Retorna 0 (sucesso)

---

## 🔄 Fluxo de Execução Temporal

### Exemplo com CPU a 500Hz:

1. **Ciclo da CPU:** 1/500s = 2ms por instrução
2. **Periféricos:** 1/60s ≈ 16,67ms por atualização
3. **Proporção:** A cada 8-9 instruções, atualiza periféricos

### Cronograma típico:
```
0ms:    Instrução 1, Atualiza periféricos
2ms:    Instrução 2
4ms:    Instrução 3
...
16ms:   Instrução 8, Atualiza periféricos
18ms:   Instrução 9
...
```

---

## 🎯 Características Importantes

### **Precisão de Timing:**
- Usa `high_resolution_clock` para medições precisas
- Implementa controle ativo de frequência com `sleep_for()`
- Compensa variações no tempo de execução das instruções

### **Tratamento de Erros:**
- Validação robusta de argumentos
- Limpeza de recursos em caso de falha
- Mensagens de erro informativas

### **Validação de Performance:**
- Coleta estatísticas durante execução
- Calcula frequência real vs. configurada
- Útil para debug e otimização

### **Arquitetura Modular:**
- Separação clara entre timing da CPU e periféricos
- Interface limpa com a classe Chip8
- Facilita manutenção e testes

---

## 📊 Observações Técnicas

### **Possíveis Melhorias:**
1. **Duplicação de código:** O `emulator.cycle()` aparece duplicado
2. **Granularidade de timing:** Poderia usar nanosegundos para maior precisão
3. **Tratamento de lag:** Não há compensação para atrasos acumulados

### **Pontos Fortes:**
1. **Controle preciso de frequência**
2. **Logging extensivo para debug**
3. **Tratamento robusto de erros**
4. **Arquitetura limpa e modular**

---

Este arquivo documenta todo o fluxo de controle do emulador, desde a análise de argumentos até o encerramento, destacando as decisões de design e a lógica de timing que garante a execução precisa do Chip-8.