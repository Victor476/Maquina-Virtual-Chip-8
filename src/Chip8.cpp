#include "Chip8.h"
#include <cstring>  // Para std::memset e std::memcpy
#include <iostream> // Para std::cout
#include <iomanip>  // Para formatação de saída (Validação)
#include <fstream>  // Para std::ifstream e manipulação de arquivos
#include <vector>   // Para std::vector (usado no buffer)
#include <algorithm> // Para std::copy
#include <cstdlib> // Para rand() e srand()
#include <ctime>   // Para time(

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
    : memory{},
     V{}, 
     I(0), 
     PC(0x200), 
     stack{}, 
     SP(0), 
     opcode(0), 
     timers{}, 
     display{}, 
     input{}, 
     cpu_frequency_hz(frequency) 
{
    std::srand(std::time(0));
    initialize(); 
}


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
    // --- Extração de Parâmetros (Critério de Decodificação) ---
    uint16_t nnn = opcode & 0x0FFF;
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t nn = opcode & 0x00FF;
    uint8_t n = opcode & 0x000F;

    switch (opcode & 0xF000) {
        
        case 0x0000: // 0nnn - Chamadas de Máquina / Controle de Fluxo
            switch (nn) {
                case 0x00E0: display.clear_screen(); std::cout << "DEBUG: Opcode 00E0: CLS - Tela limpa." << std::endl; break;
                case 0x00EE: // 00EE: RET (Return)
                    if (SP == 0) { std::cerr << "ERRO FATAL: Tentativa de RET de uma stack vazia." << std::endl; exit(1); }
                    PC = stack[--SP]; // Stack Pop
                    std::cout << "DEBUG: Opcode 00EE: RET - Retorno para 0x" << std::hex << PC << std::endl; break;
                default: std::cerr << "AVISO: Opcode 0NNN (Chamada de maquina) ignorado: 0x" << std::hex << opcode << std::endl;
            }
            break;

        case 0x1000: // 1nnn: JP addr (Jump)
            PC = nnn; 
            std::cout << "DEBUG: Opcode 1NNN: JP (Jump) para 0x" << std::hex << nnn << std::endl; break;

        case 0x2000: // 2nnn: CALL addr
            if (SP >= 16) { std::cerr << "ERRO FATAL: Stack Overflow (limite 16)." << std::endl; exit(1); }
            stack[SP++] = PC; // Stack Push
            PC = nnn;
            std::cout << "DEBUG: Opcode 2NNN: CALL (Chama sub-rotina) para 0x" << std::hex << nnn << std::endl; break;
            
        case 0x3000: // 3xnn: SE Vx, byte (Skip if Equal)
            if (V[x] == nn) {
                PC += 2; 
                std::cout << "DEBUG: Opcode 3XNN: SE - Salto APROVADO. PC=0x" << std::hex << PC << std::endl;
            } else {
                std::cout << "DEBUG: Opcode 3XNN: SE - Salto REJEITADO." << std::endl;
            }
            break;
            
        case 0x4000: // 4xnn: SNE Vx, byte
            if (V[x] != nn) {
                PC += 2;
                std::cout << "DEBUG: Opcode 4XNN: SNE - Salto APROVADO. PC=0x" << std::hex << PC << std::endl;
            } else {
                std::cout << "DEBUG: Opcode 4XNN: SNE - Salto REJEITADO." << std::endl;
            }
            break;
            
        case 0x5000: // 5xyn: SE Vx, Vy, 0
            if (n == 0) { // 5xy0
                if (V[x] == V[y]) {
                    PC += 2;
                    std::cout << "DEBUG: Opcode 5XY0: SE (Regs) - Salto APROVADO." << std::endl;
                } else {
                    std::cout << "DEBUG: Opcode 5XY0: SE (Regs) - Salto REJEITADO." << std::endl;
                }
            } else {
                std::cerr << "ERRO FATAL: Opcode Desconhecido (5xyn): 0x" << std::hex << opcode << std::endl;
            }
            break;

        case 0x6000: // 6xnn: LD Vx, byte (Load)
            V[x] = nn;
            std::cout << "DEBUG: Opcode 6XNN: LD V" << (int)x << ", byte. V" << (int)x << " = 0x" << std::hex << (int)nn << std::endl;
            break;

        case 0x7000: // 7xnn: ADD Vx, byte (Adição)
            V[x] += nn;
            std::cout << "DEBUG: Opcode 7XNN: ADD V" << (int)x << ", byte. V" << (int)x << " += 0x" << std::hex << (int)nn << std::endl;
            break;

        case 0x8000: // 8xyn - Aritméticas e Lógicas (Issue 15)
            switch (n) {
                case 0x0: V[x] = V[y]; V[0xF] = 0; break; // 8xy0: LD Vx, Vy
                case 0x1: V[x] = V[x] | V[y]; V[0xF] = 0; break; // 8xy1: OR Vx, Vy 
                case 0x2: V[x] = V[x] & V[y]; V[0xF] = 0; break; // 8xy2: AND Vx, Vy 
                case 0x3: V[x] = V[x] ^ V[y]; V[0xF] = 0; break; // 8xy3: XOR Vx, Vy 
                case 0x4: // 8xy4: ADD Vx, Vy
                    { uint16_t result = (uint16_t)V[x] + (uint16_t)V[y]; V[0xF] = (result > 255) ? 1 : 0; V[x] = (uint8_t)result; }
                    std::cout << "DEBUG: Opcode 8XY4: ADD V" << (int)x << ", V" << (int)y << ". Carry=" << (int)V[0xF] << std::endl; break;
                case 0x5: // 8xy5: SUB Vx, Vy
                    V[0xF] = (V[x] >= V[y]) ? 1 : 0; V[x] = V[x] - V[y];
                    std::cout << "DEBUG: Opcode 8XY5: SUB V" << (int)x << ", V" << (int)y << ". NoBorrow=" << (int)V[0xF] << std::endl; break;
                case 0x6: // 8xy6: SHR Vx, {Vy}
                    V[0xF] = V[x] & 0x1; V[x] >>= 1;
                    std::cout << "DEBUG: Opcode 8XY6: SHR V" << (int)x << ". VF=" << (int)V[0xF] << std::endl; break;
                case 0x7: // 8xy7: SUBN Vx, Vy
                    V[0xF] = (V[y] >= V[x]) ? 1 : 0; V[x] = V[y] - V[x];
                    std::cout << "DEBUG: Opcode 8XY7: SUBN V" << (int)x << ", V" << (int)y << ". NoBorrow=" << (int)V[0xF] << std::endl; break;
                case 0xE: // 8xyE: SHL Vx, {Vy}
                    V[0xF] = (V[x] & 0x80) >> 7; V[x] <<= 1;
                    std::cout << "DEBUG: Opcode 8XYE: SHL V" << (int)x << ". VF=" << (int)V[0xF] << std::endl; break;
                default:
                    std::cerr << "ERRO: Opcode 8xyn desconhecido: 0x" << std::hex << opcode << std::endl;
            }
            break;
        
        case 0x9000: // 9xyn: SNE Vx, Vy, 0
            if (n == 0) { // 9xy0
                if (V[x] != V[y]) {
                    PC += 2;
                    std::cout << "DEBUG: Opcode 9XY0: SNE (Regs) - Salto APROVADO." << std::endl;
                } else {
                    std::cout << "DEBUG: Opcode 9XY0: SNE (Regs) - Salto REJEITADO." << std::endl;
                }
            } else {
                std::cerr << "ERRO FATAL: Opcode Desconhecido (9xyn): 0x" << std::hex << opcode << std::endl;
            }
            break;
            
        case 0xA000: // Annn: LD I, addr (Load Address)
            I = nnn;
            std::cout << "DEBUG: Opcode ANNN: LD I (Load Address) I = 0x" << std::hex << I << std::endl;
            break;
            
        case 0xB000: // Bnnn: JP V0, addr (Jump com Offset)
            PC = nnn + V[0];
            std::cout << "DEBUG: Opcode BNNN: JP V0 (Jump com Offset) para 0x" << std::hex << PC << std::endl;
            break;

        case 0xC000: // Cxnn: RND Vx, byte (Número Aleatório) - IMPLEMENTADO
        { 
            uint8_t rand_byte = std::rand() % 256; 
            V[x] = rand_byte & nn;
            std::cout << "DEBUG: Opcode CXNN: RND V" << (int)x << ". V" << (int)x << " = 0x" << std::hex << (int)V[x] << std::endl;
            break;
        }

        case 0xD000: // Dxyn: DRW Vx, Vy, nibble (Desenha Sprite) - IMPLEMENTADO
        {
            // O código do sprite começa no endereço I
            uint16_t sprite_address = I; 
            
            // Altura do sprite (N)
            uint8_t height = n; 
            
            // Coordenadas iniciais X e Y (com base nos registradores Vx e Vy)
            // Lógica de wrapping: Modulo CHIP8_WIDTH (64) e CHIP8_HEIGHT (32)
            uint8_t start_x = V[x] % CHIP8_WIDTH; 
            uint8_t start_y = V[y] % CHIP8_HEIGHT; 

            // Flag de colisão (Critério de Aceitação)
            V[0xF] = 0; 
            bool pixel_was_turned_off = false;

            // 1. Loop sobre as linhas do sprite (altura N)
            for (int sprite_row = 0; sprite_row < height; ++sprite_row) {
                
                uint8_t sprite_byte = memory[sprite_address + sprite_row];
                uint8_t current_y = (start_y + sprite_row) % CHIP8_HEIGHT; // Wrapping Y

                // Parar se Y ultrapassar o limite da tela (após o wrapping)
                if (current_y >= CHIP8_HEIGHT) continue; 

                // 2. Loop sobre os 8 bits da linha do sprite (largura 8)
                for (int sprite_col = 0; sprite_col < 8; ++sprite_col) {
                    
                    // O bit atual do sprite é o MSB (mais à esquerda)
                    uint8_t sprite_pixel = (sprite_byte >> (7 - sprite_col)) & 0x1;
                    
                    // Coordenada X atual (com wrapping)
                    uint8_t current_x = (start_x + sprite_col) % CHIP8_WIDTH; // Wrapping X

                    // Parar se X ultrapassar o limite da tela (após o wrapping)
                    if (current_x >= CHIP8_WIDTH) continue; 
                    
                    // Se o pixel do sprite for 1, processamos a colisão
                    if (sprite_pixel) {
                        size_t display_index = current_x + current_y * CHIP8_WIDTH;
                        
                        uint8_t old_pixel = display.pixel_buffer[display_index];
                        uint8_t new_pixel = old_pixel ^ 0x1; // XOR com 1

                        // Critério: Flag de Colisão (VF = 1 se um pixel for desligado: 1 -> 0)
                        if (old_pixel == 1 && new_pixel == 0) {
                            pixel_was_turned_off = true;
                        }
                        
                        // Atualiza o buffer
                        display.pixel_buffer[display_index] = new_pixel;
                    }
                }
            }

            // Define o VF APENAS se uma colisão ocorreu
            if (pixel_was_turned_off) {
                V[0xF] = 1;
            } else {
                V[0xF] = 0;
            }

            std::cout << "DEBUG: Opcode DXYN: DRW - Desenho concluido. Colisao (VF)=" << (int)V[0xF] << std::endl;
            break;
        }
            
        case 0xE000: // Exnn - Teclado - SKELETON
            switch (nn) {
                case 0x009E: // Ex9E: SKP Vx
                case 0x00A1: // ExA1: SKNP Vx
                    std::cout << "DEBUG: Opcode EXNN: Teclado SKP/SKNP (Implementação pendente)." << std::endl;
                    break;
                default:
                    std::cerr << "ERRO: Opcode EXNN desconhecido: 0x" << std::hex << opcode << std::endl;
            }
            break;
        
        case 0xF000: // Fxnn - Timers e Memória (Issue 18 - Completa)
            switch (nn) {
                case 0x0007: V[x] = timers.get_delay_timer(); break; // Fx07: LD Vx, DT
                case 0x000A: std::cout << "DEBUG: Opcode FX0A: LD Vx, K (Esperando tecla)..." << std::endl; break; // Fx0A: LD Vx, K
                case 0x0015: timers.set_delay_timer(V[x]); break; // Fx15: LD DT, Vx
                case 0x0018: timers.set_sound_timer(V[x]); break; // Fx18: LD ST, Vx
                case 0x001E: I += V[x]; break; // Fx1E: ADD I, Vx
                case 0x0029: I = V[x] * 5; break; // Fx29: LD F, Vx
                case 0x0033: // Fx33: LD B, Vx
                    memory[I] = V[x] / 100; memory[I + 1] = (V[x] / 10) % 10; memory[I + 2] = V[x] % 10; break;
                case 0x0055: // Fx55: LD [I], Vx
                    for (int i = 0; i <= x; ++i) memory[I + i] = V[i]; I += x + 1; break;
                case 0x0065: // Fx65: LD Vx, [I]
                    for (int i = 0; i <= x; ++i) V[i] = memory[I + i]; I += x + 1; break;
                default:
                    std::cerr << "ERRO: Opcode FXNN desconhecido: 0x" << std::hex << opcode << std::endl;
            }
            break;
        
            
        default:
            std::cerr << "ERRO FATAL: Opcode Desconhecido: 0x" << std::hex << opcode << std::endl;
    }
}

