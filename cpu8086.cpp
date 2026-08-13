#include "cpu8086.h"
#include <sstream>

CPU8086::CPU8086() : ram(MEM_SIZE, 0) {}

uint16_t& CPU8086::regByIndex(uint8_t index) {
    switch (index & 0x07) {
        case 0: return AX;
        case 1: return CX;
        case 2: return DX;
        case 3: return BX;
        case 4: return SP;
        case 5: return BP;
        case 6: return SI;
        default: return DI;
    }
}

void CPU8086::loadProgram(const uint8_t* data, size_t size) {
    for (size_t i = 0; i < size && i < MEM_SIZE; ++i) {
        ram[i] = data[i];
    }
    ip = 0;
    halted = false;
    instructionsExecuted = 0;

    AX = CX = DX = BX = BP = SI = DI = 0;
    FLAGS = 0;

    // Pilha comeca no topo da RAM e cresce pra baixo, como no 8086 real.
    SP = static_cast<uint16_t>(MEM_SIZE - 2);
}

uint8_t CPU8086::fetch8() {
    return ram[ip++];
}

uint16_t CPU8086::fetch16() {
    uint16_t low = ram[ip++];
    uint16_t high = ram[ip++];
    return static_cast<uint16_t>(low | (high << 8));
}

void CPU8086::push16(uint16_t value) {
    SP -= 2;
    ram[SP] = static_cast<uint8_t>(value & 0xFF);
    ram[SP + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
}

uint16_t CPU8086::pop16() {
    uint16_t low = ram[SP];
    uint16_t high = ram[SP + 1];
    SP += 2;
    return static_cast<uint16_t>(low | (high << 8));
}

void CPU8086::updateFlagsAdd(uint16_t a, uint16_t b, uint32_t result) {
    FLAGS = 0;
    if ((result & 0xFFFF) == 0) FLAGS |= FLAG_ZF;
    if (result & 0x8000) FLAGS |= FLAG_SF;
    if (result > 0xFFFF) FLAGS |= FLAG_CF;
    bool signA = a & 0x8000, signB = b & 0x8000, signR = result & 0x8000;
    if (signA == signB && signR != signA) FLAGS |= FLAG_OF;
}

void CPU8086::updateFlagsSub(uint16_t a, uint16_t b, uint32_t result) {
    FLAGS = 0;
    if ((result & 0xFFFF) == 0) FLAGS |= FLAG_ZF;
    if (result & 0x8000) FLAGS |= FLAG_SF;
    if (b > a) FLAGS |= FLAG_CF;
    bool signA = a & 0x8000, signB = b & 0x8000, signR = result & 0x8000;
    if (signA != signB && signR != signA) FLAGS |= FLAG_OF;
}

void CPU8086::step() {
    if (ip >= MEM_SIZE) {
        halted = true;
        return;
    }

    uint8_t opcode = fetch8();

    // Familias de opcodes onde o registrador esta nos 3 bits baixos
    if (opcode >= Op::MOV_IMM16 && opcode <= Op::MOV_IMM16 + 7) {
        regByIndex(opcode - Op::MOV_IMM16) = fetch16();
    }
    else if (opcode >= Op::PUSH_BASE && opcode <= Op::PUSH_BASE + 7) {
        push16(regByIndex(opcode - Op::PUSH_BASE));
    }
    else if (opcode >= Op::POP_BASE && opcode <= Op::POP_BASE + 7) {
        regByIndex(opcode - Op::POP_BASE) = pop16();
    }
    else if (opcode >= Op::INC_BASE && opcode <= Op::INC_BASE + 7) {
        uint16_t& r = regByIndex(opcode - Op::INC_BASE);
        uint32_t result = static_cast<uint32_t>(r) + 1;
        updateFlagsAdd(r, 1, result);
        r = static_cast<uint16_t>(result);
    }
    else if (opcode >= Op::DEC_BASE && opcode <= Op::DEC_BASE + 7) {
        uint16_t& r = regByIndex(opcode - Op::DEC_BASE);
        uint32_t result = static_cast<uint32_t>(r) - 1;
        updateFlagsSub(r, 1, result);
        r = static_cast<uint16_t>(result);
    }
    else {
        switch (opcode) {
            case Op::ADD_RM_R16: {
                // ModRM com mod=11 (registrador-registrador direto).
                // Enderecamento de memoria ([BX+SI] etc) fica pra Parte 2.
                uint8_t modrm = fetch8();
                uint8_t regField = (modrm >> 3) & 0x07;
                uint8_t rmField = modrm & 0x07;
                uint16_t& dst = regByIndex(rmField);
                uint16_t src = regByIndex(regField);
                uint32_t result = static_cast<uint32_t>(dst) + src;
                updateFlagsAdd(dst, src, result);
                dst = static_cast<uint16_t>(result);
                break;
            }
            case Op::SUB_RM_R16: {
                uint8_t modrm = fetch8();
                uint8_t regField = (modrm >> 3) & 0x07;
                uint8_t rmField = modrm & 0x07;
                uint16_t& dst = regByIndex(rmField);
                uint16_t src = regByIndex(regField);
                uint32_t result = static_cast<uint32_t>(dst) - src;
                updateFlagsSub(dst, src, result);
                dst = static_cast<uint16_t>(result);
                break;
            }
            case Op::HLT:
                halted = true;
                break;

            default:
                // Opcode ainda nao implementado: paramos em vez de
                // executar algo errado silenciosamente.
                halted = true;
                break;
        }
    }

    instructionsExecuted++;
}

void CPU8086::run() {
    while (!halted) {
        step();
    }
}

std::string CPU8086::dumpRegisters() const {
    std::ostringstream out;
    out << "AX=" << AX << " CX=" << CX << " DX=" << DX << " BX=" << BX << "\n"
        << "SP=" << SP << " BP=" << BP << " SI=" << SI << " DI=" << DI << "\n"
        << "FLAGS: "
        << "ZF=" << ((FLAGS & FLAG_ZF) ? 1 : 0) << " "
        << "CF=" << ((FLAGS & FLAG_CF) ? 1 : 0) << " "
        << "SF=" << ((FLAGS & FLAG_SF) ? 1 : 0) << " "
        << "OF=" << ((FLAGS & FLAG_OF) ? 1 : 0);
    return out.str();
}
