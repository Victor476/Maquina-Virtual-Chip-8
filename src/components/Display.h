#ifndef DISPLAY_H
#define DISPLAY_H

#include <cstdint>
#include <array>
#include <SDL3/SDL.h> 

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

    // --- NOVOS MÉTODOS PÚBLICOS PARA GERENCIAMENTO DE GRÁFICOS ---
    bool init_graphics(uint32_t scale); // Inicializa SDL Window/Renderer e salva o fator de escala
    void render();                      // Desenha o pixel_buffer no Renderer
    void destroy_graphics();            // Destrói Window/Renderer

private:
    // --- MEMBROS PRIVADOS SDL ---
    SDL_Window* window;
    SDL_Renderer* renderer;
    uint32_t scale_factor; // Fator de zoom (e.g., 10x)
};

#endif // DISPLAY_H