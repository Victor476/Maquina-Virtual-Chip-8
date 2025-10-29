#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint>
#include <array>
#include "components/TimerManager.h"
#include "components/Display.h" 
#include "components/Input.h" 

class Chip8 {
public:
    Chip8();
    void initialize();
    void load_rom(const char* filename, uint16_t load_address);
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
    
    // Funções auxiliares
    void load_font_set();
    void fetch_opcode();
    void execute_opcode();
};

#endif // CHIP8_H