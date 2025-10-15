#ifndef DISPLAY_H
#define DISPLAY_H

#include <cstdint>
#include <array>

// Resolução padrão do Chip-8
constexpr int CHIP8_WIDTH = 64;
constexpr int CHIP8_HEIGHT = 32;
constexpr int CHIP8_PIXEL_COUNT = CHIP8_WIDTH * CHIP8_HEIGHT; // 2048

class Display {
public:
    Display();

    // Ação principal da Opcode 00E0: Limpar a tela
    void clear_screen();
    
    // O buffer de pixels que a CPU manipulará. 
    // Usamos uint8_t para representar cada pixel (0 ou 1)
    std::array<uint8_t, CHIP8_PIXEL_COUNT> pixel_buffer;

    // TODO: Adicionar lógica SDL (janela e renderizador) aqui.

private:
    // Futuras variáveis de SDL, se necessário.
};

#endif // DISPLAY_H