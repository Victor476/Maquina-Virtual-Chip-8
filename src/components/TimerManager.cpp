#include "TimerManager.h"
#include <SDL3/SDL.h>
#include <iostream>
#include <cmath> // Para sin() e M_PI
#include <cstring> // Para memcpy, se necessário

// --- Constantes para Geração de Áudio ---
const int BEEF_FREQUENCY = 440; // Frequência do beep (A4)
const int SAMPLE_RATE = 44100;  // Taxa de amostragem padrão
static double audio_phase = 0.0; // Fase global para continuidade da onda
SDL_AudioSpec desired_spec;


// =====================================================================
// CRITÉRIO: FUNÇÃO DE CALLBACK PARA GERAR A ONDA SONORA (Deve estar no topo)
// =====================================================================
void AudioCallback(void *userdata, Uint8 *stream, int len) {

    // 'len' é o tamanho do buffer em bytes. Convertemos para floats.
    float *audio_stream = (float *)stream;
    int num_samples = len / sizeof(float);
    
    // CORREÇÃO: Removida a declaração duplicada 'float *audio_stream'
    // A declaração dentro do for é desnecessária.

    for (int i = 0; i < num_samples; ++i) {
        // Gera uma onda senoidal (tom puro)
        float value = sin(audio_phase * 2.0 * M_PI);
        
        // Escreve o sample no buffer (volume baixo para evitar clipping)
        audio_stream[i] = value * 0.2f; 
        
        // Atualiza a fase: avança baseado na frequência do beep
        audio_phase += (double)BEEF_FREQUENCY / SAMPLE_RATE;
        if (audio_phase >= 1.0) {
            audio_phase -= 1.0;
        }
    }
}

// =====================================================================
// GERENCIAMENTO DA CLASSE (TimerManager)
// =====================================================================

// Construtor (apenas inicializa membros)
TimerManager::TimerManager() 
    : delay_timer(0), sound_timer(0), 
      is_audio_playing(false),
      audio_device_id(0)
{}

// Implementação dos métodos de áudio (agora o AudioCallback está definido)
// src/components/TimerManager.cpp (Dentro de TimerManager::init_audio())

bool TimerManager::init_audio() {
    SDL_AudioSpec desired_spec;
    
    // Configuração do formato de áudio (APENAS MEMBROS VÁLIDOS NA SDL3)
    desired_spec.freq = SAMPLE_RATE;
    desired_spec.format = SDL_AUDIO_F32; 
    desired_spec.channels = 1;          
    audio_device_id = SDL_OpenAudioDevice(
        0,       // Não captura áudio
        &desired_spec
    );
    
    if (audio_device_id == 0) {
        std::cerr << "ERRO SDL Audio: Falha ao abrir dispositivo de audio: " << SDL_GetError() << std::endl;
        return false;
    }

    // Pausa o áudio inicialmente (espera ST > 0)
    SDL_PauseAudioDevice(audio_device_id); 
    is_audio_playing = false;

    std::cout << "DEBUG: Subsistema de Audio inicializado." << std::endl;
    return true;
}

void TimerManager::destroy_audio() {
    if (audio_device_id != 0) {
        SDL_CloseAudioDevice(audio_device_id);
        audio_device_id = 0;
    }
}

void TimerManager::start_sound() {
    if (!is_audio_playing) {
        SDL_ResumeAudioDevice(audio_device_id); 
        is_audio_playing = true;
        std::cout << "DEBUG: Som REAL iniciado (ST > 0)." << std::endl;
    }
}

void TimerManager::stop_sound() {
    if (is_audio_playing) {
        SDL_PauseAudioDevice(audio_device_id); 
        is_audio_playing = false;
        std::cout << "DEBUG: Som REAL parado (ST = 0)." << std::endl;
    }
}

void TimerManager::update_timers() {
    if (delay_timer > 0) {
        delay_timer--;
    }

    if (sound_timer > 0) {
        sound_timer--;
        if (sound_timer > 0) {
            start_sound();
        } else {
            stop_sound();
        }
    } else {
        stop_sound();
    }
}