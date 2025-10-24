#include <stdlib.h>
#include <time.h>
#include "instructions.h"
#include "../memory/memory.h"
#include "../utils/utils.h"
#include "../vmx/vmx.h"
#include "../syscalls/syscalls.h"

/**
 * Decodifica una instrucción desde memoria
 * Lee el opcode y operandos según el formato de instrucción VMX
 * 
 * @param cpu estructura de la CPU
 * @param direccion dirección donde está la instrucción
 * @param instr estructura donde guardar la instrucción decodificada
 * @return tamaño en bytes de la instrucción
 */
static uint8_t anchoDesdeSector(uint8_t sector) {
    switch (sector) {
        case 0: return 4; // registro completo
        case 1: return 1; // byte bajo
        case 2: return 1; // byte alto
        case 3: return 2; // palabra
        default: return 4;
    }
}

static uint8_t bytesDesdeCodigo(uint8_t codigo) {
    switch (codigo & 0x03) {
        case 0x00: return 4; // long
        case 0x02: return 2; // word
        case 0x03: return 1; // byte
        default: return 4;
    }
}

static uint8_t leerByteInstr(CPU *cpu, uint32_t direccionLogica) {
    uint32_t direccionFisica = traducirDireccion(cpu, direccionLogica, 1);
    return cpu->mem[direccionFisica];
}

static uint16_t leerWordInstr(CPU *cpu, uint32_t direccionLogica) {
    uint32_t direccionFisica = traducirDireccion(cpu, direccionLogica, 2);
    uint16_t alto = cpu->mem[direccionFisica];
    uint16_t bajo = cpu->mem[direccionFisica + 1];
    return (uint16_t)((alto << 8) | bajo);
}

static int esRegistroSegmento(uint8_t reg) {
    return reg == REG_CS || reg == REG_DS || reg == REG_ES || reg == REG_SS || reg == REG_KS || reg == REG_PS;
}

static uint32_t obtenerIndiceSegmentoPila(CPU *cpu) {
    if (cpu->regs[REG_SP] == 0xFFFFFFFF) {
        terminarConError(VMX_ERROR_STACK_UNDERFLOW, "stack no inicializada");
    }
    return (cpu->regs[REG_SP] >> 16) & 0xFFFF;
}

static uint16_t obtenerTamanioSegmentoPila(CPU *cpu, uint32_t indice) {
    if (indice >= cpu->cantSegmentos) {
        terminarConError(VMX_ERROR_STACK_UNDERFLOW, "segmento de pila inválido");
    }
    return cpu->segmentos[indice].tamano;
}

static uint16_t obtenerOffsetPila(CPU *cpu) {
    return (uint16_t)(cpu->regs[REG_SP] & 0xFFFF);
}

static void actualizarSP(CPU *cpu, uint32_t indice, uint16_t offset) {
    cpu->regs[REG_SP] = ((uint32_t) indice << 16) | offset;
}

static void stackPush32(CPU *cpu, uint32_t valor) {
    uint32_t indice = obtenerIndiceSegmentoPila(cpu);
    uint16_t tamSeg = obtenerTamanioSegmentoPila(cpu, indice);
    uint16_t offset = obtenerOffsetPila(cpu);

    if (offset < 4 || tamSeg < 4) {
        terminarConError(VMX_ERROR_STACK_OVERFLOW, "stack overflow");
    }

    offset -= 4;
    if (offset >= tamSeg) {
        terminarConError(VMX_ERROR_STACK_OVERFLOW, "stack overflow");
    }

    uint32_t direccion = ((uint32_t) indice << 16) | offset;
    escribirMemoria32(cpu, direccion, valor);
    actualizarSP(cpu, indice, offset);
}

static uint32_t stackPop32(CPU *cpu) {
    uint32_t indice = obtenerIndiceSegmentoPila(cpu);
    uint16_t tamSeg = obtenerTamanioSegmentoPila(cpu, indice);
    uint16_t offset = obtenerOffsetPila(cpu);

    if (offset >= tamSeg) {
        terminarConError(VMX_ERROR_STACK_UNDERFLOW, "stack underflow");
    }

    uint32_t direccion = ((uint32_t) indice << 16) | offset;
    uint32_t valor = leerMemoria32(cpu, direccion);

    offset += 4;
    if (offset > tamSeg) {
        offset = tamSeg;
    }

    actualizarSP(cpu, indice, offset);
    return valor;
}

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
    instr->op2.ancho = 4;
    if (tipoOp2 == TIPO_REGISTRO) {
        uint8_t descriptor = leerByteInstr(cpu, direccion + pos);
        instr->op2.datos.registro.codReg = descriptor & 0x1F;
        instr->op2.datos.registro.sector = (descriptor >> 6) & 0x03;
        instr->op2.ancho = anchoDesdeSector(instr->op2.datos.registro.sector);
        pos += 1;
    } else if (tipoOp2 == TIPO_INMEDIATO) {
        int16_t valor16 = (int16_t) leerWordInstr(cpu, direccion + pos);
        instr->op2.datos.valor = (int32_t) valor16;
        instr->op2.ancho = 2;
        pos += 2;
    } else if (tipoOp2 == TIPO_MEMORIA) {
        uint32_t dirLog = direccion + pos;
        uint8_t descriptor = leerByteInstr(cpu, dirLog);
        instr->op2.datos.memoria.codReg = descriptor & 0x1F;
        instr->op2.datos.memoria.tam = (descriptor >> 6) & 0x03;
        instr->op2.ancho = bytesDesdeCodigo(instr->op2.datos.memoria.tam);
        int16_t offset16 = (int16_t) leerWordInstr(cpu, dirLog + 1);
        instr->op2.datos.memoria.offset = offset16;
        pos += 3;
    } else {
        instr->op2.datos.valor = 0;
    }

    instr->op1.tipo = tipoOp1;
    instr->op1.ancho = 4;
    if (tipoOp1 == TIPO_REGISTRO) {
        uint8_t descriptor = leerByteInstr(cpu, direccion + pos);
        instr->op1.datos.registro.codReg = descriptor & 0x1F;
        instr->op1.datos.registro.sector = (descriptor >> 6) & 0x03;
        instr->op1.ancho = anchoDesdeSector(instr->op1.datos.registro.sector);
        pos += 1;
    } else if (tipoOp1 == TIPO_INMEDIATO) {
        int16_t valor16 = (int16_t) leerWordInstr(cpu, direccion + pos);
        instr->op1.datos.valor = (int32_t) valor16;
        instr->op1.ancho = 2;
        pos += 2;
    } else if (tipoOp1 == TIPO_MEMORIA) {
        uint32_t dirLog = direccion + pos;
        uint8_t descriptor = leerByteInstr(cpu, dirLog);
        instr->op1.datos.memoria.codReg = descriptor & 0x1F;
        instr->op1.datos.memoria.tam = (descriptor >> 6) & 0x03;
        instr->op1.ancho = bytesDesdeCodigo(instr->op1.datos.memoria.tam);
        int16_t offset16 = (int16_t) leerWordInstr(cpu, dirLog + 1);
        instr->op1.datos.memoria.offset = offset16;
        pos += 3;
    } else {
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
        case TIPO_REGISTRO: {
            uint8_t reg = op->datos.registro.codReg;
            uint8_t sector = op->datos.registro.sector;
            uint32_t valor = cpu->regs[reg];
            switch (sector) {
                case 0:
                    return valor;
                case 1:
                    return (uint32_t)(int8_t)(valor & 0xFF);
                case 2:
                    return (uint32_t)(int8_t)((valor >> 8) & 0xFF);
                case 3:
                    return (uint32_t)(int16_t)(valor & 0xFFFF);
                default:
                    return valor;
            }
        }
        case TIPO_INMEDIATO:
            return (uint32_t) op->datos.valor;
        case TIPO_MEMORIA: {
            uint8_t reg = op->datos.memoria.codReg;
            int16_t desplazamiento = op->datos.memoria.offset;
            uint32_t base = (reg == 0) ? cpu->regs[REG_DS] : cpu->regs[reg];
            uint16_t segmento = (base >> 16) & 0xFFFF;
            int32_t offset = (int16_t)(base & 0xFFFF) + desplazamiento;
            uint32_t dirLog = ((uint32_t)segmento << 16) | ((uint32_t)offset & 0xFFFF);
            uint8_t bytes = bytesDesdeCodigo(op->datos.memoria.tam);
            if (bytes == 1) {
                return (uint32_t)(int8_t) leerMemoria8(cpu, dirLog);
            } else if (bytes == 2) {
                return (uint32_t)(int16_t) leerMemoria16(cpu, dirLog);
            } else {
                return leerMemoria32(cpu, dirLog);
            }
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
        case TIPO_REGISTRO: {
            uint8_t reg = op->datos.registro.codReg;
            uint8_t sector = op->datos.registro.sector;
            uint32_t original = cpu->regs[reg];
            switch (sector) {
                case 0:
                    cpu->regs[reg] = valor;
                    break;
                case 1:
                    cpu->regs[reg] = (original & 0xFFFFFF00u) | (valor & 0xFFu);
                    break;
                case 2:
                    cpu->regs[reg] = (original & 0xFFFF00FFu) | ((valor & 0xFFu) << 8);
                    break;
                case 3:
                    cpu->regs[reg] = (original & 0xFFFF0000u) | (valor & 0xFFFFu);
                    break;
                default:
                    cpu->regs[reg] = valor;
                    break;
            }
            break;
        }
        case TIPO_MEMORIA: {
            uint8_t reg = op->datos.memoria.codReg;
            int16_t desplazamiento = op->datos.memoria.offset;
            uint32_t base = (reg == 0) ? cpu->regs[REG_DS] : cpu->regs[reg];
            uint16_t segmento = (base >> 16) & 0xFFFF;
            int32_t offset = (int16_t)(base & 0xFFFF) + desplazamiento;
            uint32_t dirLog = ((uint32_t)segmento << 16) | ((uint32_t)offset & 0xFFFF);
            uint8_t bytes = bytesDesdeCodigo(op->datos.memoria.tam);
            if (bytes == 1) {
                escribirMemoria8(cpu, dirLog, (uint8_t) (valor & 0xFFu));
            } else if (bytes == 2) {
                escribirMemoria16(cpu, dirLog, (uint16_t) (valor & 0xFFFFu));
            } else {
                escribirMemoria32(cpu, dirLog, valor);
            }
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
        terminarConError(VMX_ERROR_DIVISION_BY_ZERO, NULL);
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

static uint32_t fetchOperandValue(CPU *cpu, Operando *op) {
    return obtenerValorOperando(cpu, op);
}

static void writeOperandValue(CPU *cpu, Operando *op, uint32_t valor) {
    establecerValorOperando(cpu, op, valor);
}

uint32_t instr_push(CPU *cpu, Instruccion *instr) {
    uint32_t valor = fetchOperandValue(cpu, &instr->op1);
    stackPush32(cpu, valor);
    return 0;
}

uint32_t instr_pop(CPU *cpu, Instruccion *instr) {
    uint32_t valor = stackPop32(cpu);
    writeOperandValue(cpu, &instr->op1, valor);
    return 0;
}

uint32_t instr_call(CPU *cpu, Instruccion *instr) {
    uint32_t direccionRetorno = cpu->regs[REG_IP];
    stackPush32(cpu, direccionRetorno);
    uint32_t destino = fetchOperandValue(cpu, &instr->op1);
    cpu->regs[REG_IP] = destino;
    return 0;
}

uint32_t instr_ret(CPU *cpu, Instruccion *instr) {
    uint32_t direccion = stackPop32(cpu);
    cpu->regs[REG_IP] = direccion;
    return 0;
}

// ==== LLAMADAS AL SISTEMA ====

uint32_t instr_sys(CPU *cpu, Instruccion *instr) {
    uint32_t syscall = obtenerValorOperando(cpu, &instr->op1);
    return ejecutarSyscall(cpu, syscall);
}