#include "Chip8.h"
#include <cstring>  // Para std::memset e std::memcpy
#include <iostream> // Para std::cout
#include <iomanip>  // Para formatação de saída (Validação)
#include <fstream>  // Para std::ifstream e manipulação de arquivos
#include <vector>   // Para std::vector (usado no buffer)
#include <algorithm> // Para std::copy
#include <SDL3/SDL.h>

// Os includes dos componentes são feitos via Chip8.h, mas mantidos aqui para clareza em alguns ambientes
// #include "components/TimerManager.h" 
// #include "components/Display.h" 
// #include "components/Input.h"

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

// =====================================================================
// CONSTRUTOR
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
    initialize(); 
}

// Construtor padrão (necessário caso a frequência não seja passada)
Chip8::Chip8() : Chip8(500) {} 

// =====================================================================
// INICIALIZAÇÃO E CONFIGURAÇÃO
// =====================================================================

void Chip8::initialize() {
    // A. Zera componentes internos da CPU (Lógica de Reset)
    // Nota: Os arrays e uints já foram zerados/inicializados na Lista de Inicialização do construtor.
    // std::memset(memory.data(), 0, memory.size()); 
    // std::memset(V, 0, sizeof(V));               
    // std::memset(stack, 0, sizeof(stack));       

    I = 0;
    SP = 0;
    opcode = 0;
    PC = 0x200; // PC inicializado no construtor, mas aqui garantimos o valor de reset

    // B. Inicializa/Zera componentes modulares (Lógica de Reset)
    timers.set_delay_timer(0); 
    timers.set_sound_timer(0); 
    display.clear_screen();    
    input.reset_keys();        

    // C. Pré-carrega o Font Set na Memória (Lógica de Configuração)
    std::memcpy(memory.data(), CHIP8_FONTSET, sizeof(CHIP8_FONTSET));

    // --- Validação ---
    std::cout << "--- Chip-8 VM Inicializada ---" << std::endl;
    std::cout << "PC: 0x" << std::hex << std::setw(4) << std::setfill('0') << PC << " (Esperado: 0x0200)" << std::endl;
    std::cout << "I: 0x" << (int)I << ", SP: " << (int)SP << ", DT: " << (int)timers.get_delay_timer() << ", ST: " << (int)timers.get_sound_timer() << " (Esperado: 0)" << std::endl;
    std::cout << "Memória[0x000]: 0x" << std::hex << (int)memory[0x000] << " (Esperado: 0xF0)" << std::endl;
    std::cout << "Memória[0x050]: 0x" << std::hex << (int)memory[0x050] << " (Esperado: 0x00)" << std::endl;
}

// =====================================================================
// CARREGAMENTO DA ROM (IMPLEMENTAÇÃO FINAL)
// =====================================================================

// Endereço máximo da memória disponível para o programa (0xFFF - 0x200 = 3584 bytes)
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

    // 4. Copiar para a Memória
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
    uint16_t primeiro_byte = load_address;
    uint16_t ultimo_byte = load_address + size - 1;
    std::cout << "Primeiro byte ROM (0x" << std::hex << primeiro_byte << "): 0x" 
              << std::setw(2) << std::setfill('0') << (int)memory[primeiro_byte] << std::endl;
    std::cout << "Ultimo byte ROM (0x" << ultimo_byte << "): 0x" 
              << std::setw(2) << std::setfill('0') << (int)memory[ultimo_byte] << std::endl;
    std::cout << "Byte apos o final (0x" << ultimo_byte + 1 << "): 0x" 
              << std::setw(2) << std::setfill('0') << (int)memory[ultimo_byte + 1] << std::endl;
}


void Chip8::execute_opcode(uint16_t opcode) {
    // Extração dos parâmetros comuns (Critério de Aceitação)
    
    // NNN (Endereço): Os 12 bits inferiores (0x0FFF)
    uint16_t nnn = opcode & 0x0FFF;
    
    // X (Registrador): O segundo nibble (0x0F00 >> 8)
    uint8_t x = (opcode & 0x0F00) >> 8;
    
    // Y (Registrador): O terceiro nibble (0x00F0 >> 4)
    uint8_t y = (opcode & 0x00F0) >> 4;
    
    // NN (Byte): O byte mais baixo (0x00FF)
    uint8_t nn = opcode & 0x00FF;
    
    // N (Nibble): O nibble mais baixo (0x000F)
    uint8_t n = opcode & 0x000F;

    // Decodificação usando o primeiro nibble (opcode & 0xF000)
    switch (opcode & 0xF000) {
        
        // =========================================================================
        case 0x0000: // 0nnn - Chamadas de Máquina / Controle de Fluxo
        // =========================================================================
            switch (nn) {
                case 0x00E0: // 00E0: CLS (Limpa a tela)
                    display.clear_screen();
                    std::cout << "DEBUG: Opcode 00E0: CLS - Tela limpa." << std::endl;
                    break;
                case 0x00EE: // 00EE: RET (Retorna de sub-rotina)
                    // Lógica: PC = stack[--SP]
                    // Implementação completa virá na Issue de Stack
                    std::cout << "DEBUG: Opcode 00EE: RET - Retorno de sub-rotina." << std::endl;
                    break;
                default: // 0NNN (Chamada para rotina da máquina) - Geralmente ignorado
                    std::cerr << "AVISO: Opcode 0NNN (Chamada de maquina) ignorado: 0x" << std::hex << opcode << std::endl;
            }
            break;

        // =========================================================================
        case 0x1000: // 1nnn: JP addr (Jump) - CRÍTICO PARA FLUXO DE CONTROLE
        // =========================================================================
            // Lógica: PC recebe o endereço nnn.
            // O PC já foi incrementado em 2 pelo fetch, então simplesmente o definimos para o novo endereço.
            PC = nnn; 
            std::cout << "DEBUG: Opcode 1NNN: JP (Jump) para 0x" << std::hex << nnn << std::endl;
            break;

        // =========================================================================
        case 0x2000: // 2nnn: CALL addr (Chama sub-rotina)
        // =========================================================================
            // Lógica: stack[++SP] = PC; PC = nnn;
            // Implementação completa virá na Issue de Stack
            std::cout << "DEBUG: Opcode 2NNN: CALL (Chama sub-rotina) para 0x" << std::hex << nnn << std::endl;
            break;
            
        // =========================================================================
        // CATEGORIA 3 a C (Instruções de Salto Condicional e Atribuição Simples)
        // =========================================================================

        case 0x3000: // 3xnn: SE Vx, byte (Skip if Equal)
            std::cout << "DEBUG: Opcode 3XNN: SE (Salto se Vx == NN)." << std::endl;
            break;
            
        case 0x4000: // 4xnn: SNE Vx, byte (Skip if Not Equal)
            std::cout << "DEBUG: Opcode 4XNN: SNE (Salto se Vx != NN)." << std::endl;
            break;

        case 0x5000: // 5xy0: SE Vx, Vy (Skip if Equal - Registradores)
            // Lógica interna: switch (n) { case 0x0: ...}
            std::cout << "DEBUG: Opcode 5XY0: SE (Salto se Vx == Vy)." << std::endl;
            break;

        case 0x6000: // 6xnn: LD Vx, byte (Load)
            std::cout << "DEBUG: Opcode 6XNN: LD (Load) NN em V" << (int)x << "." << std::endl;
            break;

        case 0x7000: // 7xnn: ADD Vx, byte (Adição)
            std::cout << "DEBUG: Opcode 7XNN: ADD (Adiciona) NN a V" << (int)x << "." << std::endl;
            break;

        case 0x9000: // 9xy0: SNE Vx, Vy (Skip if Not Equal - Registradores)
            // Lógica interna: switch (n) { case 0x0: ...}
            std::cout << "DEBUG: Opcode 9XY0: SNE (Salto se Vx != Vy)." << std::endl;
            break;

        case 0xA000: // Annn: LD I, addr (Load Address)
            std::cout << "DEBUG: Opcode ANNN: LD I (Load Address) " << std::hex << nnn << " em I." << std::endl;
            break;

        case 0xB000: // Bnnn: JP V0, addr (Jump com Offset)
            std::cout << "DEBUG: Opcode BNNN: JP (Jump com Offset V0)." << std::endl;
            break;
            
        case 0xC000: // Cxnn: RND Vx, byte (Número Aleatório)
            std::cout << "DEBUG: Opcode CXNN: RND (Random) com mascara NN em V" << (int)x << "." << std::endl;
            break;

        // =========================================================================
        case 0x8000: // 8xyn - Aritméticas e Lógicas
        // =========================================================================
            switch (n) {
                case 0x0: /* 8xy0: LD Vx, Vy */ break;
                case 0x1: /* 8xy1: OR Vx, Vy */ break;
                // ... (outros cases 0x2 a 0x7) ...
                case 0xE: /* 8xye: SHL Vx, {, Vy} */ break;
                default:
                    std::cerr << "ERRO: Opcode 8xyn desconhecido: 0x" << std::hex << opcode << std::endl;
            }
            break;

        // =========================================================================
        case 0xD000: // Dxyn: DRW Vx, Vy, nibble (Desenha Sprite)
        // =========================================================================
            // Implementação completa virá na Issue de Desenho
            std::cout << "DEBUG: Opcode DXYN: DRW (Desenho) de sprite com altura " << (int)n << "." << std::endl;
            break;
            
        // =========================================================================
        case 0xE000: // Exnn - Teclado
        // =========================================================================
            switch (nn) {
                case 0x009E: // Ex9E: SKP Vx (Skip if Key Pressed)
                    std::cout << "DEBUG: Opcode EX9E: SKP (Salto se tecla V" << (int)x << " pressionada)." << std::endl;
                    break;
                case 0x00A1: // ExA1: SKNP Vx (Skip if Key Not Pressed)
                    std::cout << "DEBUG: Opcode EXA1: SKNP (Salto se tecla V" << (int)x << " não pressionada)." << std::endl;
                    break;
                default:
                    std::cerr << "ERRO: Opcode EXNN desconhecido: 0x" << std::hex << opcode << std::endl;
            }
            break;
        
        // =========================================================================
        case 0xF000: // Fxnn - Timers e Memória
        // =========================================================================
            switch (nn) {
                case 0x0007: /* Fx07: LD Vx, DT */ break;
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

void Chip8::process_input(SDL_Event& event) {
    input.handle_event(event);
}

void Chip8::update_timers() {
    timers.update_timers();
}

uint16_t Chip8::fetch_opcode() {
    // Critério: Ler dois bytes sequenciais da memória, começando no PC.
    // Lendo o Byte Mais Significativo (MSB)
    uint16_t msb = memory[PC]; 
    // Lendo o Byte Menos Significativo (LSB)
    uint16_t lsb = memory[PC + 1];

    // Critério: Combinar em um uint16_t, respeitando a ordem big-endian.
    // Big-Endian: MSB e deslocado 8 bits para a esquerda (0xXX00) e depois combinado com LSB (0xXXYY).
    opcode = (msb << 8) | lsb;

    // Critério: O PC deve ser incrementado em 2 após a busca.
    PC += 2; 

    return opcode;
}

// O ciclo da CPU agora é a sequência Fetch-Decode-Execute.
void Chip8::cycle() {
    // 1. FETCH
    uint16_t current_opcode = fetch_opcode();

    execute_opcode(current_opcode);   
 
    std::cout << "DEBUG: PC=0x" << std::hex << PC - 2 
              << ", Opcode Buscado: 0x" << current_opcode << std::endl;
}