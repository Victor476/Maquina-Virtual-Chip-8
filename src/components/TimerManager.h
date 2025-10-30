#ifndef TIMERMANAGER_H
#define TIMERMANAGER_H

#include <cstdint>
#include <SDL3/SDL.h>

class TimerManager {
public:
    TimerManager();
    uint8_t get_delay_timer() const { return delay_timer; }
    void set_delay_timer(uint8_t value) { delay_timer = value; }
    uint8_t get_sound_timer() const { return sound_timer; }
    void set_sound_timer(uint8_t value) { sound_timer = value; }
    void start_sound();
    void stop_sound();
    void update_timers();
    bool init_audio(); 
    void destroy_audio();
    
    
private:
    uint8_t delay_timer; // Delay Timer (DT)
    uint8_t sound_timer; // Sound Timer (ST)
    SDL_AudioDeviceID audio_device_id;
    bool is_audio_playing;
    
};

#endif // TIMERMANAGER_H