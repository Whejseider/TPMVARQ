#include <time.h>
#include "instructions.h"
#include "../memory/memory.h"

uint32_t leerInstruccion(CPU *cpu, uint32_t direccion, Instruccion *instr) {
    uint32_t pos = 0;
    uint8_t primerByte;

    uint32_t direccionFisica = traducirDireccion(cpu, direccion, 1);
    primerByte = cpu->mem[direccionFisica];
    pos++;

    instr->direccion = direccion;

    uint8_t tipo_op2 = (primerByte >> 6) & 0x03;
    uint8_t tipo_op1 = (primerByte >> 4) & 0x03;

    if (tipo_op1 == 0 && tipo_op2 != 0) {
        uint8_t tipo_aux = tipo_op2;
        tipo_op2 = tipo_op1;
        tipo_op1 = tipo_aux;
    }

    instr->opcode = primerByte & 0x1F;

    instr->op2.tipo = tipo_op2;
    if (tipo_op2 == TIPO_REGISTRO) {
        instr->op2.datos.valor = leerMemoria8(cpu, direccion + pos) & 0x1F;
        pos += 1;
    } else if (tipo_op2 == TIPO_INMEDIATO) {
        int16_t valor16 = (int16_t) leerMemoria16(cpu, direccion + pos);
        instr->op2.datos.valor = (int32_t) valor16;
        pos += 2;
    } else if (tipo_op2 == TIPO_MEMORIA) {
        uint32_t dirLog = direccion + pos;
        instr->op2.datos.memoria.codReg = leerMemoria8(cpu, dirLog) & 0x1F;
        int16_t offset16 = (int16_t) leerMemoria16(cpu, dirLog + 1);
        instr->op2.datos.memoria.offset = (int32_t) offset16;
        pos += 3;
    } else if (tipo_op2 == TIPO_NINGUNO) {
        instr->op2.datos.valor = 0;
    }

    instr->op1.tipo = tipo_op1;
    if (tipo_op1 == TIPO_REGISTRO) {
        instr->op1.datos.valor = leerMemoria8(cpu, direccion + pos) & 0x1F;
        pos += 1;
    } else if (tipo_op1 == TIPO_INMEDIATO) {
        int16_t valor16 = (int16_t) leerMemoria16(cpu, direccion + pos);
        instr->op1.datos.valor = (int32_t) valor16;
        pos += 2;
    } else if (tipo_op1 == TIPO_MEMORIA) {
        uint32_t dirLog = direccion + pos;
        instr->op1.datos.memoria.codReg = leerMemoria8(cpu, dirLog) & 0x1F;
        int16_t offset16 = (int16_t) leerMemoria16(cpu, dirLog + 1);
        instr->op1.datos.memoria.offset = (int32_t) offset16;
        pos += 3;
    } else if (tipo_op1 == TIPO_NINGUNO) {
        instr->op1.datos.valor = 0;
    }

    return pos;
}

uint32_t obtenerValorOperando(CPU *cpu, Operando *op) {
    switch (op->tipo) {
        case TIPO_REGISTRO:
            return cpu->regs[op->datos.valor];
        case TIPO_INMEDIATO:
            return (uint32_t) op->datos.valor;
        case TIPO_MEMORIA: {
            uint8_t reg = op->datos.memoria.codReg;
            int32_t desplazamiento = op->datos.memoria.offset;
            uint32_t base = (reg == 0) ? cpu->regs[REG_DS] : cpu->regs[reg];
            uint32_t dirLog = base + (uint32_t) desplazamiento;
            return leerMemoria32(cpu, dirLog);
        }
        default:
            return 0;
    }
}

void establecerValorOperando(CPU *cpu, Operando *op, uint32_t valor) {
    switch (op->tipo) {
        case TIPO_REGISTRO:
            cpu->regs[op->datos.valor] = (int32_t) valor;
            break;
        case TIPO_MEMORIA: {
            uint8_t reg = op->datos.memoria.codReg;
            int32_t desplazamiento = op->datos.memoria.offset;
            uint32_t base = (reg == 0) ? cpu->regs[REG_DS] : cpu->regs[reg];
            uint32_t dirLog = base + (uint32_t) desplazamiento;
            escribirMemoria32(cpu, dirLog, valor);
            break;
        }
        default:
            break;
    }
}

// ==== INTRUCCIONES ====

// Instrucciones aritméticas y lógicas
uint32_t instr_mov(CPU *cpu, Instruccion *instr) {
    uint32_t valor = obtenerValorOperando(cpu, &instr->op2);
    establecerValorOperando(cpu, &instr->op1, valor);
    return leerInstruccion(cpu, instr->direccion, instr);
}

uint32_t instr_add(CPU *cpu, Instruccion *instr) {
    int32_t val1 = (int32_t) obtenerValorOperando(cpu, &instr->op1);
    int32_t val2 = (int32_t) obtenerValorOperando(cpu, &instr->op2);
    int32_t resultado = val1 + val2;
    establecerValorOperando(cpu, &instr->op1, (uint32_t) resultado);
    actualizarCC(cpu, (uint32_t) resultado);
    return leerInstruccion(cpu, instr->direccion, instr);
}

uint32_t instr_sub(CPU *cpu, Instruccion *instr) {
    int32_t val1 = (int32_t) obtenerValorOperando(cpu, &instr->op1);
    int32_t val2 = (int32_t) obtenerValorOperando(cpu, &instr->op2);
    int32_t resultado = val1 - val2;
    establecerValorOperando(cpu, &instr->op1, (uint32_t) resultado);
    actualizarCC(cpu, (uint32_t) resultado);
    return leerInstruccion(cpu, instr->direccion, instr);
}

uint32_t instr_mul(CPU *cpu, Instruccion *instr) {
    int32_t val1 = (int32_t) obtenerValorOperando(cpu, &instr->op1);
    int32_t val2 = (int32_t) obtenerValorOperando(cpu, &instr->op2);
    int32_t resultado = val1 * val2;
    establecerValorOperando(cpu, &instr->op1, (uint32_t) resultado);
    actualizarCC(cpu, (uint32_t) resultado);
    return leerInstruccion(cpu, instr->direccion, instr);
}

uint32_t instr_div(CPU *cpu, Instruccion *instr) {
    int32_t val1 = (int32_t) obtenerValorOperando(cpu, &instr->op1);
    int32_t val2 = (int32_t) obtenerValorOperando(cpu, &instr->op2);
    if (val2 == 0) {
        mostrarError("División por cero");
        exit(1);
    }
    int32_t resultado = val1 / val2;
    int32_t resto = val1 % val2;
    establecerValorOperando(cpu, &instr->op1, (uint32_t) resultado);
    cpu->regs[REG_AC] = (uint32_t) resto;
    actualizarCC(cpu, (uint32_t) resultado);
    return leerInstruccion(cpu, instr->direccion, instr);
}

uint32_t instr_cmp(CPU *cpu, Instruccion *instr) {
    int32_t val1 = (int32_t) obtenerValorOperando(cpu, &instr->op1);
    int32_t val2 = (int32_t) obtenerValorOperando(cpu, &instr->op2);
    int32_t resultado = val1 - val2;
    actualizarCC(cpu, (uint32_t) resultado);
    return leerInstruccion(cpu, instr->direccion, instr);
}

uint32_t instr_shl(CPU *cpu, Instruccion *instr) {
    uint32_t val1 = obtenerValorOperando(cpu, &instr->op1);
    uint32_t val2 = obtenerValorOperando(cpu, &instr->op2);
    uint32_t resultado = val1 << val2;
    establecerValorOperando(cpu, &instr->op1, resultado);
    actualizarCC(cpu, (uint32_t) resultado);
    return leerInstruccion(cpu, instr->direccion, instr);
}

uint32_t instr_shr(CPU *cpu, Instruccion *instr) {
    uint32_t val1 = obtenerValorOperando(cpu, &instr->op1);
    uint32_t val2 = obtenerValorOperando(cpu, &instr->op2);
    uint32_t resultado = val1 >> val2;
    establecerValorOperando(cpu, &instr->op1, resultado);
    actualizarCC(cpu, (uint32_t) resultado);
    return leerInstruccion(cpu, instr->direccion, instr);
}

uint32_t instr_sar(CPU *cpu, Instruccion *instr) {
    int32_t val1 = (int32_t) obtenerValorOperando(cpu, &instr->op1);
    int32_t val2 = (int32_t) obtenerValorOperando(cpu, &instr->op2);
    int32_t resultado = val1 >> val2;
    establecerValorOperando(cpu, &instr->op1, (uint32_t) resultado);
    actualizarCC(cpu, (uint32_t) resultado);
    return leerInstruccion(cpu, instr->direccion, instr);
}

uint32_t instr_and(CPU *cpu, Instruccion *instr) {
    uint32_t val1 = obtenerValorOperando(cpu, &instr->op1);
    uint32_t val2 = obtenerValorOperando(cpu, &instr->op2);
    uint32_t resultado = val1 & val2;
    establecerValorOperando(cpu, &instr->op1, resultado);
    actualizarCC(cpu, (uint32_t) resultado);
    return leerInstruccion(cpu, instr->direccion, instr);
}

uint32_t instr_or(CPU *cpu, Instruccion *instr) {
    uint32_t val1 = obtenerValorOperando(cpu, &instr->op1);
    uint32_t val2 = obtenerValorOperando(cpu, &instr->op2);
    uint32_t resultado = val1 | val2;
    establecerValorOperando(cpu, &instr->op1, resultado);
    actualizarCC(cpu, (uint32_t) resultado);
    return leerInstruccion(cpu, instr->direccion, instr);
}

uint32_t instr_xor(CPU *cpu, Instruccion *instr) {
    uint32_t val1 = obtenerValorOperando(cpu, &instr->op1);
    uint32_t val2 = obtenerValorOperando(cpu, &instr->op2);
    uint32_t resultado = val1 ^ val2;
    establecerValorOperando(cpu, &instr->op1, resultado);
    actualizarCC(cpu, (uint32_t) resultado);
    return leerInstruccion(cpu, instr->direccion, instr);
}

uint32_t instr_swap(CPU *cpu, Instruccion *instr) {
    uint32_t val1 = obtenerValorOperando(cpu, &instr->op1);
    uint32_t val2 = obtenerValorOperando(cpu, &instr->op2);
    establecerValorOperando(cpu, &instr->op1, val2);
    establecerValorOperando(cpu, &instr->op2, val1);
    return leerInstruccion(cpu, instr->direccion, instr);
}

uint32_t instr_ldl(CPU *cpu, Instruccion *instr) {
    uint32_t val1 = obtenerValorOperando(cpu, &instr->op1);
    uint32_t val2 = obtenerValorOperando(cpu, &instr->op2);
    uint32_t resultado = (val1 & 0xFFFF0000) | (val2 & 0x0000FFFF);
    establecerValorOperando(cpu, &instr->op1, resultado);
    return leerInstruccion(cpu, instr->direccion, instr);
}

uint32_t instr_ldh(CPU *cpu, Instruccion *instr) {
    uint32_t val1 = obtenerValorOperando(cpu, &instr->op1);
    uint32_t val2 = obtenerValorOperando(cpu, &instr->op2);
    uint32_t resultado = (val1 & 0x0000FFFF) | ((val2 & 0x0000FFFF) << 16);
    establecerValorOperando(cpu, &instr->op1, resultado);
    return leerInstruccion(cpu, instr->direccion, instr);
}

uint32_t instr_rnd(CPU *cpu, Instruccion *instr) {
    int32_t limite = (int32_t) obtenerValorOperando(cpu, &instr->op2);
    uint32_t resultado = 0;
    if (limite > 0) {
        static int inicializado = 0;
        if (!inicializado) {
            srand(time(NULL));
            inicializado = 1;
        }
        resultado = rand() % (limite + 1);
    }
    establecerValorOperando(cpu, &instr->op1, resultado);
    return leerInstruccion(cpu, instr->direccion, instr);
}

//
// Instrucciones de Salto
//
uint32_t instr_jmp(CPU *cpu, Instruccion *instr) {
    uint32_t direccion = obtenerValorOperando(cpu, &instr->op1);
    cpu->regs[REG_IP] = direccion;
    return 0;
}

uint32_t instr_jz(CPU *cpu, Instruccion *instr) {
    if (cpu->regs[REG_CC] & CC_Z_MASK) {
        uint32_t direccion = obtenerValorOperando(cpu, &instr->op1);
        cpu->regs[REG_IP] = direccion;
        return 0;
    }
    return leerInstruccion(cpu, instr->direccion, instr);
}

uint32_t instr_jp(CPU *cpu, Instruccion *instr) {
    if (!(cpu->regs[REG_CC] & (CC_N_MASK | CC_Z_MASK))) {
        uint32_t direccion = obtenerValorOperando(cpu, &instr->op1);
        cpu->regs[REG_IP] = direccion;
        return 0;
    }
    return leerInstruccion(cpu, instr->direccion, instr);
}

uint32_t instr_jn(CPU *cpu, Instruccion *instr) {
    if (cpu->regs[REG_CC] & CC_N_MASK) {
        uint32_t direccion = obtenerValorOperando(cpu, &instr->op1);
        cpu->regs[REG_IP] = direccion;
        return 0;
    }
    return leerInstruccion(cpu, instr->direccion, instr);
}

uint32_t instr_jnz(CPU *cpu, Instruccion *instr) {
    if (!(cpu->regs[REG_CC] & CC_Z_MASK)) {
        uint32_t direccion = obtenerValorOperando(cpu, &instr->op1);
        cpu->regs[REG_IP] = direccion;
        return 0;
    }
    return leerInstruccion(cpu, instr->direccion, instr);
}

uint32_t instr_jnp(CPU *cpu, Instruccion *instr) {
    if (cpu->regs[REG_CC] & (CC_N_MASK | CC_Z_MASK)) {
        uint32_t direccion = obtenerValorOperando(cpu, &instr->op1);
        cpu->regs[REG_IP] = direccion;
        return 0;
    }
    return leerInstruccion(cpu, instr->direccion, instr);
}

uint32_t instr_jnn(CPU *cpu, Instruccion *instr) {
    if (!(cpu->regs[REG_CC] & CC_N_MASK)) {
        uint32_t direccion = obtenerValorOperando(cpu, &instr->op1);
        cpu->regs[REG_IP] = direccion;
        return 0;
    }
    return leerInstruccion(cpu, instr->direccion, instr);
}


//
// Otros
//
uint32_t instr_not(CPU *cpu, Instruccion *instr) {
    uint32_t val = obtenerValorOperando(cpu, &instr->op1);
    uint32_t resultado = ~val;
    establecerValorOperando(cpu, &instr->op1, resultado);
    actualizarCC(cpu, (uint32_t) resultado);
    return leerInstruccion(cpu, instr->direccion, instr);
}

uint32_t instr_stop(CPU *cpu, Instruccion *instr) {
    cpu->regs[REG_IP] = 0xFFFFFFFF;
    cpu->ejecutando = 0;
    return 0;
}

uint32_t instr_sys(CPU *cpu, Instruccion *instr) {
    uint32_t syscall = obtenerValorOperando(cpu, &instr->op1);
    switch (syscall) {
        case 1:
            sysRead(cpu);
            break;
        case 2:
            sysWrite(cpu);
            break;
        default:
            fprintf(stderr, "Sistema call no implementada: %d\n",
                    syscall); // TODO Provisorio, modificar el mostrarError o algo pero bueno
            break;
    }
    return leerInstruccion(cpu, instr->direccion, instr);
}


void sysRead(CPU *cpu) {
    uint32_t modo = cpu->regs[REG_EAX];
    uint32_t direccion = cpu->regs[REG_EDX];
    uint32_t config = cpu->regs[REG_ECX];
    uint16_t cantidad = config & 0xFFFF;
    uint16_t tamano = (config >> 16) & 0xFFFF;

    for (int i = 0; i < cantidad; i++) {
        int32_t valor = 0;
        uint32_t direccionLogica = direccion + (i * tamano);
        printf("[%04X]: ", direccionLogica);

        if (modo & 0x10) { // binario
            char binStr[33];
            printf("Ingrese un numero en binario: ");
            scanf("%32s", binStr);
            valor = strtol(binStr, NULL, 2);
        } else if (modo & 0x08) { // hexadecimal
            printf("Ingrese un numero en hexadecimal: ");
            scanf("%x", (unsigned int *) &valor);
        } else if (modo & 0x04) { //octla
            printf("Ingrese un numero en octal: ");
            scanf("%o", (unsigned int *) &valor);
        }
        if (modo & 0x02) { // char
            char c;
            printf("Ingrese un caracter: ");
            scanf(" %c", &c);
            valor = (int32_t) c;
        } else if (modo & 0x01) { // decimal
            printf("Ingrese un numero en decimal: ");
            scanf("%d", &valor);
        }

        if (tamano == 1) {
            escribirMemoria8(cpu, direccionLogica, (uint8_t) valor);
        } else if (tamano == 2) {
            escribirMemoria16(cpu, direccionLogica, (uint16_t) valor);
        } else if (tamano == 4) {
            escribirMemoria32(cpu, direccionLogica, (uint32_t) valor);
        }
    }
}

void sysWrite(CPU *cpu) {
    uint32_t modo = cpu->regs[REG_EAX];
    uint32_t direccion = cpu->regs[REG_EDX];
    uint32_t config = cpu->regs[REG_ECX];
    uint16_t cantidad = config & 0xFFFF;
    uint16_t tamano = (config >> 16) & 0xFFFF;

    for (int i = 0; i < cantidad; i++) {
        uint32_t direccionLogica = direccion + (i * tamano);
        int32_t valor = 0;

        if (tamano == 1) {
            valor = (int32_t) (int8_t) leerMemoria8(cpu, direccionLogica);
        } else if (tamano == 2) {
            valor = (int32_t) (int16_t) leerMemoria16(cpu, direccionLogica);
        } else if (tamano == 4) {
            valor = (int32_t) leerMemoria32(cpu, direccionLogica);
        }

        printf("[%04X]: ", direccionLogica);


        if (modo & 0x10) { // binario
            if (modo & 0x0F) printf(" ");
            printf("0b");
            for (int bit = 31; bit >= 0; bit--) {
                printf("%d", (valor >> bit) & 1);
            }
        }
        if (modo & 0x08) { // hexadeicmal
            if (modo & 0x07) printf(" ");
            printf("0x%X", (unsigned int) valor);
        }
        if (modo & 0x04) { // octal
            if (modo & 0x03) printf(" ");
            printf("0o%o", (unsigned int) valor);
        }
        if (modo & 0x02) { // char
            if (modo & 0x01) printf(" ");
            char c = valor & 0xFF;
            if (c >= 32 && c <= 126) {
                printf(" %c ", c);
            } else {
                printf(" . ");
            }
        }
        if (modo & 0x01) { // decimal
            printf("%d", valor);
        }
        printf("\n");
    }
}