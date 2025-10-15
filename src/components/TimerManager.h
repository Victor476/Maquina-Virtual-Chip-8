#ifndef TIMERMANAGER_H
#define TIMERMANAGER_H

#include <cstdint>

class TimerManager {
public:
    TimerManager();

    // Métodos para a CPU ler e escrever nos temporizadores (requisito de opcode)
    uint8_t get_delay_timer() const { return delay_timer; }
    void set_delay_timer(uint8_t value) { delay_timer = value; }
    uint8_t get_sound_timer() const { return sound_timer; }
    void set_sound_timer(uint8_t value) { sound_timer = value; }

    // Função que será chamada a 60Hz pelo loop principal
    void update_timers();
    
private:
    uint8_t delay_timer; // Delay Timer (DT)
    uint8_t sound_timer; // Sound Timer (ST)
};

#endif // TIMERMANAGER_H