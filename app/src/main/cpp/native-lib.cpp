#include <jni.h>
#include <string>
#include "cpu8086.h"

extern "C" JNIEXPORT jstring JNICALL
Java_com_gabedeveloper_xodo_MainActivity_runDemo(JNIEnv* env, jobject /* this */) {
    CPU8086 cpu;

    // Programa de teste com opcodes REAIS do 8086:
    //   MOV AX, 5        ; B8 05 00
    //   MOV BX, 3        ; BB 03 00
    //   ADD AX, BX       ; 01 D8   (ModRM: mod=11, reg=BX(3), rm=AX(0))
    //   PUSH AX          ; 50
    //   MOV AX, 0        ; B8 00 00  (zera AX pra provar que o POP recupera)
    //   POP AX           ; 58
    //   DEC AX           ; 48        (AX vira 7, testa flags)
    //   HLT              ; F4
    uint8_t program[] = {
        0xB8, 0x05, 0x00,       // MOV AX, 5
        0xBB, 0x03, 0x00,       // MOV BX, 3
        0x01, 0xD8,             // ADD AX, BX
        0x50,                   // PUSH AX
        0xB8, 0x00, 0x00,       // MOV AX, 0
        0x58,                   // POP AX
        0x48,                   // DEC AX
        0xF4                    // HLT
    };

    cpu.loadProgram(program, sizeof(program));
    cpu.run();

    std::string result =
        "Instrucoes executadas: " + std::to_string(cpu.instructionsExecuted) + "\n" +
        cpu.dumpRegisters() + "\n\n" +
        "Agora com opcodes REAIS do 8086 (nao mais inventados),\n"
        "64KB de RAM simulada, pilha (PUSH/POP) e flags de verdade.\n"
        "Proxima parte: JMP/Jcc/CALL/RET + interrupcoes BIOS/DOS\n"
        "pra rodar um .com de verdade na tela.";

    return env->NewStringUTF(result.c_str());
}

