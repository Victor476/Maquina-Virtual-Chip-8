# 👾 Maquina-Virtual-Chip-8

Este projeto consiste na implementação de uma **Máquina Virtual (VM)** para a plataforma **Chip-8**, uma linguagem de programação interpretada simples desenvolvida nos anos 70. O objetivo principal é simular a arquitetura básica de um sistema computacional, incluindo CPU, memória, registradores e periféricos, conforme detalhado na especificação do trabalho.

O emulador é desenvolvido em **C++** e utiliza a biblioteca **SDL (Simple DirectMedia Layer)** para gerenciamento de gráficos, teclado e áudio.

-----

## 🏗️ Arquitetura e Componentes

A arquitetura de software é modular e orientada a componentes, simulando o hardware Chip-8:

  * **CPU Core:** Implementa o ciclo **Fetch-Decode-Execute** de instruções de 16 bits. [cite\_start]Gerencia os 16 registradores de 8 bits (V0-VF), o Registrador de Endereço (I), o Program Counter (PC) e a Pilha [cite: 29-43].
  * **Memória:** Possui **4KB (4096 bytes)** de RAM. O programa (ROM) é carregado no espaço de `0x200` a `0xFFF`. [cite\_start]Os sprites dos dígitos hexadecimais são pré-carregados na área reservada a partir de `0x000` [cite: 22-26, 48].
  * [cite\_start]**Display:** Simula a tela **monocromática de 64x32 pixels**[cite: 44]. [cite\_start]A renderização é feita via sprites e a tela é atualizada a **60Hz**[cite: 143].
  * [cite\_start]**Temporizadores:** Implementa o **Delay Timer (DT)** e o **Sound Timer (ST)**, ambos de 8 bits, que decrementam a uma taxa de **60Hz**[cite: 109, 144].
  * [cite\_start]**Input:** Simula o teclado hexadecimal de 16 teclas e implementa o mapeamento configurável das entradas do teclado físico [cite: 97, 103-104].

-----

## ⚙️ Configuração e Execução

### Pré-requisitos

Para compilar e executar o projeto, você precisará de:

1.  Um compilador C++ (g++ ou clang++).
2.  Um sistema de compilação (**CMake** ou **Make**).
3.  A biblioteca **SDL2** (Simple DirectMedia Layer) e suas bibliotecas relacionadas (para gráficos, som e entrada).

### Estrutura de Pastas

Todo o código-fonte e a documentação estão contidos na pasta raiz do projeto.

```
Maquina-Virtual-Chip-8/
├── src/            # Código-fonte (Chip8.cpp/h, componentes, main.cpp)
├── roms/           # Programas Chip-8 (.ch8) para teste
├── docs/           # Documentação de entrega (README_COMPILAR.pdf, README_USO.pdf)
└── CMakeLists.txt  # Arquivo de configuração da compilação
```

### Compilação

As instruções detalhadas de como compilar o projeto estão no arquivo `docs/README_COMPILAR.pdf`.

```bash
# Exemplo de comandos (utilizando CMake)
mkdir build
cd build
cmake ..
make
```

### Execução

O programa deve ser executado via linha de comando.

**Sintaxe Básica:**

```bash
./chip8_emulator [opções] caminho/para/a/rom.ch8
```

#### Opções de Linha de Comando

| Opção | Descrição | Padrão |
| :--- | :--- | :--- |
| `--clock <velocidade>` | Define a velocidade do clock da CPU em ciclos (instruções) por segundo (Hz). | `500Hz` |
| `--scale <fator>` | [cite\_start]Define o fator de escala (zoom) da janela[cite: 140]. Ex: um fator de 10 resulta em 640x320 pixels. | `10` |
| `--load-addr <endereço>` | [cite\_start]Endereço de memória onde o programa deve ser carregado (em hexadecimal)[cite: 142]. | `0x200` |
| `caminho/para/a/rom.ch8` | Caminho do arquivo do programa Chip-8 (ROM) a ser carregado. | **Obrigatório** |

**Exemplo:**

```bash
./chip8_emulator --clock 700 --scale 15 roms/pong.ch8
```