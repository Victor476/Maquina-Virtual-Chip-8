#include "Chip8.h"
#include <cstring>  // Para std::memset e std::memcpy
#include <iostream> // Para std::cout
#include <iomanip>  // Para formatação de saída (Validação)
#include <fstream>  // Para std::ifstream e manipulação de arquivos
#include <vector>   // Para std::vector (usado no buffer)
#include <algorithm> // Para std::copy

// Dados dos sprites dos dígitos hexadecimais (0-F). 
// (Mantido, assumindo que as declarações no Chip8.h estão corretas)
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

// =====================================================================
// CONSTRUTOR / INICIALIZAÇÃO / LOAD ROM
// (Assumindo que esta lógica está correta e funcional)
// =====================================================================

Chip8::Chip8(uint32_t frequency) 
    : memory{}, V{}, I(0), PC(0x200), stack{}, SP(0), opcode(0), timers{}, display{}, input{}, cpu_frequency_hz(frequency) 
{
    initialize(); 
}

Chip8::Chip8() : Chip8(500) {} 

void Chip8::initialize() {
    std::memset(memory.data(), 0, memory.size()); 
    std::memset(V, 0, sizeof(V));               
    std::memset(stack, 0, sizeof(stack));       
    I = 0; SP = 0; opcode = 0; PC = 0x200; 

    timers.set_delay_timer(0); 
    timers.set_sound_timer(0); 
    display.clear_screen();    
    input.reset_keys();        
    std::memcpy(memory.data(), CHIP8_FONTSET, sizeof(CHIP8_FONTSET));

    std::cout << "--- Chip-8 VM Inicializada ---" << std::endl;
    std::cout << "PC: 0x" << std::hex << std::setw(4) << std::setfill('0') << PC << " (Esperado: 0x0200)" << std::endl;
    std::cout << "I: 0x" << (int)I << ", SP: " << (int)SP << ", DT: " << (int)timers.get_delay_timer() << ", ST: " << (int)timers.get_sound_timer() << " (Esperado: 0)" << std::endl;
    std::cout << "Memória[0x000]: 0x" << std::hex << (int)memory[0x000] << " (Esperado: 0xF0)" << std::endl;
    std::cout << "Memória[0x050]: 0x" << std::hex << (int)memory[0x050] << " (Esperado: 0x00)" << std::endl;
}

const uint16_t MAX_ROM_SIZE = 0xFFF - 0x200; 

void Chip8::load_rom(const char* filename, uint16_t load_address) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) { /* Error handling */ std::cerr << "ERRO FATAL: Nao foi possivel abrir o arquivo ROM: " << filename << std::endl; exit(1); }
    std::streampos size = file.tellg();
    file.seekg(0, std::ios::beg);
    if (size > MAX_ROM_SIZE) { /* Error handling */ std::cerr << "ERRO FATAL: O arquivo ROM (" << size << " bytes) e muito grande." << std::endl; exit(1); }
    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) { /* Error handling */ std::cerr << "ERRO FATAL: Falha ao ler o conteudo do arquivo ROM: " << filename << std::endl; exit(1); }
    uint16_t start_addr = load_address;
    std::copy(buffer.begin(), buffer.end(), memory.begin() + start_addr);

    std::cout << "ROM '" << filename << "' carregada com sucesso!" << std::endl;
    std::cout << "Tamanho: " << size << " bytes. Endereco de Carga: 0x" << std::hex << start_addr << std::endl;

    // Validation prints removed for brevity (assuming they are there)
}

bool Chip8::init_display_graphics(uint32_t scale) {
    return display.init_graphics(scale);
}

void Chip8::render_display() {
    display.render();
}

void Chip8::destroy_display_graphics() {
    display.destroy_graphics();
}

void Chip8::process_input(SDL_Event& event) 
{ 
    input.handle_event(event); 
}

void Chip8::update_timers() 
{ 
    timers.update_timers(); 
}

uint16_t Chip8::fetch_opcode() {
    uint16_t msb = memory[PC]; 
    uint16_t lsb = memory[PC + 1];
    opcode = (msb << 8) | lsb;
    PC += 2; 
    return opcode;
}

void Chip8::cycle() {
    uint16_t current_opcode = fetch_opcode();
    execute_opcode(current_opcode); 
    // DEBUG LOG MANTIDO:
    std::cout << "DEBUG: PC=0x" << std::hex << PC - 2 << ", Opcode Buscado: 0x" << current_opcode << std::endl;
}

// =====================================================================
// EXECUÇÃO DO OPCODE (Issue 8/9: Fluxo, Atribuição, Timers)
// =====================================================================

void Chip8::execute_opcode(uint16_t opcode) {
    uint16_t nnn = opcode & 0x0FFF;
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t nn = opcode & 0x00FF;
    uint8_t n = opcode & 0x000F;

    switch (opcode & 0xF000) {
        
        case 0x0000: // 0nnn - Chamadas de Máquina / Controle de Fluxo
            switch (nn) {
                case 0x00E0: display.clear_screen(); std::cout << "DEBUG: Opcode 00E0: CLS - Tela limpa." << std::endl; break;
                case 0x00EE: /* RET */
                    if (SP == 0) { std::cerr << "ERRO FATAL: Tentativa de RET de uma stack vazia." << std::endl; exit(1); }
                    PC = stack[--SP]; // Stack Pop: Decrementa e depois usa
                    std::cout << "DEBUG: Opcode 00EE: RET - Retorno para 0x" << std::hex << PC << std::endl; break;
                default: std::cerr << "AVISO: Opcode 0NNN (Chamada de maquina) ignorado: 0x" << std::hex << opcode << std::endl;
            }
            break;

        case 0x1000: PC = nnn; std::cout << "DEBUG: Opcode 1NNN: JP (Jump) para 0x" << std::hex << nnn << std::endl; break;

        case 0x2000: // 2nnn: CALL addr
            if (SP >= 16) { std::cerr << "ERRO FATAL: Stack Overflow (limite 16)." << std::endl; exit(1); }
            stack[SP++] = PC; // Stack Push: Salva PC e depois incrementa
            PC = nnn;
            std::cout << "DEBUG: Opcode 2NNN: CALL (Chama sub-rotina) para 0x" << std::hex << nnn << std::endl; break;
            
        // =========================================================================
        // Opcodes IMPLEMENTADOS PARA QUEBRAR O LOOP: 6xnn, 7xnn, 3xnn
        // =========================================================================
        case 0x3000: // 3xnn: SE Vx, byte (Skip if Equal) - IMPLEMENTADO
            if (V[x] == nn) {
                PC += 2; // Pula a próxima instrução
                std::cout << "DEBUG: Opcode 3XNN: SE - Salto APROVADO. PC=0x" << std::hex << PC << std::endl;
            } else {
                std::cout << "DEBUG: Opcode 3XNN: SE - Salto REJEITADO." << std::endl;
            }
            break;
            
        case 0x4000: /* 4xnn: SNE Vx, byte */ break;
        case 0x5000: /* 5xy0: SE Vx, Vy */ break;

        case 0x6000: // 6xnn: LD Vx, byte (Load) - IMPLEMENTADO
            V[x] = nn;
            std::cout << "DEBUG: Opcode 6XNN: LD V" << (int)x << ", byte. V" << (int)x << " = 0x" << std::hex << (int)nn << std::endl;
            break;

        case 0x7000: // 7xnn: ADD Vx, byte (Adição) - IMPLEMENTADO
            V[x] += nn; // Adiciona (overflow é permitido para uint8_t)
            std::cout << "DEBUG: Opcode 7XNN: ADD V" << (int)x << ", byte. V" << (int)x << " += 0x" << std::hex << (int)nn << std::endl;
            break;

        case 0x9000: /* 9xy0: SNE Vx, Vy */ break;
        case 0xA000: /* Annn: LD I, addr */ break;
        case 0xB000: /* Bnnn: JP V0, addr */ break;
        case 0xC000: /* Cxnn: RND Vx, byte */ break;

        case 0x8000: /* 8xyn - Aritméticas e Lógicas (Skeleton) */ break;
        case 0xD000: /* Dxyn: DRW Vx, Vy, nibble (Desenha Sprite) */ break;
            
        case 0xE000: /* Exnn - Teclado (Skeleton) */ break;
        
        // =========================================================================
        case 0xF000: // Fxnn - Timers e Memória
        // =========================================================================
            switch (nn) {
                case 0x0007: // Fx07: LD Vx, DT (Load Delay Timer) - IMPLEMENTADO
                    V[x] = timers.get_delay_timer();
                    std::cout << "DEBUG: Opcode FX07: LD V" << (int)x << ", DT. V" << (int)x << "=" << (int)V[x] << std::endl;
                    break;
                case 0x000A: /* Fx0A: LD Vx, K */ break;
                case 0x0015: /* Fx15: LD DT, Vx */ break;
                case 0x001E: /* Fx1E: ADD I, Vx */ break;
                // ... (outros Fxnn cases) ...
                default:
                    std::cerr << "ERRO: Opcode FXNN desconhecido: 0x" << std::hex << opcode << std::endl;
            }
            break;
            
        default:
            std::cerr << "ERRO FATAL: Opcode Desconhecido: 0x" << std::hex << opcode << std::endl;
    }
}