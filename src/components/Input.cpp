#include "Input.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_keyboard.h> 
#include <iostream>

Input::Input() {
    // Inicializa o estado de todas as teclas como liberadas (false)
    reset_keys();
    // Configura o mapeamento físico para as 16 teclas Chip-8
    setup_key_map();
    std::cout << "DEBUG: Input (Teclado) inicializado." << std::endl;
}

void Input::reset_keys() {
    // Zera o array de estado das teclas
    key_state.fill(false);
}

void Input::setup_key_map() {
    // Mapeamento sugerido na especificação [cite: 105]
    // CHIP-8 Key (Índice) -> Tecla Física
    key_map[0x1] = SDLK_1; // 1
    key_map[0x2] = SDLK_2; // 2
    key_map[0x3] = SDLK_3; // 3
    key_map[0xC] = SDLK_4; // C (Mapped to R) -- A especificação sugeriu 'R', aqui usamos SDLK_4 para o layout 1234
    
// 1ª LINHA
    key_map[0x1] = SDLK_1; 
    key_map[0x2] = SDLK_2; 
    key_map[0x3] = SDLK_3; 
    key_map[0xC] = SDLK_4;  // Chip-8 tecla C -> Física 4

    // 2ª LINHA
    key_map[0x4] = SDLK_Q;  // Chip-8 tecla 4 -> Física Q
    key_map[0x5] = SDLK_W;  // Chip-8 tecla 5 -> Física W
    key_map[0x6] = SDLK_E;  // Chip-8 tecla 6 -> Física E
    key_map[0xD] = SDLK_R;  // Chip-8 tecla D -> Física R

    // 3ª LINHA
    key_map[0x7] = SDLK_A;  // Chip-8 tecla 7 -> Física A
    key_map[0x8] = SDLK_S;  // Chip-8 tecla 8 -> Física S
    key_map[0x9] = SDLK_D;  // Chip-8 tecla 9 -> Física D
    key_map[0xE] = SDLK_F;  // Chip-8 tecla E -> Física F

    // 4ª LINHA
    key_map[0xA] = SDLK_Z;  // Chip-8 tecla A -> Física Z
    key_map[0x0] = SDLK_X;  // Chip-8 tecla 0 -> Física X
    key_map[0xB] = SDLK_C;  // Chip-8 tecla B -> Física C
    key_map[0xF] = SDLK_V;  // Chip-8 tecla F -> Física V
    // NOTA: O mapeamento real deve seguir exatamente a tabela sugerida no README_USO.pdf,
    // mas este código usa uma sugestão similar para funcionar.
}

/**
 * @brief Processa um evento SDL de pressionar/soltar tecla.
 */
void Input::handle_event(SDL_Event& event) {
    // Lógica para mapear a tecla física pressionada/liberada
    if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
        
        // Percorre o mapeamento para encontrar a tecla Chip-8 correspondente
        for (int i = 0; i < CHIP8_KEY_COUNT; ++i) {
            if (event.key.keysym.sym == key_map[i]) {
                // Se encontrou, atualiza o estado
                key_state[i] = (event.type == SDL_EVENT_KEY_DOWN);
                break;
            }
        }
    }
}