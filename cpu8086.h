#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>

// Opcodes reais do 8086 (encoding verdadeiro, nao mais inventado).
// Registrador de 16 bits e codificado nos 3 bits baixos do opcode
// em varias familias (MOV imm, PUSH, POP, INC, DEC):
//   0=AX 1=CX 2=DX 3=BX 4=SP 5=BP 6=SI 7=DI
namespace Op {
    constexpr uint8_t ADD_RM_R16 = 0x01; // ADD r/m16, r16 (usa ModRM)
    constexpr uint8_t SUB_RM_R16 = 0x29; // SUB r/m16, r16 (usa ModRM)
    constexpr uint8_t PUSH_BASE  = 0x50; // 0x50 + reg
    constexpr uint8_t POP_BASE   = 0x58; // 0x58 + reg
    constexpr uint8_t INC_BASE   = 0x40; // 0x40 + reg
    constexpr uint8_t DEC_BASE   = 0x48; // 0x48 + reg
    constexpr uint8_t MOV_IMM16  = 0xB8; // 0xB8 + reg, imm16
    constexpr uint8_t HLT        = 0xF4;
}

// Bits da FLAGS (so os que ja usamos)
enum FlagBits : uint16_t {
    FLAG_CF = 1 << 0,
    FLAG_ZF = 1 << 6,
    FLAG_SF = 1 << 7,
    FLAG_OF = 1 << 11
};

class CPU8086 {
public:
    // Registradores gerais de 16 bits, na ordem de codificacao do 8086
    uint16_t AX = 0, CX = 0, DX = 0, BX = 0;
    uint16_t SP = 0, BP = 0, SI = 0, DI = 0;

    uint16_t FLAGS = 0;
    uint32_t instructionsExecuted = 0;

    // RAM simulada: 64KB (um segmento). Segmentos reais entram na Parte 2.
    static constexpr size_t MEM_SIZE = 0x10000;
    std::vector<uint8_t> ram;

    CPU8086();

    // Carrega o programa a partir do endereco 0x0000 da RAM e posiciona
    // a pilha (SP) no topo da memoria.
    void loadProgram(const uint8_t* data, size_t size);
    void run();

    std::string dumpRegisters() const;

private:
    size_t ip = 0;
    bool halted = false;

    void step();

    // Helpers de acesso a registrador via indice 0-7 (encoding do 8086)
    uint16_t& regByIndex(uint8_t index);

    // Helpers de pilha
    void push16(uint16_t value);
    uint16_t pop16();

    // Atualiza ZF/SF/OF/CF depois de uma operacao aritmetica de 16 bits
    void updateFlagsAdd(uint16_t a, uint16_t b, uint32_t result);
    void updateFlagsSub(uint16_t a, uint16_t b, uint32_t result);

    uint8_t fetch8();
    uint16_t fetch16();
};
