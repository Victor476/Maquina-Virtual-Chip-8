# 🛠️ Instruções de Compilação da Máquina Virtual Chip-8

Este documento detalha o processo de compilação do interpretador Chip-8 utilizando o sistema de build **CMake** e a biblioteca **SDL3**.

## 1. Requisitos e Dependências Necessárias

Para compilar o projeto em qualquer plataforma, você deve ter instalados:

**Compilador C++:** GCC/g++ (versão 9+) ou Clang/clang++ (O projeto usa o padrão C++17).
* **Sistema de Build:** CMake (versão 3.10 ou superior).
* **Biblioteca Gráfica/I/O:** **SDL3** (Simple DirectMedia Layer) - A versão de código-fonte está incluída no diretório `vendor/`.

## 2. Preparação dos Arquivos (Integração SDL3)

**IMPORTANTE:** Para ambientes restritos (como o WSL sem permissões de `sudo`), o projeto está configurado para **compilar a SDL3 e SDL3\_main a partir do código-fonte** incluído no diretório `vendor/`. Nenhuma instalação de dependências de sistema (`libsdl2-dev`) é necessária no ambiente host para a SDL, mas pode ser necessária para o áudio.

## 3. Processo de Compilação

Siga estes passos no terminal (Linux/WSL) para configurar o projeto e gerar o executável `chip8_emulator`.

### Passo A: Navegar e Limpar (Estado de Fábrica)

Navegue para o diretório raiz do projeto (`Maquina-Virtual-Chip-8/`). É recomendável usar o target `rebuild` se ele foi adicionado ao seu `CMakeLists.txt`:

```bash

# Navegue até o diretório raiz do projeto

cd Maquina-Virtual-Chip-8/ 

# Remove o build anterior (se existir) e recria a pasta

rm -rf build 

mkdir build

cd build

### Passo B: Configuração do CMake

O CMake deve ser configurado com o tipo de build Release para otimizar o desempenho (500Hz).

```bash
# Configura o projeto, verificando dependências e SDL3

cmake -DCMAKE_BUILD_TYPE=Release ..

```

### Passo C: Compilação e Geração do Executável

Utilize make com a flag -j para acelerar a compilação (usando múltiplos núcleos da CPU).

```bash

# Compila e gera o executável (chip8_emulator)

make -j

```



## 4. Localização do Executável

O executável compilado (chip8_emulator) estará no diretório build/.

