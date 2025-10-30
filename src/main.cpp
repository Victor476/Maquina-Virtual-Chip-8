#include <iostream>
#include <SDL3/SDL.h> 
#include <chrono>   
#include <thread>   
#include <algorithm>
#include <cstring>  
#include <iomanip>  // Para std::fixed, std::setprecision
#include "Chip8.h"    
#include "components/Display.h"

using namespace std::chrono;

// Constantes de Timing
constexpr int DEFAULT_CPU_HZ = 500;
constexpr int PERIPHERAL_HZ = 60;
constexpr auto US_PER_60HZ_CYCLE = duration_cast<microseconds>(seconds(1)) / PERIPHERAL_HZ;

// Constante para o Fator de Escala
constexpr uint32_t DEFAULT_SCALE = 10;
uint32_t scale_factor = DEFAULT_SCALE; // Variável global (ou estática) para armazenar o fator de escala

// Função para analisar argumentos e configurar o clock, escala e o caminho da ROM
uint32_t parse_args(int argc, char* argv[], const char** rom_path, uint32_t default_clock) {
    uint32_t clock_hz = default_clock;
    
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--clock") == 0 && i + 1 < argc) {
            try {
                clock_hz = std::stoul(argv[++i]);
                std::cout << "DEBUG: Clock configurado para " << clock_hz << " Hz." << std::endl;
            } catch (...) {
                std::cerr << "AVISO: Valor de --clock invalido. Usando padrao: " << default_clock << " Hz." << std::endl;
            }
        } 
        else if (strcmp(argv[i], "--scale") == 0 && i + 1 < argc) { 
            try {
                scale_factor = std::stoul(argv[++i]);
                std::cout << "DEBUG: Fator de escala configurado para " << scale_factor << "x. (Resolucao: " 
                          << CHIP8_WIDTH * scale_factor << "x" << CHIP8_HEIGHT * scale_factor << ")" << std::endl;
            } catch (...) {
                std::cerr << "AVISO: Valor de --scale invalido. Usando padrao: " << DEFAULT_SCALE << "x." << std::endl;
            }
        }
        else if (argv[i][0] != '-' || (argv[i][0] == '-' && argv[i][1] != '-')) {
            *rom_path = argv[i];
        }
    }
    return clock_hz;
}

int main(int argc, char* argv[]) {
    // --- 1. CONFIGURAÇÃO INICIAL E PARSE DE ARGUMENTOS ---
    const char* rom_path = nullptr;
    uint32_t clock_hz = parse_args(argc, argv, &rom_path, DEFAULT_CPU_HZ);

    if (!rom_path) {
        std::cerr << "ERRO: Forneca o caminho para o arquivo ROM (.ch8) como argumento." << std::endl;
        std::cerr << "Uso: ./chip8_emulator [--clock <hz>] [--scale <N>] <caminho/para/a/rom.ch8>" << std::endl;
        return 1;
    }

    // Inicializar SDL (VÍDEO E EVENTOS)
    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO) < 0) { 
        std::cerr << "ERRO SDL: Falha ao inicializar eventos e video: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    // --- 2. PREPARAÇÃO DA VM, GRÁFICOS E CARREGAMENTO ---
    Chip8 emulator(clock_hz); 
    emulator.load_rom(rom_path, 0x200); 

    // Inicializar o Display com as configurações de escala
    if (!emulator.init_display_graphics(scale_factor)) { 
        SDL_Quit();
        return 1;
    }

    // Variáveis para Validação (Issue 6 - Medição da Performance TOTAL)
    long long cycles_executed_total = 0;
    auto start_time = high_resolution_clock::now(); // Tempo em que o loop principal começa

    // Tempo alvo para CADA ciclo da CPU (1/N Hz)
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
                quit = true; // Seta a flag para sair do loop
            } else {
                emulator.process_input(event);
            }
        }

        // B. Ciclo da CPU (Fetch-Decode-Execute)
        emulator.cycle(); 
        cycles_executed_total++; // Conta o ciclo executado

        // C. Controle de Timing da CPU (Mantém a frequência em N Hz)
        auto cpu_end_time = high_resolution_clock::now();
        auto elapsed_cpu_time = duration_cast<microseconds>(cpu_end_time - cpu_start_time);
        
        if (elapsed_cpu_time < CPU_CYCLE_DURATION) {
            std::this_thread::sleep_for(CPU_CYCLE_DURATION - elapsed_cpu_time);
        }
        
        // D. Controle de Timing de Periféricos (60Hz)
        auto now = high_resolution_clock::now();
        auto elapsed_60hz_time = now - last_60hz_tick;

        if (elapsed_60hz_time >= US_PER_60HZ_CYCLE) {
            emulator.update_timers(); 
            emulator.render_display();
            last_60hz_tick = now;
        }
        
        // ** NOTA: Lógica de VALIDAÇÃO REMOVIDA daqui **
    }

    // --- 4. ENCERRAMENTO E VALIDAÇÃO FINAL ---
    emulator.destroy_display_graphics(); 
    
    // Calcula o tempo total de execução e a frequência média final
    auto end_time = high_resolution_clock::now();
    auto total_duration = duration_cast<seconds>(end_time - start_time);
    
    // Evita divisão por zero se a duração total for zero
    if (total_duration.count() > 0) {
        double average_actual_hz = (double)cycles_executed_total / total_duration.count();
        std::cout << "\n=================================================" << std::endl;
        std::cout << "VALIDACAO FINAL (ISSUE 6 - PERFORMANCE):" << std::endl;
        std::cout << "Tempo total de execucao: " << total_duration.count() << " segundos." << std::endl;
        std::cout << "Total de ciclos executados: " << cycles_executed_total << std::endl;
        std::cout << "Frequencia media da CPU: " << std::fixed << std::setprecision(2) 
                  << average_actual_hz << " Hz (Alvo: " << clock_hz << " Hz)." << std::endl;
        std::cout << "=================================================" << std::endl;
    }
    
    SDL_Quit();
    std::cout << "VM encerrada de forma limpa." << std::endl;
    return 0;
}