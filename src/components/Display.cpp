#include "Display.h"
#include <cstring> // Para std::memset
#include <iostream>

Display::Display() {
    // Chama a função de limpeza na inicialização
    clear_screen();
    std::cout << "DEBUG: Display " << CHIP8_WIDTH << "x" << CHIP8_HEIGHT << " inicializado." << std::endl;
}

/**
 * @brief Limpa a tela (Opcode 00E0). Zera todos os pixels no buffer.
 */
void Display::clear_screen() {
    // Zera o buffer inteiro, desligando todos os pixels.
    // Usamos o tamanho total (64 * 32 = 2048 bytes)
    std::memset(pixel_buffer.data(), 0, CHIP8_PIXEL_COUNT);
}

// Futuras funções de renderização SDL virão aqui.