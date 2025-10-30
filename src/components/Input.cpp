#include "Input.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_keyboard.h> 
#include <iostream>
#include <cstring> // Para std::memset (Embora você use fill)

Input::Input() {
    reset_keys();
    setup_key_map();
    std::cout << "DEBUG: Input (Teclado) inicializado." << std::endl;
}

void Input::reset_keys() {
    key_state.fill(false);
}

void Input::setup_key_map() {
    // 1ª LINHA: 1 2 3 C -> Teclas 1, 2, 3, 4
    key_map[0x1] = SDLK_1; 
    key_map[0x2] = SDLK_2; 
    key_map[0x3] = SDLK_3; 
    key_map[0xC] = SDLK_4;  // Chip-8 tecla C -> Física 4 (ou R, mas 4 é usado para layout compacto)

    // 2ª LINHA: 4 5 6 D -> Teclas Q, W, E, R
    key_map[0x4] = SDLK_Q;  // Chip-8 tecla 4 -> Física Q
    key_map[0x5] = SDLK_W;  // Chip-8 tecla 5 -> Física W
    key_map[0x6] = SDLK_E;  // Chip-8 tecla 6 -> Física E
    key_map[0xD] = SDLK_R;  // Chip-8 tecla D -> Física R

    // 3ª LINHA: 7 8 9 E -> Teclas A, S, D, F
    key_map[0x7] = SDLK_A;  // Chip-8 tecla 7 -> Física A
    key_map[0x8] = SDLK_S;  // Chip-8 tecla 8 -> Física S
    key_map[0x9] = SDLK_D;  // Chip-8 tecla 9 -> Física D
    key_map[0xE] = SDLK_F;  // Chip-8 tecla E -> Física F

    // 4ª LINHA: A 0 B F -> Teclas Z, X, C, V
    key_map[0xA] = SDLK_Z;  // Chip-8 tecla A -> Física Z
    key_map[0x0] = SDLK_X;  // Chip-8 tecla 0 -> Física X
    key_map[0xB] = SDLK_C;  // Chip-8 tecla B -> Física C
    key_map[0xF] = SDLK_V;  // Chip-8 tecla F -> Física V
}


void Input::handle_event(SDL_Event& event) {
    if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
        // Critério: Captura de Eventos e Matriz de Estado
        SDL_Keycode key_code = event.key.key;
        bool is_pressed = (event.type == SDL_EVENT_KEY_DOWN);
        
        // Percorre o mapeamento para encontrar a tecla Chip-8 correspondente
        for (int i = 0; i < CHIP8_KEY_COUNT; ++i) {
            if (key_code == key_map[i]) { 
                key_state[i] = is_pressed;
                
                // Critério de Validação: Adicionar Log
                std::cout << "LOG: Tecla Chip-8 0x" << std::hex << i 
                          << (is_pressed ? " PRESSIONADA" : " LIBERADA") 
                          << " (Fisica: " << SDL_GetKeyName(key_code) << ")" << std::endl;
                
                return; 
            }
        }
    }
}