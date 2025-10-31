

# 🎮 Guia de Uso da Máquina Virtual Chip-8

Este documento explica como executar o emulador Chip-8 (`chip8_emulator`) e detalha o mapeamento do teclado hexadecimal (0-F) para que você possa interagir com os programas e jogos (ROMs) da plataforma.

## 1. Sintaxe de Execução (CLI)

O emulador é executado através da linha de comando (CLI) e exige o caminho do arquivo ROM como argumento obrigatório.

A sintaxe básica é:

```bash
./build/chip8_emulator [OPÇÕES] <caminho/para/a/rom.ch8>
````

### Opções de Linha de Comando

| Opção | Descrição | Valor Padrão |
| :--- | :--- | :--- |
| `--clock <Hz>` | [cite\_start]Define a frequência de execução da CPU (ciclos por segundo)[cite: 137, 139]. | 500 Hz |
| `--scale <fator>` | [cite\_start]Define o fator de escala (zoom) da janela[cite: 140]. [cite\_start]Um fator de 10 resulta em uma janela de 640x320 pixels[cite: 141]. | 10 |
| `<caminho/rom.ch8>` | [cite\_start]O caminho absoluto ou relativo para o arquivo ROM do Chip-8[cite: 131]. | (Obrigatório) |

**Exemplo de Execução (Modo Rápido com Zoom):**

```bash
./build/chip8_emulator --clock 750 --scale 15 ../roms/PONG
```

## 2\. Mapeamento do Teclado Hexadecimal

[cite\_start]O Chip-8 usa um teclado de 16 teclas (0 a F)[cite: 97]. Você deve usar as teclas do teclado físico conforme a tabela abaixo para interagir com o emulador.

[cite\_start]O mapeamento segue o padrão QWERTY para as linhas centrais (Q-W-E-R, A-S-D-F, Z-X-C-V) e os números 1-4 para a linha superior, conforme o esquema sugerido na especificação[cite: 105].

| Teclado CHIP-8 (Hex) | Tecla Física | Função Típica (Ex: Pong) |
| :---: | :---: | :--- |
| **1** | **1** | Jogador 1: Mover para Cima |
| **2** | **2** | - |
| **3** | **3** | - |
| **C** | **4** | Jogador 2: Mover para Cima |
| **4** | **Q** | Jogador 1: Mover para Baixo |
| **5** | **W** | - |
| **6** | **E** | - |
| **D** | **R** | Jogador 2: Mover para Baixo |
| **7** | **A** | - |
| **8** | **S** | - |
| **9** | **D** | - |
| **E** | **F** | - |
| **A** | **Z** | - |
| **0** | **X** | - |
| **B** | **C** | - |
| **F** | **V** | - |

