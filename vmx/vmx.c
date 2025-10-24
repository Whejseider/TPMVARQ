#include <string.h>
#include "vmx.h"
#include "../instructions/instructions.h"
#include "../utils/utils.h"
#include "../memory/memory.h"

static FuncionInstruccion tablaInstrucciones[CANTIDAD_INSTRUCCIONES] = {0};

/**
 * Inicializa la tabla de instrucciones con punteros a funciones
 * Mapea cada código de operación a su función correspondiente
 */
void inicializarTablaInstrucciones() {
    memset(tablaInstrucciones, 0, sizeof(tablaInstrucciones));

    tablaInstrucciones[OP_SYS] = instr_sys;
    tablaInstrucciones[OP_JMP] = instr_jmp;
    tablaInstrucciones[OP_JZ] = instr_jz;
    tablaInstrucciones[OP_JP] = instr_jp;
    tablaInstrucciones[OP_JN] = instr_jn;
    tablaInstrucciones[OP_JNZ] = instr_jnz;
    tablaInstrucciones[OP_JNP] = instr_jnp;
    tablaInstrucciones[OP_JNN] = instr_jnn;
    tablaInstrucciones[OP_NOT] = instr_not;
    tablaInstrucciones[OP_PUSH] = instr_push;
    tablaInstrucciones[OP_POP] = instr_pop;
    tablaInstrucciones[OP_CALL] = instr_call;
    tablaInstrucciones[OP_RET] = instr_ret;
    tablaInstrucciones[OP_STOP] = instr_stop;
    tablaInstrucciones[OP_MOV] = instr_mov;
    tablaInstrucciones[OP_ADD] = instr_add;
    tablaInstrucciones[OP_SUB] = instr_sub;
    tablaInstrucciones[OP_MUL] = instr_mul;
    tablaInstrucciones[OP_DIV] = instr_div;
    tablaInstrucciones[OP_CMP] = instr_cmp;
    tablaInstrucciones[OP_SHL] = instr_shl;
    tablaInstrucciones[OP_SHR] = instr_shr;
    tablaInstrucciones[OP_SAR] = instr_sar;
    tablaInstrucciones[OP_AND] = instr_and;
    tablaInstrucciones[OP_OR] = instr_or;
    tablaInstrucciones[OP_XOR] = instr_xor;
    tablaInstrucciones[OP_SWAP] = instr_swap;
    tablaInstrucciones[OP_LDL] = instr_ldl;
    tablaInstrucciones[OP_LDH] = instr_ldh;
    tablaInstrucciones[OP_RND] = instr_rnd;
}

/**
 * Configura la tabla de segmentos de memoria
 * CS: segmento de código (0)
 * DS: segmento de datos (1)
 */
void inicializaTablaSegmentos(CPU *cpu, LayoutSegmentos *layout) {
    uint32_t cursor = 0;
    cpu->cantSegmentos = 0;

    uint16_t tamanos[MAX_SEGMENTOS] = {
            layout->tamanoPS,
            layout->tamanoKS,
            layout->tamanoCS,
            layout->tamanoDS,
            layout->tamanoES,
            layout->tamanoSS
    };

    for (int i = 0; i < MAX_SEGMENTOS; ++i) {
        uint16_t tamano = tamanos[i];
        if (tamano == 0) {
            continue;
        }

        if ((size_t)cursor + tamano > cpu->tamMem) {
            terminarConError(VMX_ERROR_MEMORY_ACCESS, "Memoria insuficiente para segmentos");
        }

        cpu->segmentos[cpu->cantSegmentos].base = (uint16_t)cursor;
        cpu->segmentos[cpu->cantSegmentos].tamano = tamano;
        cursor += tamano;
        cpu->cantSegmentos++;
    }
}

/**
 * Inicializa los registros de la CPU con valores por defecto
 * Configura CS, DS e IP para comenzar la ejecución
 */
void inicializarRegistros(CPU *cpu, LayoutSegmentos *layout, uint16_t argc, uint32_t argvPtr) {
    memset(cpu->regs, 0, sizeof(cpu->regs));

    uint16_t indicesSegmento[MAX_SEGMENTOS];
    for (int i = 0; i < MAX_SEGMENTOS; ++i) {
        indicesSegmento[i] = (uint16_t)-1;
    }

    uint16_t tamanos[MAX_SEGMENTOS] = {
            layout->tamanoPS,
            layout->tamanoKS,
            layout->tamanoCS,
            layout->tamanoDS,
            layout->tamanoES,
            layout->tamanoSS
    };

    for (uint16_t idx = 0, seg = 0; idx < cpu->cantSegmentos && seg < MAX_SEGMENTOS; ++seg) {
        if (tamanos[seg] == 0) {
            continue;
        }
        indicesSegmento[seg] = idx;
        idx++;
    }

    cpu->regs[REG_PS] = (indicesSegmento[SEG_PS] == (uint16_t)-1) ? 0xFFFFFFFF : (indicesSegmento[SEG_PS] << 16);
    cpu->regs[REG_KS] = (indicesSegmento[SEG_KS] == (uint16_t)-1) ? 0xFFFFFFFF : (indicesSegmento[SEG_KS] << 16);
    cpu->regs[REG_CS] = (indicesSegmento[SEG_CS] == (uint16_t)-1) ? 0xFFFFFFFF : (indicesSegmento[SEG_CS] << 16) | layout->entryPoint;
    cpu->regs[REG_DS] = (indicesSegmento[SEG_DS] == (uint16_t)-1) ? 0xFFFFFFFF : (indicesSegmento[SEG_DS] << 16);
    cpu->regs[REG_ES] = (indicesSegmento[SEG_ES] == (uint16_t)-1) ? 0xFFFFFFFF : (indicesSegmento[SEG_ES] << 16);
    cpu->regs[REG_SS] = (indicesSegmento[SEG_SS] == (uint16_t)-1) ? 0xFFFFFFFF : (indicesSegmento[SEG_SS] << 16);

    cpu->regs[REG_IP] = cpu->regs[REG_CS];

    if (indicesSegmento[SEG_SS] == (uint16_t)-1) {
        cpu->regs[REG_SP] = 0xFFFFFFFF;
    } else {
        uint16_t baseSS = cpu->segmentos[indicesSegmento[SEG_SS]].base;
        uint16_t tamSS = cpu->segmentos[indicesSegmento[SEG_SS]].tamano;
        cpu->regs[REG_SP] = ((uint32_t)indicesSegmento[SEG_SS] << 16) | (baseSS + tamSS);
    }

    cpu->ejecutando = 1;

    if (indicesSegmento[SEG_SS] != (uint16_t)-1) {
        uint16_t baseSS = cpu->segmentos[indicesSegmento[SEG_SS]].base;
        uint32_t tope = baseSS + cpu->segmentos[indicesSegmento[SEG_SS]].tamano;

        tope -= 4;
        escribirMemoria32(cpu, ((uint32_t)indicesSegmento[SEG_SS] << 16) | tope, 0xFFFFFFFF);
        tope -= 4;
        escribirMemoria32(cpu, ((uint32_t)indicesSegmento[SEG_SS] << 16) | tope, argc);
        tope -= 4;
        escribirMemoria32(cpu, ((uint32_t)indicesSegmento[SEG_SS] << 16) | tope, argvPtr);

        cpu->regs[REG_SP] = ((uint32_t)indicesSegmento[SEG_SS] << 16) | tope;
    }
}

/**
 * Ciclo principal de ejecución de la máquina virtual
 * Lee instrucciones, las decodifica y las ejecuta hasta encontrar STOP
 */
void vmxRun(CPU *cpu) {
    Instruccion instr;
    uint32_t tamanoInstr;

    inicializarTablaInstrucciones();

    while (cpu->ejecutando) {
        uint16_t segmentoIp = (cpu->regs[REG_IP] >> 16) & 0xFFFF;
        if (segmentoIp != 0) {
            mostrarError("IP fuera del segmento de código.");
            break;
        }

        uint16_t offsetIp = cpu->regs[REG_IP] & 0xFFFF;
        if (offsetIp >= cpu->segmentos[0].tamano) {
            cpu->ejecutando = 0;
            break;
        }

        tamanoInstr = leerInstruccion(cpu, cpu->regs[REG_IP], &instr);

        cpu->regs[REG_OPC] = instr.opcode;

        if (instr.op1.tipo == TIPO_MEMORIA) {
            int32_t offsetSigno = instr.op1.datos.memoria.offset;
            uint32_t valor = ((uint32_t)instr.op1.datos.memoria.codReg << 16) |
                             ((uint32_t)offsetSigno & 0x0000FFFF);
            cpu->regs[REG_OP1] = (instr.op1.tipo << 24) | (valor & 0x00FFFFFF);
        } else {
            int32_t valorConSigno = instr.op1.datos.valor;
            cpu->regs[REG_OP1] = (instr.op1.tipo << 24) | ((uint32_t)valorConSigno & 0x00FFFFFF);
        }

        if (instr.op2.tipo == TIPO_MEMORIA) {
            int32_t offsetSigno = instr.op2.datos.memoria.offset;
            uint32_t valor = ((uint32_t)instr.op2.datos.memoria.codReg << 16) |
                             ((uint32_t)offsetSigno & 0x0000FFFF);
            cpu->regs[REG_OP2] = (instr.op2.tipo << 24) | (valor & 0x00FFFFFF);
        } else {
            int32_t valorConSigno = instr.op2.datos.valor;
            cpu->regs[REG_OP2] = (instr.op2.tipo << 24) | ((uint32_t)valorConSigno & 0x00FFFFFF);
        }

        cpu->regs[REG_IP] += tamanoInstr;

        if (tablaInstrucciones[instr.opcode] != NULL) {
            uint32_t resultado = tablaInstrucciones[instr.opcode](cpu, &instr);
        } else {
            terminarConError(VMX_ERROR_INVALID_INSTRUCTION, NULL);
        }
    }
}

/**
 * Actualiza el registro de condición (CC) basado en el resultado de una operación
 * Establece flags de cero (Z) y negativo (N)
 */
void actualizarCC(CPU *cpu, uint32_t resultado) {
    cpu->regs[REG_CC] = 0;
    if ((resultado & 0xFFFFFFFF) == 0) cpu->regs[REG_CC] |= CC_Z_MASK;
    if ((int32_t)resultado < 0) cpu->regs[REG_CC] |= CC_N_MASK;
}