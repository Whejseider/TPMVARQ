#include "dissasembler.h"
#include "../instructions/instructions.h"

void mostrarDisassembler(CPU *cpu, uint16_t tamanoCodigo) {
    uint32_t pos = 0;
    Instruccion instr;

    while (pos < tamanoCodigo) {
        uint32_t tamanoInstr = leerInstruccion(cpu, pos, &instr);

        // Mostrar dirección física
        printf("[%04X] ", pos);

        // Mostrar bytes de la instrucción
        for (uint32_t i = 0; i < tamanoInstr; i++) {
            printf("%02X ", cpu->mem[pos + i]);
        }

        // Formato
        for (uint32_t i = tamanoInstr; i < 6; i++) {
            printf("   ");
        }

        printf("| ");

        mostrarMnemonico(instr.opcode);

        // Mostrar operandos
        if (instr.op1.tipo != TIPO_NINGUNO) {
            printf(" ");
            mostrarOperando(&instr.op1, cpu);
        }

        if (instr.op2.tipo != TIPO_NINGUNO) {
            printf(", ");
            mostrarOperando(&instr.op2, cpu);
        }

        printf("\n");
        pos += tamanoInstr;
    }
}

void mostrarMnemonico(uint8_t opcode) {
    const char *mnemonicos[] = {
            "SYS", "JMP", "JZ", "JP", "JN", "JNZ", "JNP", "JNN",
            "NOT", "???", "???", "???", "???", "???", "???", "STOP",
            "MOV", "ADD", "SUB", "MUL", "DIV", "CMP", "SHL", "SHR",
            "SAR", "AND", "OR", "XOR", "SWAP", "LDL", "LDH", "RND"
    };

    if (opcode < 32) {
        printf("%s", mnemonicos[opcode]);
    } else {
        printf("???");
    }
}

void mostrarOperando(Operando *op, CPU *cpu) {
    uint32_t modo = cpu->regs[REG_EAX];
    uint32_t valor = op->datos.valor;

    switch (op->tipo) {
        case TIPO_REGISTRO: {
            const char *nombres[] = {
                    "LAR", "MAR", "MBR", "IP", "OPC", "OP1", "OP2", "???",
                    "???", "???", "EAX", "EBX", "ECX", "EDX", "EEX", "EFX",
                    "AC", "CC", "???", "???", "???", "???", "???", "???",
                    "???", "???", "CS", "DS", "???", "???", "???", "???"
            };
            if (valor < 32) {
                printf("%s", nombres[valor]);
            } else {
                printf("R%d", valor);
            }
            break;
        }

        case TIPO_INMEDIATO: {
            if (modo & 0x10) { // binario
                for (int i = 31; i >= 0; i--) {
                    printf("%d", (valor >> i) & 1);
                }
            } else if (modo & 0x08) { // hexadecimal
                printf("0x%X", valor);
            } else if (modo & 0x04) { // octal
                printf("0%o", valor);
            } else if (modo & 0x02) { // char
                if (valor >= 32 && valor <= 126) {
                    printf("'%c'", (char) valor);
                } else {
                    printf("'?'"); // no imprimible
                }
            } else if (modo & 0x01) { // decimal
                printf("%d", valor);
            } else {
                printf("%d", valor);
            }
            break;
        }

        case TIPO_MEMORIA: {
            uint8_t reg = op->datos.memoria.codReg;
            uint16_t despl = op->datos.memoria.offset;

            printf("[");
            if (reg == 0) {
                printf("DS");
            } else {
                const char *nombres[] = {
                        "LAR", "MAR", "MBR", "IP", "OPC", "OP1", "OP2", "???",
                        "???", "???", "EAX", "EBX", "ECX", "EDX", "EEX", "EFX",
                        "AC", "CC", "???", "???", "???", "???", "???", "???",
                        "???", "???", "CS", "DS", "???", "???", "???", "???"
                };
                if (reg < 32) {
                    printf("%s", nombres[reg]);
                } else {
                    printf("R%d", reg);
                }
            }

            if (despl > 0) {
                printf("+%d", despl);
            } else if (despl < 0) {
                printf("%d", despl);
            }

            printf("]");
            break;
        }

        default:
            printf("???");
            break;
    }
}