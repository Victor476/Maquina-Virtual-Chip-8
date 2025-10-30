#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint>
#include <array>
#include "components/TimerManager.h"
#include "components/Display.h" 
#include "components/Input.h" 

class Chip8 {
public:
    Chip8(uint32_t frequency); 
    void process_input(SDL_Event& event);
    void update_timers();
    void initialize();
    void load_rom(const char* filename, uint16_t load_address = 0x200);
    void cycle();
    bool init_display_graphics(uint32_t scale); // Wrapper para display.init_graphics
    void render_display();                       // Wrapper para display.render
    void destroy_display_graphics();
    void set_key_pressed(uint8_t key_value);
    void load_font_set();
    void execute_opcode(uint16_t opcode);
    uint16_t fetch_opcode();
    bool is_waiting_for_key() { return m_is_waiting_for_key; }




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
    bool m_is_waiting_for_key;
    uint8_t key_register_to_load;
    
};

#endif // CHIP8_H