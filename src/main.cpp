#include <iostream>
#include <SDL3/SDL.h> 
#include <chrono>   
#include <thread>   
#include <algorithm>
#include <cstring>  // Para strcmp
#include <iomanip>  // Para std::fixed, std::setprecision
#include "Chip8.h"   

using namespace std::chrono;

// Constantes de Timing
constexpr int DEFAULT_CPU_HZ = 500;
constexpr int PERIPHERAL_HZ = 60;
// Usamos microsegundos para evitar arredondamentos em milissegundos
constexpr auto US_PER_60HZ_CYCLE = duration_cast<microseconds>(seconds(1)) / PERIPHERAL_HZ;


uint32_t parse_args(int argc, char* argv[], const char** rom_path, uint32_t default_clock) {
    uint32_t clock_hz = default_clock;
    
    // Iteramos sobre os argumentos para encontrar flags e o caminho da ROM
    for (int i = 1; i < argc; ++i) {
        
        if (strcmp(argv[i], "--clock") == 0 && i + 1 < argc) {
            // Processa a flag --clock
            try {
                clock_hz = std::stoul(argv[++i]);
                std::cout << "DEBUG: Clock configurado para " << clock_hz << " Hz." << std::endl;
            } catch (...) {
                std::cerr << "AVISO: Valor de --clock invalido. Usando padrao: " << default_clock << " Hz." << std::endl;
            }
        } 
        
        // Se o argumento NAO COMECAR com '--', assume que é o caminho da ROM
        else if (argv[i][0] != '-' || (argv[i][0] == '-' && argv[i][1] != '-')) {
            // Define o caminho da ROM
            *rom_path = argv[i];
        }
        // Nota: A logica acima permite que o caminho da ROM seja o ultimo argumento, que e o que você precisa.
    }
    return clock_hz;
}

int main(int argc, char* argv[]) {
    // --- 1. CONFIGURAÇÃO INICIAL ---
    const char* rom_path = nullptr;
    uint32_t clock_hz = parse_args(argc, argv, &rom_path, DEFAULT_CPU_HZ);

    if (!rom_path) {
        std::cerr << "ERRO: Forneca o caminho para o arquivo ROM (.ch8) como argumento." << std::endl;
        std::cerr << "Uso: ./chip8_emulator [--clock <hz>] <caminho/para/a/rom.ch8>" << std::endl;
        return 1;
    }

    if (SDL_Init(SDL_INIT_EVENTS) < 0) {
        std::cerr << "ERRO SDL: Falha ao inicializar eventos: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    // --- 2. PREPARAÇÃO DA VM E VARIÁVEIS DE TIMING ---
    Chip8 emulator(clock_hz); 
    emulator.load_rom(rom_path); 

    // Variáveis para Validação (Issue 6)
    long long cycles_executed = 0;
    auto last_log_time = high_resolution_clock::now();
    
    // Tempo alvo para CADA ciclo da CPU (500Hz)
    const auto CPU_CYCLE_DURATION = duration_cast<microseconds>(seconds(1)) / clock_hz; 
    
    auto last_60hz_tick = high_resolution_clock::now();
    bool quit = false;

    std::cout << "Iniciando loop principal..." << std::endl;
    while (!quit) {
        auto cpu_start_time = high_resolution_clock::now();

        // A. Processar Input (SDL Events)
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                quit = true;
            } else {
                emulator.process_input(event);
            }
        }

        // B. Ciclo da CPU (Fetch-Decode-Execute)
        emulator.cycle(); 
        cycles_executed++; // Conta o ciclo executado

        // C. Controle de Timing da CPU (Mantém a frequência em N Hz)
        auto cpu_end_time = high_resolution_clock::now();
        auto elapsed_cpu_time = duration_cast<microseconds>(cpu_end_time - cpu_start_time);
        
        // Se o ciclo demorou menos que o esperado, esperamos o tempo restante.
        if (elapsed_cpu_time < CPU_CYCLE_DURATION) {
            std::this_thread::sleep_for(CPU_CYCLE_DURATION - elapsed_cpu_time);
        }
        
        // D. Controle de Timing de Periféricos (60Hz)
        auto now = high_resolution_clock::now();
        auto elapsed_60hz_time = now - last_60hz_tick;

        if (elapsed_60hz_time >= US_PER_60HZ_CYCLE) {
            // 1. Decremento dos Temporizadores (Critério 60Hz)
            emulator.update_timers(); 
            
            // 2. Renderização da Tela (Critério 60Hz)
            // emulator.display.render(); // A chamada de renderizacao ira aqui
            
            last_60hz_tick = now; // Reinicia o contador de 60Hz
        }
        
        // E. VALIDAÇÃO DA PERFORMANCE (Issue 6 - Log a cada segundo)
        auto log_duration = duration_cast<seconds>(now - last_log_time);
        if (log_duration >= seconds(1)) {
            // Calcula a frequência real (Hz)
            double actual_hz = (double)cycles_executed / log_duration.count();
            std::cout << "VALIDACAO: CPU rodando a " << std::fixed << std::setprecision(2) 
                      << actual_hz << " Hz (Alvo: " << clock_hz << " Hz)." << std::endl;
            
            // Reajusta contadores
            cycles_executed = 0;
            last_log_time = now;
        }
    }

    // --- 4. ENCERRAMENTO ---
    SDL_Quit();
    std::cout << "VM encerrada de forma limpa." << std::endl;
    return 0;
}