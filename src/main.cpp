#include <iostream>

// Inclua a SDL para usar a macro SDL_main
#include <SDL3/SDL.h>

// A SDL remapeia 'main' para SDL_main para compatibilidade multiplataforma.
// Use 'int main(int argc, char* argv[])' normalmente, mas o linker 
// agora será capaz de encontrar o ponto de entrada correto.

int main(int argc, char* argv[]) {
    std::cout << "Iniciando Maquina Virtual Chip-8..." << std::endl;
    // O código de inicialização do SDL (SDL_Init) irá aqui eventualmente.
    return 0;
}