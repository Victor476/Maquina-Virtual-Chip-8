#ifndef INPUT_H
#define INPUT_H

#include <cstdint>
#include <array>
#include <SDL3/SDL_keyboard.h> 

// O teclado Chip-8 tem 16 teclas (0 a F)
constexpr int CHIP8_KEY_COUNT = 16;

class Input {
public:
    Input();

    // Array que armazena o estado de cada tecla (true se pressionada, false se liberada)
    std::array<bool, CHIP8_KEY_COUNT> key_state; 

    // Mapeamento sugerido na especificação (teclas físicas para índices 0-F)
    // O índice corresponde à tecla Chip-8 (0x0 a 0xF)
    std::array<SDL_Keycode, CHIP8_KEY_COUNT> key_map; 

    // Métodos para o loop principal e opcodes
    void handle_event(SDL_Event& event);
    void reset_keys();

private:
    void setup_key_map();
};

#endif // INPUT_H