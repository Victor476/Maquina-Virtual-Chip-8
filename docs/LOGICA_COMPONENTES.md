# 📚 Documentação da Lógica dos Componentes

Este documento explica detalhadamente a lógica de cada função implementada nos componentes do emulador Chip-8.

---

## 📺 Display.cpp/h

### **Responsabilidade**
Gerencia o buffer de pixels 64x32 e a renderização via SDL3.

---

### `Display::Display()`
**Propósito:** Construtor da classe Display.

**Lógica:**
1. Inicializa os ponteiros `window` e `renderer` como `nullptr`
2. Define `scale_factor` como 10 (padrão)
3. Limpa o buffer de pixels usando `clear_screen()`

**Motivo:** Garante que o objeto Display sempre comece em estado seguro e previsível.

---

### `void Display::clear_screen()`
**Propósito:** Limpa todo o buffer de pixels (equivalente ao opcode 00E0).

**Lógica:**
```
1. Usa std::fill para preencher todo pixel_buffer com 0
2. Cada posição representa um pixel: 0 = apagado, 1 = aceso
```

**Por que funciona:**
- `pixel_buffer` é um array linear de 2048 posições (64×32)
- Zerar todos os valores apaga toda a tela
- Operação O(n) mas acontece raramente

**Validação:** Imprime mensagem de debug confirmando a limpeza.

---

### `bool Display::init_graphics(uint32_t scale)`
**Propósito:** Inicializa a janela SDL e o renderizador.

**Lógica passo a passo:**
```
1. Armazena o scale_factor recebido (ex: 10 = janela 640×320)

2. Calcula dimensões da janela:
   - window_width = 64 × scale_factor
   - window_height = 32 × scale_factor

3. Cria janela SDL com título "Chip-8 Emulator"
   - Se falhar → retorna false

4. Cria renderizador SDL (acelerado por hardware)
   - Se falhar → destroi janela e retorna false

5. Define cor de desenho padrão (branco: 255, 255, 255)

6. Retorna true se tudo funcionou
```

**Decisão arquitetural:** 
- Usa `SDL_WINDOW_RESIZABLE` para permitir redimensionamento
- Validação robusta: se qualquer etapa falhar, limpa recursos alocados

**Validação:** Logs de debug informam sucesso ou erro em cada etapa.

---

### `void Display::render()`
**Propósito:** Desenha o buffer de pixels na janela SDL.

**Lógica do algoritmo:**
```
1. Limpa o renderizador (fundo preto)

2. Para cada pixel no buffer (2048 iterações):
   a. Se pixel == 1 (aceso):
      - Calcula posição X: index % 64
      - Calcula posição Y: index / 64
      - Cria SDL_FRect com:
        * x = pixel_x × scale_factor
        * y = pixel_y × scale_factor
        * w = scale_factor
        * h = scale_factor
      - Desenha retângulo branco na posição

3. Apresenta o renderizador (atualiza janela)
```

**Otimização possível:** Poderia usar batch rendering, mas para 2048 pixels é rápido o suficiente.

**Por que funciona:**
- Conversão de índice linear (0-2047) para coordenadas 2D (x,y)
- Scale_factor transforma coordenadas lógicas (64×32) em pixels reais
- Apenas pixels ligados (1) são desenhados → fundo fica preto naturalmente

---

### `void Display::destroy_graphics()`
**Propósito:** Libera recursos SDL ao encerrar.

**Lógica:**
```
1. Se renderer existe:
   - SDL_DestroyRenderer(renderer)
   - Define renderer = nullptr

2. Se window existe:
   - SDL_DestroyWindow(window)
   - Define window = nullptr
```

**Por que é importante:**
- Previne memory leaks
- SDL não libera recursos automaticamente
- Definir como nullptr previne dangling pointers

---

## ⌨️ Input.cpp/h

### **Responsabilidade**
Gerencia entrada do teclado e mapeia teclas físicas para teclas Chip-8.

---

### `Input::Input()`
**Propósito:** Construtor da classe Input.

**Lógica:**
```
1. Chama reset_keys() → zera estado de todas as teclas
2. Chama setup_key_map() → configura mapeamento QWERTY
3. Imprime log de debug
```

**Inicialização ordenada:** Garante que o estado seja consistente antes de qualquer evento.

---

### `void Input::reset_keys()`
**Propósito:** Reseta o estado de todas as 16 teclas Chip-8.

**Lógica:**
```
1. Usa key_state.fill(false)
2. key_state é std::array<bool, 16>
3. Índices 0x0 a 0xF representam teclas hexadecimais
```

**Quando é usado:**
- Na inicialização
- Potencialmente ao resetar o emulador

---

### `void Input::setup_key_map()`
**Propósito:** Configura o mapeamento entre teclas físicas (QWERTY) e teclas Chip-8 (0-F).

**Lógica do mapeamento:**
```
Layout Chip-8:        Layout QWERTY:
1 2 3 C               1 2 3 4
4 5 6 D       →       Q W E R
7 8 9 E               A S D F
A 0 B F               Z X C V
```

**Implementação:**
```cpp
key_map[0x1] = SDLK_1;  // Chip-8 '1' → Teclado '1'
key_map[0x2] = SDLK_2;  // Chip-8 '2' → Teclado '2'
key_map[0x4] = SDLK_Q;  // Chip-8 '4' → Teclado 'Q'
// ... etc
```

**Decisão de design:**
- Mantém layout ergonômico em QWERTY
- 4×4 compacto, fácil de usar com uma mão
- Correspondência intuitiva (1,2,3 permanecem iguais)

---

### `void Input::handle_event(SDL_Event& event)`
**Propósito:** Processa eventos SDL e atualiza o estado das teclas.

**Lógica detalhada:**
```
1. Verifica se evento é KEY_DOWN ou KEY_UP:
   - Se não for → retorna (ignora outros eventos)

2. Extrai informações:
   - key_code = código da tecla física pressionada
   - is_pressed = true se KEY_DOWN, false se KEY_UP

3. Busca no mapeamento (loop de 0 a 15):
   Para cada índice i:
   a. Se key_code == key_map[i]:
      - Atualiza key_state[i] = is_pressed
      - Imprime log: "Tecla Chip-8 0xX PRESSIONADA/LIBERADA"
      - Retorna (encontrou, não precisa continuar)

4. Se nenhuma correspondência → não faz nada (tecla ignorada)
```

**Por que funciona:**
- key_state[i] representa estado atual da tecla Chip-8 'i'
- Opcodes EX9E/EXA1 consultam este array diretamente
- Loop de busca é O(16) = constante, aceitável

**Exemplo de fluxo:**
```
Usuário pressiona 'W'
→ SDL gera KEY_DOWN com SDLK_W
→ handle_event detecta que key_map[0x5] == SDLK_W
→ key_state[0x5] = true
→ Opcode EX9E pode verificar se tecla '5' está pressionada
```

**Validação:** Logs mostram qual tecla Chip-8 foi afetada e o nome da tecla física.

---

## ⏱️ TimerManager.cpp/h

### **Responsabilidade**
Gerencia Delay Timer, Sound Timer e geração de áudio.

---

### `TimerManager::TimerManager()`
**Propósito:** Construtor da classe TimerManager.

**Lógica:**
```
1. Inicializa delay_timer = 0
2. Inicializa sound_timer = 0
3. Inicializa is_audio_playing = false
4. Inicializa audio_device_id = 0
```

**Estado inicial seguro:** Nenhum timer ativo, áudio desligado.

---

### `bool TimerManager::init_audio()`
**Propósito:** Inicializa o sistema de áudio SDL.

**Lógica passo a passo:**
```
1. Cria SDL_AudioSpec desired_spec com:
   - freq = 44100 Hz (taxa de amostragem padrão)
   - format = SDL_AUDIO_F32 (float de 32 bits)
   - channels = 1 (mono)

2. Abre dispositivo de áudio:
   - SDL_OpenAudioDevice(0, &desired_spec)
   - 0 = não é dispositivo de captura (apenas reprodução)
   - Retorna ID do dispositivo

3. Se falhar (ID == 0):
   - Imprime erro com SDL_GetError()
   - Retorna false

4. Se sucesso:
   - Pausa áudio inicialmente (SDL_PauseAudioDevice)
   - Define is_audio_playing = false
   - Retorna true
```

**Por que pausar inicialmente:**
- Sound Timer começa em 0
- Áudio só deve tocar quando ST > 0
- Evita som indesejado na inicialização

**Validação:** Log de debug confirma inicialização bem-sucedida.

---

### `void AudioCallback(void *userdata, Uint8 *stream, int len)`
**Propósito:** Função callback que gera a onda sonora (chamada automaticamente pela SDL).

**Lógica do algoritmo:**
```
1. Converte buffer de bytes para float:
   - audio_stream = (float*)stream
   - num_samples = len / sizeof(float)

2. Para cada sample (loop):
   a. Calcula valor da onda senoidal:
      - value = sin(audio_phase × 2π)
      - Produz onda suave entre -1.0 e 1.0
   
   b. Escreve no buffer:
      - audio_stream[i] = value × 0.2
      - Multiplicador 0.2 reduz volume (evita clipping)
   
   c. Atualiza fase:
      - audio_phase += FREQUENCY / SAMPLE_RATE
      - FREQUENCY = 440 Hz (nota Lá/A4)
      - SAMPLE_RATE = 44100 Hz
      - Se phase >= 1.0 → phase -= 1.0 (wrap around)

3. Callback retorna → SDL reproduz buffer
```

**Matemática da onda:**
```
Frequência = 440 Hz (ciclos por segundo)
Sample Rate = 44100 amostras por segundo
Incremento por sample = 440/44100 ≈ 0.00997

A cada 100 samples:
- Fase avança ≈ 0.997
- Quase um ciclo completo da senoide
- 440 ciclos completos em 1 segundo = 440 Hz ✓
```

**Por que senoidal:**
- Tom puro e agradável
- Mais suave que onda quadrada
- Fácil de gerar matematicamente

---

### `void TimerManager::start_sound()`
**Propósito:** Inicia reprodução do áudio (quando ST > 0).

**Lógica:**
```
1. Verifica if (!is_audio_playing):
   - Evita chamar múltiplas vezes desnecessariamente

2. Se não está tocando:
   - SDL_ResumeAudioDevice(audio_device_id)
   - Define is_audio_playing = true
   - Imprime log de debug
```

**Por que verificar flag:**
- `SDL_ResumeAudioDevice` é idempotente, mas o check evita logs repetitivos
- Economiza chamadas SDL desnecessárias

---

### `void TimerManager::stop_sound()`
**Propósito:** Para reprodução do áudio (quando ST == 0).

**Lógica:**
```
1. Verifica if (is_audio_playing):
   - Só para se realmente estiver tocando

2. Se está tocando:
   - SDL_PauseAudioDevice(audio_device_id)
   - Define is_audio_playing = false
   - Imprime log de debug
```

**Simetria com start_sound:** Mesmo padrão de verificação de flag.

---

### `void TimerManager::update_timers()`
**Propósito:** Decrementa os timers e controla o áudio (chamado a 60Hz).

**Lógica completa:**
```
1. Delay Timer:
   if (delay_timer > 0):
       delay_timer--

2. Sound Timer e controle de áudio:
   if (sound_timer > 0):
       sound_timer--
       
       if (sound_timer > 0):  ← Ainda maior que 0 após decremento
           start_sound()      ← Garante que som esteja tocando
       else:                   ← Chegou a 0 agora
           stop_sound()        ← Para o som
   else:                       ← Já estava em 0
       stop_sound()            ← Garante que som esteja parado
```

**Fluxo de estados do Sound Timer:**
```
ST = 10 → update → ST = 9, start_sound()
ST = 9  → update → ST = 8, start_sound()
...
ST = 2  → update → ST = 1, start_sound()
ST = 1  → update → ST = 0, stop_sound()  ← SOM PARA AQUI
ST = 0  → update → ST = 0, stop_sound()  (redundante mas seguro)
```

**Decisão de design:**
- Decrementa ANTES de verificar
- Dupla verificação garante que som pare no momento exato
- Stop redundante em else não prejudica (idempotente)

**Validação:** Logs mostram quando som inicia/para.

---

### `void TimerManager::destroy_audio()`
**Propósito:** Libera recursos de áudio ao encerrar.

**Lógica:**
```
1. if (audio_device_id != 0):
   - SDL_CloseAudioDevice(audio_device_id)
   - audio_device_id = 0
```

**Por que verificar ID:**
- Se init_audio() falhou, ID seria 0
- Evita fechar dispositivo inválido

---

### Getters e Setters

#### `uint8_t get_delay_timer()`
**Propósito:** Retorna valor atual do Delay Timer (usado pelo opcode FX07).

**Lógica:** Simplesmente retorna `delay_timer`.

---

#### `uint8_t get_sound_timer()`
**Propósito:** Retorna valor atual do Sound Timer.

**Lógica:** Simplesmente retorna `sound_timer`.

---

#### `void set_delay_timer(uint8_t value)`
**Propósito:** Define novo valor para Delay Timer (opcode FX15).

**Lógica:**
```
1. delay_timer = value
2. Imprime log mostrando novo valor
```

**Comportamento:** Timer começa a decrementar na próxima chamada de `update_timers()`.

---

#### `void set_sound_timer(uint8_t value)`
**Propósito:** Define novo valor para Sound Timer (opcode FX18).

**Lógica:**
```
1. sound_timer = value
2. Imprime log mostrando novo valor
```

**Comportamento:**
- Se value > 0 → áudio começará a tocar no próximo `update_timers()`
- Se value == 0 → áudio parará (se estiver tocando)

---

## 🔄 Interação Entre Componentes

### **Exemplo Completo: Opcode DXYN (Desenhar Sprite)**

```
1. CPU executa opcode DXYN em Chip8.cpp
   ↓
2. Lê sprite da memória (endereço I)
   ↓
3. Acessa display.pixel_buffer diretamente
   ↓
4. Para cada pixel do sprite:
   - Faz XOR com pixel atual
   - Atualiza pixel_buffer[index]
   - Detecta colisão (1→0)
   ↓
5. Define VF = 1 se houve colisão
   ↓
6. No próximo tick 60Hz, main.cpp chama:
   emulator.render_display()
   ↓
7. Chip8 chama display.render()
   ↓
8. Display lê pixel_buffer e desenha na tela
```

### **Exemplo Completo: Opcode FX18 (Set Sound Timer)**

```
1. CPU executa opcode FX18 em Chip8.cpp
   ↓
2. Chama timers.set_sound_timer(V[x])
   ↓
3. TimerManager armazena novo valor
   ↓
4. No próximo tick 60Hz, main.cpp chama:
   emulator.update_timers()
   ↓
5. Chip8 chama timers.update_timers()
   ↓
6. TimerManager decrementa sound_timer
   ↓
7. Se ST > 0 → start_sound()
   ↓
8. SDL_ResumeAudioDevice ativa callback
   ↓
9. AudioCallback gera onda senoidal continuamente
   ↓
10. Quando ST chega a 0 → stop_sound()
```

---

## 📊 Complexidade Temporal

| Função | Complexidade | Justificativa |
|--------|--------------|---------------|
| `Display::clear_screen()` | O(2048) | Preenche todo buffer |
| `Display::render()` | O(2048) | Itera sobre todos pixels |
| `Input::handle_event()` | O(16) | Busca linear no mapeamento |
| `TimerManager::update_timers()` | O(1) | Apenas decrementos e comparações |
| `AudioCallback()` | O(n) | n = número de samples (fixo por chunk) |

---

## 🎯 Princípios Aplicados

### **Single Responsibility Principle (SRP)**
- Display → Apenas gráficos
- Input → Apenas teclado
- TimerManager → Timers + áudio relacionado

### **Don't Repeat Yourself (DRY)**
- `reset_keys()` reutilizado no construtor
- `start_sound()` e `stop_sound()` evitam duplicação de lógica SDL

### **Fail-Fast**
- `init_graphics()` e `init_audio()` retornam false ao primeiro erro
- Logs imediatos facilitam debugging

### **Defensive Programming**
- Verificações `if (is_audio_playing)` previnem estados inválidos
- Ponteiros nullptr após destruição
- Validação de retornos SDL

---

## 📝 Conclusão

Os componentes demonstram:
- ✅ **Lógica clara e bem comentada**
- ✅ **Separação de responsabilidades**
- ✅ **Tratamento robusto de erros**
- ✅ **Logging extensivo para debugging**
- ✅ **Eficiência adequada ao contexto**

Cada função tem propósito bem definido e implementação direta, facilitando manutenção e extensão futura.
