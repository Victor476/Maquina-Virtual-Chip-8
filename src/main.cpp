#include <iostream>
#include <SDL3/SDL.h> // Necessário para a função main()
#include "Chip8.h"    // Inclui a definição da sua Máquina Virtual

int main(int argc, char* argv[]) {
    // 1. Lidar com a inicialização da SDL (Temporariamente Desativado)
    // Devido à Issue de Configuração, pulamos a inicialização completa da SDL,
    // mas o código final iria aqui.

    std::cout << "Iniciando Maquina Virtual Chip-8..." << std::endl;

    // 2. Instanciar a VM (Isso chama Chip8::Chip8() que chama initialize())
    Chip8 emulator;

    // 3. Loop principal (Temporariamente simples)
    bool quit = false;
    
    // Assumindo que você comentou a SDL no CMake, não podemos processar eventos.
    // O loop final teria uma checagem de evento SDL aqui.
    while (!quit) {
        // O ciclo da CPU e a atualização da tela viriam aqui

        // Apenas para demonstração e evitar loop infinito:
        quit = true; 
    }

    std::cout << "VM encerrada com sucesso. Verifique o console para a mensagem de inicialização." << std::endl;

    return 0;
}