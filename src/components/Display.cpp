#include "Display.h"
#include <cstring> // Para std::memset
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>

// Construtor do Display (apenas limpa o buffer na inicialização)
Display::Display() : window(nullptr), renderer(nullptr), scale_factor(0) {
    clear_screen();
    std::cout << "DEBUG: Display 64x32 buffer inicializado." << std::endl;
}

void Display::clear_screen() {
    // Zera o buffer inteiro (monocromático)
    std::memset(pixel_buffer.data(), 0, CHIP8_PIXEL_COUNT);
}

// =====================================================================
// IMPLEMENTAÇÃO SDL (CRITÉRIO DE ACEITAÇÃO DA ISSUE 9)
// =====================================================================

bool Display::init_graphics(uint32_t scale) {
    scale_factor = scale;
    uint32_t width = CHIP8_WIDTH * scale;
    uint32_t height = CHIP8_HEIGHT * scale;

    // Criar a Janela
    window = SDL_CreateWindow(
        "Chip-8 Emulator",
        width, 
        height, 
        0 
    );
    if (!window) {
        std::cerr << "ERRO SDL: Janela nao pode ser criada: " << SDL_GetError() << std::endl;
        return false;
    }

    // Criar o Renderer
    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        std::cerr << "ERRO SDL: Renderer nao pode ser criado: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        return false;
    }
    
    // Define a cor de fundo inicial (preto)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    std::cout << "DEBUG: Janela SDL criada (" << width << "x" << height << ")." << std::endl;
    return true;
}

void Display::render() {
    if (!renderer) return;

    // 1. Limpar a tela física (preto)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // 2. Definir a cor do pixel (branco - monocromático)
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); 

    // 3. Varrer o buffer de pixels Chip-8
    for (int y = 0; y < CHIP8_HEIGHT; ++y) {
        for (int x = 0; x < CHIP8_WIDTH; ++x) {
            size_t index = x + y * CHIP8_WIDTH;
            
            if (pixel_buffer[index] == 1) { // Se o pixel estiver 'ligado'
                // Desenha o retângulo escalonado (Critério Fator de Escala)
                SDL_FRect rect = {
                (float)(x * scale_factor), // <-- CASTING explícito para float
                (float)(y * scale_factor), // <-- CASTING explícito para float
                (float)scale_factor, 
                (float)scale_factor
            };
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }

    // 4. Apresentar o que foi desenhado na tela
    SDL_RenderPresent(renderer);
}

void Display::destroy_graphics() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    std::cout << "DEBUG: Janela SDL destruida." << std::endl;
}