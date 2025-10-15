#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint>
#include <array>

// Incluir os novos módulos
#include "components/TimerManager.h"
#include "components/Display.h" 
// #include "components/Input.h" (Será adicionado depois)

class Chip8 {
public:
    Chip8();
    void initialize();
    void load_rom(const char* filename, uint16_t load_address = 0x200);
    void cycle();

private:
    // Core CPU State
    std::array<uint8_t, 4096> memory;   
    uint8_t V[16];                      
    uint16_t I;                          
    uint16_t PC;                         
    uint16_t stack[16];                  
    uint8_t SP;                          // Stack Pointer [cite: 45]
    uint16_t opcode;                    // Instrução atual

    // --- Componentes Isolados (Novos) ---
    TimerManager timers;    // Gerencia DT e ST (removeu delay_timer e sound_timer)
    Display display;        // Gerencia o buffer de 64x32 (removeu array de pixels)

    // Funções auxiliares
    void load_font_set();
    void fetch_opcode();
    void execute_opcode();
};

#endif // CHIP8_H