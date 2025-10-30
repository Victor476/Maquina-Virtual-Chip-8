#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint>
#include <array>
#include "components/TimerManager.h"
#include "components/Display.h" 
#include "components/Input.h" 

class Chip8 {
public:
    // 1. Construtor Padrão (necessário se definirmos outro construtor)
    Chip8();
    
    // 2. Construtor de Frequência (OBRIGATÓRIO para main.cpp funcionar)
    Chip8(uint32_t frequency); 
    
    // 3. Funções de I/O que criamos para acessar componentes privados:
    void process_input(SDL_Event& event);
    void update_timers();
    

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
    uint8_t SP;                          
    uint16_t opcode;                    
    TimerManager timers;    
    Display display;        
    Input input;
    uint32_t cpu_frequency_hz;
    // Funções auxiliares
    void load_font_set();
    uint16_t fetch_opcode(); // <-- Ajuste de retorno
    void execute_opcode(uint16_t opcode);
    
};

#endif // CHIP8_H