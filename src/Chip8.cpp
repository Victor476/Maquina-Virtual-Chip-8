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

        case 0x8000: // 8xyn - Aritméticas e Lógicas
            switch (n) {
                
                case 0x0: // 8xy0: LD Vx, Vy (Load)
                    // Lógica: Vx = Vy
                    V[x] = V[y];
                    std::cout << "DEBUG: Opcode 8XY0: LD V" << (int)x << ", V" << (int)y << ". V" << (int)x << "=" << (int)V[x] << std::endl;
                    break;
                
                case 0x1: // 8xy1: OR Vx, Vy (OR Bitwise)
                    // Lógica: Vx = Vx | Vy. VF não é afetado.
                    V[x] = V[x] | V[y];
                    std::cout << "DEBUG: Opcode 8XY1: OR V" << (int)x << ", V" << (int)y << std::endl;
                    break;
                
                case 0x2: // 8xy2: AND Vx, Vy (AND Bitwise)
                    // Lógica: Vx = Vx & Vy. VF não é afetado.
                    V[x] = V[x] & V[y];
                    std::cout << "DEBUG: Opcode 8XY2: AND V" << (int)x << ", V" << (int)y << std::endl;
                    break;
                
                case 0x3: // 8xy3: XOR Vx, Vy (XOR Bitwise)
                    // Lógica: Vx = Vx ^ Vy. VF não é afetado.
                    V[x] = V[x] ^ V[y];
                    std::cout << "DEBUG: Opcode 8XY3: XOR V" << (int)x << ", V" << (int)y << std::endl;
                    break;

                case 0x4: // 8xy4: ADD Vx, Vy (Soma com Carry)
                    // Lógica: Vx = Vx + Vy. VF = 1 se houver carry.
                    {
                        // Use um tipo de 16 bits para detectar o overflow
                        uint16_t result = (uint16_t)V[x] + (uint16_t)V[y];
                        
                        // Se houver overflow (resultado > 255), VF = 1
                        V[0xF] = (result > 255) ? 1 : 0;
                        
                        // O resultado é truncado para 8 bits
                        V[x] = (uint8_t)result;
                    }
                    std::cout << "DEBUG: Opcode 8XY4: ADD V" << (int)x << ", V" << (int)y << ". Carry=" << (int)V[0xF] << std::endl;
                    break;

                case 0x5: // 8xy5: SUB Vx, Vy (Subtração com Borrow)
                    // Lógica: Vx = Vx - Vy. VF = 1 se NÃO houver borrow (Vx >= Vy).
                    
                    // VF = 1 se Vx >= Vy antes da subtração
                    V[0xF] = (V[x] >= V[y]) ? 1 : 0;
                    
                    // A subtração em 8 bits é automática, e o overflow é o resultado
                    V[x] = V[x] - V[y]; 
                    
                    std::cout << "DEBUG: Opcode 8XY5: SUB V" << (int)x << ", V" << (int)y << ". NoBorrow=" << (int)V[0xF] << std::endl;
                    break;
                
                case 0x6: // 8xy6: SHR Vx, {Vy} (Shift Right)
                    // Lógica: Vx = Vx >> 1. VF = LSB antes do shift.
                    
                    // VF recebe o bit menos significativo de Vx (o bit 0)
                    V[0xF] = V[x] & 0x1;
                    
                    V[x] >>= 1;
                    
                    // Nota: O opcode original usava o valor de Vy, mas a implementação moderna ignora Vy e usa Vx.
                    std::cout << "DEBUG: Opcode 8XY6: SHR V" << (int)x << ". VF=" << (int)V[0xF] << std::endl;
                    break;
                
                case 0x7: // 8xy7: SUBN Vx, Vy (Reverse Subtraction)
                    // Lógica: Vx = Vy - Vx. VF = 1 se NÃO houver borrow (Vy >= Vx).
                    
                    // VF = 1 se Vy >= Vx antes da subtração
                    V[0xF] = (V[y] >= V[x]) ? 1 : 0;
                    
                    V[x] = V[y] - V[x];
                    
                    std::cout << "DEBUG: Opcode 8XY7: SUBN V" << (int)x << ", V" << (int)y << ". NoBorrow=" << (int)V[0xF] << std::endl;
                    break;
                
                case 0xE: // 8xyE: SHL Vx, {Vy} (Shift Left)
                    // Lógica: Vx = Vx << 1. VF = MSB antes do shift.
                    
                    // VF recebe o bit mais significativo de Vx (o bit 7)
                    V[0xF] = (V[x] & 0x80) >> 7; 
                    
                    V[x] <<= 1;
                    
                    std::cout << "DEBUG: Opcode 8XYE: SHL V" << (int)x << ". VF=" << (int)V[0xF] << std::endl;
                    break;

                default:
                    std::cerr << "ERRO: Opcode 8xyn desconhecido: 0x" << std::hex << opcode << std::endl;
            }
            break; // Fim do case 0x8000
        
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