#include <time.h>
#include "instructions.h"
#include "../memory/memory.h"

/**
 * Decodifica una instrucción desde memoria
 * Lee el opcode y operandos según el formato de instrucción VMX
 * 
 * @param cpu estructura de la CPU
 * @param direccion dirección donde está la instrucción
 * @param instr estructura donde guardar la instrucción decodificada
 * @return tamaño en bytes de la instrucción
 */
uint32_t leerInstruccion(CPU *cpu, uint32_t direccion, Instruccion *instr) {
    uint32_t pos = 0;
    uint8_t primerByte;

    uint32_t direccionFisica = traducirDireccion(cpu, direccion, 1);
    primerByte = cpu->mem[direccionFisica];
    pos++;

    instr->direccion = direccion;

    uint8_t tipoOp2 = (primerByte >> 6) & 0x03;
    uint8_t tipoOp1 = (primerByte >> 4) & 0x03;

    if (tipoOp1 == 0 && tipoOp2 != 0) {
        tipoOp1 = tipoOp2;
        tipoOp2 = 0;
    }

    instr->opcode = primerByte & 0x1F;

    instr->op2.tipo = tipoOp2;
    if (tipoOp2 == TIPO_REGISTRO) {
        instr->op2.datos.valor = leerMemoria8(cpu, direccion + pos) & 0x1F;
        pos += 1;
    } else if (tipoOp2 == TIPO_INMEDIATO) {
        int16_t valor16 = (int16_t) leerMemoria16(cpu, direccion + pos);
        instr->op2.datos.valor = (int32_t) valor16;
        pos += 2;
    } else if (tipoOp2 == TIPO_MEMORIA) {
        uint32_t dirLog = direccion + pos;
        instr->op2.datos.memoria.codReg = leerMemoria8(cpu, dirLog) & 0x1F;
        int16_t offset16 = (int16_t) leerMemoria16(cpu, dirLog + 1);
        instr->op2.datos.memoria.offset = (int32_t) offset16;
        pos += 3;
    } else if (tipoOp2 == TIPO_NINGUNO) {
        instr->op2.datos.valor = 0;
    }

    instr->op1.tipo = tipoOp1;
    if (tipoOp1 == TIPO_REGISTRO) {
        instr->op1.datos.valor = leerMemoria8(cpu, direccion + pos) & 0x1F;
        pos += 1;
    } else if (tipoOp1 == TIPO_INMEDIATO) {
        int16_t valor16 = (int16_t) leerMemoria16(cpu, direccion + pos);
        instr->op1.datos.valor = (int32_t) valor16;
        pos += 2;
    } else if (tipoOp1 == TIPO_MEMORIA) {
        uint32_t dirLog = direccion + pos;
        instr->op1.datos.memoria.codReg = leerMemoria8(cpu, dirLog) & 0x1F;
        int16_t offset16 = (int16_t) leerMemoria16(cpu, dirLog + 1);
        instr->op1.datos.memoria.offset = (int32_t) offset16;
        pos += 3;
    } else if (tipoOp1 == TIPO_NINGUNO) {
        instr->op1.datos.valor = 0;
    }

    return pos;
}

/**
 * Obtiene el valor de un operando según su tipo
 * Maneja registros, valores inmediatos y direcciones de memoria
 */
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

/**
 * Establece el valor de un operando según su tipo
 * Maneja registros y direcciones de memoria
 */
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

// ==== INSTRUCCIONES ARITMÉTICAS Y LÓGICAS ====

uint32_t instr_mov(CPU *cpu, Instruccion *instr) {
    uint32_t valor = obtenerValorOperando(cpu, &instr->op2);
    establecerValorOperando(cpu, &instr->op1, valor);
    return 1;
}

uint32_t instr_add(CPU *cpu, Instruccion *instr) {
    int32_t val1 = (int32_t) obtenerValorOperando(cpu, &instr->op1);
    int32_t val2 = (int32_t) obtenerValorOperando(cpu, &instr->op2);
    int32_t resultado = val1 + val2;
    establecerValorOperando(cpu, &instr->op1, (uint32_t) resultado);
    actualizarCC(cpu, (uint32_t) resultado);
    return 1;
}

uint32_t instr_sub(CPU *cpu, Instruccion *instr) {
    int32_t val1 = (int32_t) obtenerValorOperando(cpu, &instr->op1);
    int32_t val2 = (int32_t) obtenerValorOperando(cpu, &instr->op2);
    int32_t resultado = val1 - val2;
    establecerValorOperando(cpu, &instr->op1, (uint32_t) resultado);
    actualizarCC(cpu, (uint32_t) resultado);
    return 1;
}

uint32_t instr_mul(CPU *cpu, Instruccion *instr) {
    int32_t val1 = (int32_t) obtenerValorOperando(cpu, &instr->op1);
    int32_t val2 = (int32_t) obtenerValorOperando(cpu, &instr->op2);
    int32_t resultado = val1 * val2;
    establecerValorOperando(cpu, &instr->op1, (uint32_t) resultado);
    actualizarCC(cpu, (uint32_t) resultado);
    return 1;
}

uint32_t instr_div(CPU *cpu, Instruccion *instr) {
    int32_t dividendo = (int32_t) obtenerValorOperando(cpu, &instr->op1);
    int32_t divisor = (int32_t) obtenerValorOperando(cpu, &instr->op2);

    if (divisor == 0) {
        mostrarError("División por cero");
        exit(1);
    }

    int32_t cociente = dividendo / divisor;
    int32_t resto = dividendo % divisor;

    // Si hay resto y los signos son diferentes, ajustar hacia abajo
    if (resto != 0 && ((dividendo ^ divisor) < 0)) {
        cociente--;
        resto += divisor;
    }

    establecerValorOperando(cpu, &instr->op1, (uint32_t)cociente);
    cpu->regs[REG_AC] = resto;
    actualizarCC(cpu, (uint32_t)cociente);

    return 1;
}

uint32_t instr_cmp(CPU *cpu, Instruccion *instr) {
    int32_t val1 = (int32_t) obtenerValorOperando(cpu, &instr->op1);
    int32_t val2 = (int32_t) obtenerValorOperando(cpu, &instr->op2);
    int32_t resultado = val1 - val2;
    actualizarCC(cpu, (uint32_t) resultado);
    return 1;
}

uint32_t instr_shl(CPU *cpu, Instruccion *instr) {
    uint32_t val1 = obtenerValorOperando(cpu, &instr->op1);
    uint32_t val2 = obtenerValorOperando(cpu, &instr->op2);
    uint32_t resultado = val1 << val2;
    establecerValorOperando(cpu, &instr->op1, resultado);
    actualizarCC(cpu, resultado);
    return 1;
}

uint32_t instr_shr(CPU *cpu, Instruccion *instr) {
    uint32_t val1 = obtenerValorOperando(cpu, &instr->op1);
    uint32_t val2 = obtenerValorOperando(cpu, &instr->op2);
    uint32_t resultado = val1 >> val2;
    establecerValorOperando(cpu, &instr->op1, resultado);
    actualizarCC(cpu, resultado);
    return 1;
}

uint32_t instr_sar(CPU *cpu, Instruccion *instr) {
    int32_t val1 = (int32_t) obtenerValorOperando(cpu, &instr->op1);
    int32_t val2 = (int32_t) obtenerValorOperando(cpu, &instr->op2);
    int32_t resultado = val1 >> val2;
    establecerValorOperando(cpu, &instr->op1, (uint32_t) resultado);
    actualizarCC(cpu, (uint32_t) resultado);
    return 1;
}

uint32_t instr_and(CPU *cpu, Instruccion *instr) {
    uint32_t val1 = obtenerValorOperando(cpu, &instr->op1);
    uint32_t val2 = obtenerValorOperando(cpu, &instr->op2);
    uint32_t resultado = val1 & val2;
    establecerValorOperando(cpu, &instr->op1, resultado);
    actualizarCC(cpu, resultado);
    return 1;
}

uint32_t instr_or(CPU *cpu, Instruccion *instr) {
    uint32_t val1 = obtenerValorOperando(cpu, &instr->op1);
    uint32_t val2 = obtenerValorOperando(cpu, &instr->op2);
    uint32_t resultado = val1 | val2;
    establecerValorOperando(cpu, &instr->op1, resultado);
    actualizarCC(cpu, resultado);
    return 1;
}

uint32_t instr_xor(CPU *cpu, Instruccion *instr) {
    uint32_t val1 = obtenerValorOperando(cpu, &instr->op1);
    uint32_t val2 = obtenerValorOperando(cpu, &instr->op2);
    uint32_t resultado = val1 ^ val2;
    establecerValorOperando(cpu, &instr->op1, resultado);
    actualizarCC(cpu, resultado);
    return 1;
}

uint32_t instr_swap(CPU *cpu, Instruccion *instr) {
    uint32_t val1 = obtenerValorOperando(cpu, &instr->op1);
    uint32_t val2 = obtenerValorOperando(cpu, &instr->op2);
    establecerValorOperando(cpu, &instr->op1, val2);
    establecerValorOperando(cpu, &instr->op2, val1);
    return 1;
}

uint32_t instr_ldl(CPU *cpu, Instruccion *instr) {
    uint32_t val1 = obtenerValorOperando(cpu, &instr->op1);
    uint32_t val2 = obtenerValorOperando(cpu, &instr->op2);
    uint32_t resultado = (val1 & 0xFFFF0000) | (val2 & 0x0000FFFF);
    establecerValorOperando(cpu, &instr->op1, resultado);
    return 1;
}

uint32_t instr_ldh(CPU *cpu, Instruccion *instr) {
    uint32_t val1 = obtenerValorOperando(cpu, &instr->op1);
    uint32_t val2 = obtenerValorOperando(cpu, &instr->op2);
    uint32_t resultado = (val1 & 0x0000FFFF) | ((val2 & 0x0000FFFF) << 16);
    establecerValorOperando(cpu, &instr->op1, resultado);
    return 1;
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
    return 1;
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
    return 1;
}

uint32_t instr_jp(CPU *cpu, Instruccion *instr) {
    if (!(cpu->regs[REG_CC] & (CC_N_MASK | CC_Z_MASK))) {
        uint32_t direccion = obtenerValorOperando(cpu, &instr->op1);
        cpu->regs[REG_IP] = direccion;
        return 0;
    }
    return 1;
}

uint32_t instr_jn(CPU *cpu, Instruccion *instr) {
    if (cpu->regs[REG_CC] & CC_N_MASK) {
        uint32_t direccion = obtenerValorOperando(cpu, &instr->op1);
        cpu->regs[REG_IP] = direccion;
        return 0;
    }
    return 1;
}

uint32_t instr_jnz(CPU *cpu, Instruccion *instr) {
    if (!(cpu->regs[REG_CC] & CC_Z_MASK)) {
        uint32_t direccion = obtenerValorOperando(cpu, &instr->op1);
        cpu->regs[REG_IP] = direccion;
        return 0;
    }
    return 1;
}

uint32_t instr_jnp(CPU *cpu, Instruccion *instr) {
    if (cpu->regs[REG_CC] & (CC_N_MASK | CC_Z_MASK)) {
        uint32_t direccion = obtenerValorOperando(cpu, &instr->op1);
        cpu->regs[REG_IP] = direccion;
        return 0;
    }
    return 1;
}

uint32_t instr_jnn(CPU *cpu, Instruccion *instr) {
    if (!(cpu->regs[REG_CC] & CC_N_MASK)) {
        uint32_t direccion = obtenerValorOperando(cpu, &instr->op1);
        cpu->regs[REG_IP] = direccion;
        return 0;
    }
    return 1;
}


//
// Otros
//
uint32_t instr_not(CPU *cpu, Instruccion *instr) {
    uint32_t val = obtenerValorOperando(cpu, &instr->op1);
    uint32_t resultado = ~val;
    establecerValorOperando(cpu, &instr->op1, resultado);
    actualizarCC(cpu, (uint32_t) resultado);
    return 1;
}

uint32_t instr_stop(CPU *cpu, Instruccion *instr) {
    cpu->regs[REG_IP] = 0xFFFFFFFF;
    cpu->ejecutando = 0;
    return 0;
}

// ==== LLAMADAS AL SISTEMA ====

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
            fprintf(stderr, "Llamada al sistema no implementada: %d\n", syscall);
            break;
    }
    return 1;
}

/**
 * Lee datos del teclado y los almacena en memoria
 * EAX: modo de entrada (decimal, hex, octal, binario, char)
 * EDX: dirección de memoria donde almacenar
 * ECX: cantidad (bits bajos) y tamaño (bits altos)
 */
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

        if (modo & 0x10) {
            char binStr[33];
            printf("Ingrese un numero en binario: ");
            scanf("%32s", binStr);
            valor = strtol(binStr, NULL, 2);
        } else if (modo & 0x08) {
            printf("Ingrese un numero en hexadecimal: ");
            scanf("%x", (unsigned int *) &valor);
        } else if (modo & 0x04) {
            printf("Ingrese un numero en octal: ");
            scanf("%o", (unsigned int *) &valor);
        } else if (modo & 0x02) {
            char c;
            printf("Ingrese un caracter: ");
            scanf(" %c", &c);
            valor = (int32_t) c;
        } else if (modo & 0x01) {
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

/**
 * Muestra datos de memoria en pantalla
 * EAX: modo de salida (decimal, hex, octal, binario, char)
 * EDX: dirección de memoria a mostrar
 * ECX: cantidad (bits bajos) y tamaño (bits altos)
 */
void sysWrite(CPU *cpu) {
    uint32_t modo = cpu->regs[REG_EAX];
    uint32_t direccion = cpu->regs[REG_EDX];
    uint32_t config = cpu->regs[REG_ECX];
    uint16_t cantidad = config & 0xFFFF;
    uint16_t tamano = (config >> 16) & 0xFFFF;

    for (int i = 0; i < cantidad; i++) {
        uint32_t direccionLogica = direccion + (i * tamano);
        uint32_t direccionFisica = traducirDireccion(cpu, direccionLogica, tamano);
        int32_t valor = 0;

        if (tamano == 1) {
            valor =  leerMemoria8(cpu, direccionLogica);
        } else if (tamano == 2) {
            int16_t valor16_t = (int16_t) leerMemoria16(cpu, direccionLogica);
            valor = (int32_t) valor16_t;
        } else if (tamano == 4) {
            valor = (int32_t) leerMemoria32(cpu, direccionLogica);
        }

        printf("[%04X]: ", direccionFisica);

        if (modo & 0x10) {
            if (modo & 0x0F) printf(" ");
            printf("0b");
            for (int bit = 31; bit >= 0; bit--) {
                printf("%d", (valor >> bit) & 1);
            }
        }
        if (modo & 0x08) {
            if (modo & 0x07) printf(" ");
            printf("0x%X", (unsigned int) valor);
        }
        if (modo & 0x04) {
            if (modo & 0x03) printf(" ");
            printf("0o%o", (unsigned int) valor);
        }
        if (modo & 0x02) {
            if (modo & 0x01) printf(" ");
            printf(" '");
            for (int byte = (tamano - 1) * 8; byte >= 0; byte -= 8) {
                char c = (valor >> byte) & 0xFF;
                if (c >= 32 && c <= 126) {
                    printf("%c", c);
                } else {
                    printf(".");
                }
            }
            printf("'");
        }
        if (modo & 0x01) {
            printf(" %d", valor);
        }
        printf("\n");
    }
}