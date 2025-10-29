#include <iostream>
#include <SDL3/SDL.h> 
#include "Chip8.h"    

int main(int argc, char* argv[]) {
    // Critério: Implementar uso via linha de comando
    if (argc < 2) {
        std::cerr << "ERRO: Forneca o caminho para o arquivo ROM (.ch8) como argumento." << std::endl;
        std::cerr << "Uso: ./chip8_emulator <caminho/para/a/rom.ch8>" << std::endl;
        return 1;
    }

    // O primeiro argumento (argv[1]) é o caminho para o arquivo ROM
    const char* rom_path = argv[1]; 

    Chip8 emulator;
    
    // Chamada à função de carregamento
    emulator.load_rom(rom_path,0x200); 

    // O loop principal (Fetch-Decode-Execute) viria aqui

    std::cout << "VM encerrada com sucesso após carregar ROM." << std::endl;

    return 0;
}