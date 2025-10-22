#include <string.h>
#include "vmx.h"
#include "../instructions/instructions.h"

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
void inicializaTablaSegmentos(CPU *cpu, uint16_t tamanoCodigo) {
    cpu->segmentos[0].base = 0;
    cpu->segmentos[0].tamano = tamanoCodigo;

    cpu->segmentos[1].base = tamanoCodigo;
    cpu->segmentos[1].tamano = RAM - tamanoCodigo;
}

/**
 * Inicializa los registros de la CPU con valores por defecto
 * Configura CS, DS e IP para comenzar la ejecución
 */
void inicializarRegistros(CPU *cpu) {
    memset(cpu->regs, 0, sizeof(cpu->regs));

    cpu->regs[REG_CS] = 0x00000000;
    cpu->regs[REG_DS] = 0x00010000;
    cpu->regs[REG_IP] = cpu->regs[REG_CS];

    cpu->ejecutando = 1;
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
            mostrarError("Instrucción inválida");
            exit(1);
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

/**
 * Muestra un mensaje de error en stderr
 */
void mostrarError(const char *mensaje) {
    fprintf(stderr, "Error: %s\n", mensaje);
}