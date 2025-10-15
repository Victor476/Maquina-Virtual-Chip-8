#include "TimerManager.h"
#include <iostream>

TimerManager::TimerManager() 
    : delay_timer(0), sound_timer(0) {}

/**
 * @brief Decrementa o Delay Timer e o Sound Timer, se seus valores forem > 0.
 * Esta função deve ser chamada 60 vezes por segundo (60Hz) pelo loop principal.
 */
void TimerManager::update_timers() {
    // Delay Timer (DT)
    if (delay_timer > 0) {
        delay_timer--; // Decrementa a 60Hz
    }

    // Sound Timer (ST)
    if (sound_timer > 0) {
        sound_timer--; // Decrementa a 60Hz
        
        // Lógica do som: produz um 'beep' enquanto ST > 0 [cite: 121-122].
        if (sound_timer == 0) {
            std::cout << "DEBUG: Som parou (Sound Timer = 0)." << std::endl; 
            // TODO: Integrar a lógica SDL para parar o áudio real aqui.
        } else if (sound_timer == 1) {
            // Emula o início do beep se estiver ativo.
            std::cout << "DEBUG: Produzindo som ('beep')." << std::endl;
            // TODO: Integrar a lógica SDL para iniciar o áudio real aqui.
        }
    }
}