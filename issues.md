
## 📋 Issues Essenciais para o Projeto Chip-8

### Fase 1: Configuração e Base

1.  [cite_start]**Configuração do Ambiente e Compilação:** Configurar o `CMakeLists.txt` para compilar o projeto em C++ e garantir o suporte multiplataforma (Linux/Windows)[cite: 147, 150].
2.  [cite_start]**Integração e Inicialização da SDL:** Configurar a SDL (Simple DirectMedia Layer) para gerenciar a janela e os eventos básicos[cite: 16, 132].
3.  [cite_start]**Implementação da Estrutura de Memória e Registradores:** Definir a estrutura de dados para 4KB de memória, 16 registradores V, registrador I, PC, SP, Stack, e os temporizadores DT/ST[cite: 22, 29, 34, 36, 39, 109].
4.  [cite_start]**Pré-carga de Sprites Hexadecimais:** Implementar a função para carregar os 16 sprites de dígitos na memória, começando em `0x000`[cite: 48, 49].
5.  [cite_start]**Carregamento da ROM:** Implementar a função para ler um arquivo `.ch8` e carregá-lo na memória, a partir do endereço padrão `0x200`[cite: 26, 131].

### Fase 2: O Núcleo da CPU (Ciclo de Execução)

6.  [cite_start]**Loop Principal e Controle de Frequência:** Implementar o *loop* principal que executa a CPU na frequência configurável via linha de comando (padrão 500Hz)[cite: 137, 139].
7.  [cite_start]**Fetch (Busca de Instrução):** Implementar a busca da instrução de 2 bytes na memória (lendo em **big-endian**) a partir do endereço do PC[cite: 115, 118].
8.  [cite_start]**Decodificação e Execução (Switch Case Central):** Criar o bloco `switch-case` central para decodificar e despachar as 36 instruções (opcodes) do Chip-8[cite: 116].

### Fase 3: Periféricos e I/O (Input/Output)

9.  [cite_start]**Implementação Gráfica (Display):** Implementar o buffer de 64x32 pixels e a lógica para renderizar este buffer na janela SDL, respeitando o fator de escala configurável[cite: 44, 140, 141].
10. [cite_start]**Atualização da Tela (60Hz):** Garantir que a tela seja atualizada a uma taxa fixa de 60Hz, separada da velocidade da CPU[cite: 143].
11. [cite_start]**Implementação do Teclado:** Configurar a entrada de eventos da SDL e implementar o mapeamento das 16 teclas hexadecimais (e.g., Q, W, E, R, etc.) para o teclado físico[cite: 97, 103, 105, 145].
12. [cite_start]**Controle e Decremento de Temporizadores:** Implementar a lógica que decrementa o **Delay Timer (DT)** e o **Sound Timer (ST)** a uma taxa de 60Hz[cite: 109, 144].
13. [cite_start]**Implementação de Áudio:** Configurar a SDL para reproduzir um som ("beep") simples (como uma onda quadrada) enquanto o **Sound Timer** for maior que zero[cite: 111, 146].

### Fase 4: Opcodes (As Instruções da CPU)

* *Observação: As instruções são vastas. Geralmente, elas são divididas em issues por categoria (Saltos, Aritméticas, Gráficas, I/O).*

14. [cite_start]**Implementação dos Opcodes de Fluxo (0x0nnn, 0x1nnn, 0x2nnn):** Lidar com saltos, chamadas de sub-rotina (usando a Stack e o SP), e retornos (`RET`)[cite: 116].
15. [cite_start]**Implementação dos Opcodes Aritméticos e Lógicos (0x8nnn):** Implementar operações como soma, subtração, XOR, e shifters, garantindo o uso correto do registrador **VF** para *carry* ou *borrow*[cite: 30].
16. [cite_start]**Implementação do Opcode de Desenho (`DXYN`):** Implementar a instrução de desenho de sprites, incluindo a lógica de XOR e a atualização da *flag* **VF** em caso de colisão de pixels[cite: 30, 45].
17. [cite_start]**Implementação dos Opcodes de Teclado (0xExnn):** Implementar as instruções de salto se tecla pressionada e espera por tecla pressionada[cite: 97].
18. [cite_start]**Implementação dos Opcodes de Temporizadores e Memória (0xFxnn):** Lidar com a leitura/escrita dos temporizadores DT/ST e o armazenamento de BCD ou múltiplos registradores na memória[cite: 110, 111].

### Fase 5: Entrega e Documentação

19. [cite_start]**Tratamento de Argumentos de Linha de Comando:** Implementar a lógica em `main.cpp` para tratar todas as opções (`--clock`, `--scale`, endereço de carga) e o tratamento de erros (ex: arquivo inexistente)[cite: 136, 142, 188, 190].
20. [cite_start]**Criação da Documentação (README\_COMPILAR.pdf):** Escrever as instruções de compilação, listando dependências e comandos[cite: 175, 176].
21. [cite_start]**Criação da Documentação (README\_USO.pdf):** Escrever as instruções de uso e documentar o mapeamento completo do teclado[cite: 104, 179].
22. [cite_start]**Testes Finais e Validação:** Garantir que o interpretador execute corretamente os programas de teste obrigatórios (splash screen, logo da IBM, Maze, Tank e Pong)[cite: 155].

Este plano de issues fornece um roteiro claro do que precisa ser feito. 