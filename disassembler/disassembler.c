#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "disassembler.h"
#include "../instructions/instructions.h"
#include "../memory/memory.h"

static const char *registroBaseNames[32] = {
        "LAR", "MAR", "MBR", "IP", "OPC", "OP1", "OP2", "SP",
        "BP", "R9", "EAX", "EBX", "ECX", "EDX", "EEX", "EFX",
        "AC", "CC", "R18", "R19", "R20", "R21", "R22", "R23",
        "R24", "R25", "CS", "DS", "ES", "SS", "KS", "PS"
};

static const char *general32[] = {"EAX", "EBX", "ECX", "EDX", "EEX", "EFX"};
static const char *general16[] = {"AX", "BX", "CX", "DX", "EX", "FX"};
static const char *general8L[] = {"AL", "BL", "CL", "DL", "EL", "FL"};
static const char *general8H[] = {"AH", "BH", "CH", "DH", "EH", "FH"};

static const char *nombreRegistro(uint8_t reg, uint8_t sector) {
    if (reg >= 32) {
        static char nombreGenerico[6];
        snprintf(nombreGenerico, sizeof(nombreGenerico), "R%u", reg);
        return nombreGenerico;
    }

    int idxGeneral = reg >= REG_EAX && reg <= REG_EFX ? reg - REG_EAX : -1;
    if (idxGeneral >= 0) {
        switch (sector) {
            case 0:
                return general32[idxGeneral];
            case 1:
                return general8L[idxGeneral];
            case 2:
                return general8H[idxGeneral];
            case 3:
                return general16[idxGeneral];
            default:
                return general32[idxGeneral];
        }
    }

    return registroBaseNames[reg];
}

static char prefijoMemoria(uint8_t codigoTam) {
    uint8_t tam = codigoTam & 0x03;
    if (tam == 0x03) return 'b';
    if (tam == 0x02) return 'w';
    return 'l';
}

static void imprimirCadenaConstante(CPU *cpu, uint16_t base, uint16_t offset, uint16_t longitud) {
    size_t bytesTotal = longitud;
    if (base + offset + longitud < base + cpu->segmentos[0].tamano && cpu->mem[base + offset + longitud] == '\0') {
        bytesTotal = longitud + 1;
    }

    size_t bytesMostrar = bytesTotal;
    int truncado = 0;
    if (bytesMostrar > 7) {
        bytesMostrar = 6;
        truncado = 1;
    }

    printf("[%04X] ", offset);
    for (size_t i = 0; i < bytesMostrar; ++i) {
        printf("%02X ", cpu->mem[base + offset + i]);
    }

    if (truncado) {
        printf(".. ");
    } else {
        for (size_t i = bytesMostrar; i < 7; ++i) {
            printf("   ");
        }
    }

    char buffer[256];
    size_t len = longitud < sizeof(buffer) - 1 ? longitud : sizeof(buffer) - 1;
    for (size_t i = 0; i < len; ++i) {
        uint8_t c = cpu->mem[base + offset + i];
        buffer[i] = (char)(isprint(c) ? c : '.');
    }
    buffer[len] = '\0';

    printf("| \"%s\"\n", buffer);
}

static void mostrarConstantes(CPU *cpu) {
    if (cpu->regs[REG_KS] == 0xFFFFFFFF) {
        return;
    }

    uint16_t indiceKS = (uint16_t)(cpu->regs[REG_KS] >> 16);
    if (indiceKS >= cpu->cantSegmentos) {
        return;
    }

    uint16_t base = cpu->segmentos[indiceKS].base;
    uint16_t tam = cpu->segmentos[indiceKS].tamano;
    uint16_t offset = 0;
    int seMostro = 0;

    while (offset < tam) {
        uint8_t c = cpu->mem[base + offset];
        if (c == '\0') {
            offset++;
            continue;
        }

        uint16_t inicio = offset;
        while (offset < tam && cpu->mem[base + offset] != '\0') {
            offset++;
        }

        uint16_t longitud = offset - inicio;
        if (longitud > 0) {
            imprimirCadenaConstante(cpu, base, inicio, longitud);
            seMostro = 1;
        }

        if (offset < tam) {
            offset++; // saltar null terminator
        }
    }

    if (seMostro) {
        printf("\n");
    }
}

void mostrarDisassembler(CPU *cpu, uint16_t tamanoCodigo) {
    mostrarConstantes(cpu);

    uint32_t selectorCS = cpu->regs[REG_CS];
    // uint16_t indiceCS = (uint16_t)(selectorCS >> 16);
    uint16_t entryPoint = (uint16_t)(selectorCS & 0xFFFF);
    uint32_t baseSelector = selectorCS & 0xFFFF0000u;

    // if (indiceCS >= cpu->cantSegmentos) {
    //     indiceCS = 0;
    // }

    uint32_t pos = 0;
    Instruccion instr;

    while (pos < tamanoCodigo) {
        uint32_t direccionLogica = baseSelector | (uint16_t) pos;
        uint32_t tamanoInstr = leerInstruccion(cpu, direccionLogica, &instr);
        if (tamanoInstr == 0) {
            break;
        }

        char marcador = (pos == entryPoint) ? '>' : ' ';
        printf("%c[%04X] ", marcador, pos);

        // Mostrar bytes de la instrucción
        for (uint32_t i = 0; i < tamanoInstr; i++) {
            uint32_t byteLogico = baseSelector | (uint16_t) (pos + i);
            uint32_t byteFisico = traducirDireccion(cpu, byteLogico, 1);
            printf("%02X ", cpu->mem[byteFisico]);
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
            "NOT", "???", "???", "PUSH", "POP", "CALL", "RET", "STOP",
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
    switch (op->tipo) {
        case TIPO_REGISTRO: {
            const char *nombre = nombreRegistro(op->datos.registro.codReg, op->datos.registro.sector);
            printf("%s", nombre);
            break;
        }
        case TIPO_INMEDIATO: {
            if (op->ancho == 1) {
                int8_t valor = (int8_t) op->datos.valor;
                if (isprint((unsigned char) valor)) {
                    printf("'%c'", valor);
                } else {
                    printf("%d", valor);
                }
            } else if (op->ancho == 2) {
                int16_t valor = (int16_t) op->datos.valor;
                printf("%d", valor);
            } else {
                int32_t valor = op->datos.valor;
                printf("%d", valor);
            }
            break;
        }
        case TIPO_MEMORIA: {
            char prefijo = prefijoMemoria(op->datos.memoria.tam);
            printf("%c[", prefijo);

            uint8_t reg = op->datos.memoria.codReg;
            int16_t desplazamiento = op->datos.memoria.offset;

            if (reg != 0) {
                printf("%s", nombreRegistro(reg, 0));
                if (desplazamiento > 0) {
                    printf("+%d", desplazamiento);
                } else if (desplazamiento < 0) {
                    printf("%d", desplazamiento);
                }
            } else {
                printf("%d", desplazamiento);
            }

            printf("]");
            break;
        }
        default:
            printf("???");
            break;
    }
}