#include "Chip8.h"
#include <cstring>  // Para std::memset e std::memcpy
#include <iostream> // Para std::cout
#include <iomanip>  // Para formatação de saída (Validação)
#include "components/TimerManager.h"
#include "components/Display.h" 
#include "components/Input.h"
#include <fstream>  // Para std::ifstream e manipulação de arquivos
#include <vector>   // Para std::vector (usado no buffer)
#include <algorithm> // Para std::copy

// Dados dos sprites dos dígitos hexadecimais (0-F). 
// 16 sprites * 5 bytes/sprite = 80 bytes totais.
const uint8_t CHIP8_FONTSET[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8::Chip8() {
    initialize();
}

void Chip8::initialize() {
    // 1. Zera componentes internos da CPU
    std::memset(memory.data(), 0, memory.size()); 
    std::memset(V, 0, sizeof(V));               
    std::memset(stack, 0, sizeof(stack));       

    // Zera registradores e opcode
    I = 0;
    SP = 0;
    opcode = 0;

    // 2. Inicializa/Zera componentes modulares
    // ATUALIZAÇÃO CRUCIAL: Usamos os setters do TimerManager
    timers.set_delay_timer(0); 
    timers.set_sound_timer(0); 
    display.clear_screen();    // Usa o método da classe Display
    input.reset_keys();        // Usa o método da classe Input

    // 3. Configura o Program Counter (PC)
    PC = 0x200; 

    // 4. Pré-carrega o Font Set na Memória
    std::memcpy(memory.data(), CHIP8_FONTSET, sizeof(CHIP8_FONTSET));

    // --- Validação (Teste Unitário/Print) ---
    std::cout << "--- Chip-8 VM Inicializada ---" << std::endl;
    // O valor do DT e ST sera lido do TimerManager
    std::cout << "PC: 0x" << std::hex << std::setw(4) << std::setfill('0') << PC << " (Esperado: 0x0200)" << std::endl;
    std::cout << "I: 0x" << (int)I << ", SP: " << (int)SP << ", DT: " << (int)timers.get_delay_timer() << ", ST: " << (int)timers.get_sound_timer() << " (Esperado: 0)" << std::endl;
    
    // Validação da Pré-carga (Inspeção)
    std::cout << "Memória[0x000]: 0x" << std::hex << (int)memory[0x000] << " (Esperado: 0xF0)" << std::endl;
    std::cout << "Memória[0x050]: 0x" << std::hex << (int)memory[0x050] << " (Esperado: 0x00)" << std::endl;
}

// Endereço máximo da memória disponível para o programa.
const uint16_t MAX_ROM_SIZE = 0xFFF - 0x200; 

void Chip8::load_rom(const char* filename, uint16_t load_address) {
    // 1. Abrir o arquivo binário e determinar o tamanho
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        std::cerr << "ERRO FATAL: Nao foi possivel abrir o arquivo ROM: " << filename << std::endl;
        exit(1); 
    }

    std::streampos size = file.tellg();
    file.seekg(0, std::ios::beg); // Volta ao início

    // 2. Verificação de Tamanho
    if (size > MAX_ROM_SIZE) {
        std::cerr << "ERRO FATAL: O arquivo ROM (" << size << " bytes) e muito grande." << std::endl;
        exit(1);
    }
    
    // 3. Ler o Conteúdo Binário
    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) {
        std::cerr << "ERRO FATAL: Falha ao ler o conteudo do arquivo ROM: " << filename << std::endl;
        exit(1);
    }

    // 4. Copiar para a Memória (Critério Principal)
    uint16_t start_addr = load_address;
    
    // Copia o buffer de char lido para o array de uint8_t da memoria
    std::copy(
        buffer.begin(), 
        buffer.end(), 
        memory.begin() + start_addr
    );

    std::cout << "ROM '" << filename << "' carregada com sucesso!" << std::endl;
    std::cout << "Tamanho: " << size << " bytes. Endereco de Carga: 0x" << std::hex << start_addr << std::endl;

    // --- Validação (Teste de Integridade) ---
    // Você usará esta parte para validar o teste de ROM
    uint16_t primeiro_byte = load_address;
    uint16_t ultimo_byte = load_address + size - 1;
    std::cout << "Primeiro byte ROM (0x" << std::hex << primeiro_byte << "): 0x" 
              << std::setw(2) << std::setfill('0') << (int)memory[primeiro_byte] << std::endl;
    std::cout << "Ultimo byte ROM (0x" << ultimo_byte << "): 0x" 
              << std::setw(2) << std::setfill('0') << (int)memory[ultimo_byte] << std::endl;
    std::cout << "Byte apos o final (0x" << ultimo_byte + 1 << "): 0x" 
              << std::setw(2) << std::setfill('0') << (int)memory[ultimo_byte + 1] << std::endl;
}
