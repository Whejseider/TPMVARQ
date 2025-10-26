#include <string.h>
#include "vmx.h"
#include "../instructions/instructions.h"
#include "../utils/utils.h"
#include "../memory/memory.h"
#include "../syscalls/syscalls.h"

// ==== TABLA DE DISPATCH DE INSTRUCCIONES ====

// Tabla que mapea cada opcode a su función implementadora
static FuncionInstruccion tablaInstrucciones[CANTIDAD_INSTRUCCIONES] = {0};

// Inicializa la tabla de dispatch de instrucciones
void inicializarTablaInstrucciones() {
    // Limpiar tabla
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

// ==== INICIALIZACIÓN DE LA CPU ====

// Implementación de inicialización de tabla de segmentos
void inicializaTablaSegmentos(CPU *cpu, LayoutSegmentos *layout) {
    uint32_t dirFis = 0;  // Dirección física actual en memoria
    cpu->cantSegmentos = 0;

    uint16_t tamanos[MAX_SEGMENTOS] = {
            layout->tamanoPS,
            layout->tamanoKS,
            layout->tamanoCS,
            layout->tamanoDS,
            layout->tamanoES,
            layout->tamanoSS
    };

    // Crear descriptores para cada segmento no vacío
    for (int i = 0; i < MAX_SEGMENTOS; ++i) {
        uint16_t tamano = tamanos[i];
        if (tamano == 0) {
            continue;
        }

        // Verificar que el segmento cabe en memoria
        if ((size_t)dirFis + tamano > cpu->tamMem) {
            terminarConError(VMX_ERROR_MEMORY_ACCESS, "Memoria insuficiente para segmentos");
        }

        // Crear descriptor con dirección base y tamaño
        cpu->segmentos[cpu->cantSegmentos].base = (uint16_t)dirFis;
        cpu->segmentos[cpu->cantSegmentos].tamano = tamano;
        dirFis += tamano;  // Avanzar a la siguiente posición
        cpu->cantSegmentos++;
    }
}

// Implementación de inicialización de registros
void inicializarRegistros(CPU *cpu, LayoutSegmentos *layout, uint16_t argc, uint32_t argvPtr) {
    // Limpiar todos los registros
    memset(cpu->regs, 0, sizeof(cpu->regs));

    // Mapear cada tipo de segmento a su índice en la tabla de descriptores
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

    // Asignar índices a cada segmento que tiene tamaño > 0
    for (uint16_t idx = 0, seg = 0; idx < cpu->cantSegmentos && seg < MAX_SEGMENTOS; ++seg) {
        if (tamanos[seg] == 0) {
            continue;
        }
        indicesSegmento[seg] = idx;
        idx++;
    }

    // Configurar registros de segmento (bits altos = índice, bits bajos = offset)
    // 0xFFFFFFFF indica segmento no existente
    cpu->regs[REG_PS] = (indicesSegmento[SEG_PS] == (uint16_t)-1) ? 0xFFFFFFFF : (indicesSegmento[SEG_PS] << 16);
    cpu->regs[REG_KS] = (indicesSegmento[SEG_KS] == (uint16_t)-1) ? 0xFFFFFFFF : (indicesSegmento[SEG_KS] << 16);
    cpu->regs[REG_CS] = (indicesSegmento[SEG_CS] == (uint16_t)-1) ? 0xFFFFFFFF : (indicesSegmento[SEG_CS] << 16) | layout->entryPoint;
    cpu->regs[REG_DS] = (indicesSegmento[SEG_DS] == (uint16_t)-1) ? 0xFFFFFFFF : (indicesSegmento[SEG_DS] << 16);
    cpu->regs[REG_ES] = (indicesSegmento[SEG_ES] == (uint16_t)-1) ? 0xFFFFFFFF : (indicesSegmento[SEG_ES] << 16);
    cpu->regs[REG_SS] = (indicesSegmento[SEG_SS] == (uint16_t)-1) ? 0xFFFFFFFF : (indicesSegmento[SEG_SS] << 16);

    // IP apunta al entry point en CS
    cpu->regs[REG_IP] = cpu->regs[REG_CS];

    if (indicesSegmento[SEG_SS] == (uint16_t)-1) {
        cpu->regs[REG_SP] = 0xFFFFFFFF;
    } else {
        uint16_t tamSS = cpu->segmentos[indicesSegmento[SEG_SS]].tamano;
        cpu->regs[REG_SP] = ((uint32_t)indicesSegmento[SEG_SS] << 16) | tamSS;
    }

    cpu->ejecutando = 1;

    if (indicesSegmento[SEG_SS] != (uint16_t)-1) {
        uint32_t tope = cpu->segmentos[indicesSegmento[SEG_SS]].tamano;

        // Empujar dirección de retorno (0xFFFFFFFF = fin de programa)
        tope -= 4;
        escribirMemoria32(cpu, ((uint32_t)indicesSegmento[SEG_SS] << 16) | tope, 0xFFFFFFFF);

        // Empujar argc (cantidad de parámetros)
        tope -= 4;
        escribirMemoria32(cpu, ((uint32_t)indicesSegmento[SEG_SS] << 16) | tope, argc);

        // Empujar argv (puntero al array de parámetros en PS)
        tope -= 4;
        escribirMemoria32(cpu, ((uint32_t)indicesSegmento[SEG_SS] << 16) | tope, argvPtr);

        cpu->regs[REG_SP] = ((uint32_t)indicesSegmento[SEG_SS] << 16) | tope;
    }
}

// ==== CICLO DE EJECUCIÓN ====

// Implementación del ciclo fetch-decode-execute
void vmxRun(CPU *cpu) {
    Instruccion instr;      // Instrucción actual decodificada
    uint32_t tamanoInstr;   // Bytes que ocupa la instrucción
    int pasoAPaso = 0;      // Flag para modo debugging paso a paso

    // Inicializar tabla de dispatch
    inicializarTablaInstrucciones();

    // === FETCH-DECODE-EXECUTE LOOP ===
    while (cpu->ejecutando) {
        // === FETCH: Verificar que IP está en el segmento CS correcto ===
        uint16_t segmentoIp = (cpu->regs[REG_IP] >> 16) & 0xFFFF;
        uint16_t segmentoCs = (cpu->regs[REG_CS] >> 16) & 0xFFFF;

        // Si CS no está inicializado correctamente -> error
        if (segmentoCs >= cpu->cantSegmentos) {
            mostrarError("Segmento CS inválido.");
            break;
        }

        // IP debe apuntar al mismo selector que CS
        if (segmentoIp != segmentoCs) {
            mostrarError("IP fuera del segmento de código.");
            break;
        }

        // Verificar que offset no excede el tamaño del segmento CS
        uint16_t offsetIp = cpu->regs[REG_IP] & 0xFFFF;
        uint16_t tamanoCS = cpu->segmentos[segmentoCs].tamano;
        if (offsetIp >= tamanoCS) {
            cpu->ejecutando = 0;
            break;
        }

        // Leer y decodificar instrucción desde memoria
        tamanoInstr = leerInstruccion(cpu, cpu->regs[REG_IP], &instr);

        // === DECODE: Actualizar registros OPC, OP1, OP2 ===
        cpu->regs[REG_OPC] = instr.opcode;

        // Codificar operando 1 en REG_OP1
        if (instr.op1.tipo == TIPO_MEMORIA) {
            int32_t offsetSigno = instr.op1.datos.memoria.offset;
            uint32_t valor = ((uint32_t)instr.op1.datos.memoria.codReg << 16) |
                             ((uint32_t)offsetSigno & 0x0000FFFF);
            cpu->regs[REG_OP1] = (instr.op1.tipo << 24) | (valor & 0x00FFFFFF);
        } else if (instr.op1.tipo == TIPO_REGISTRO) {
            cpu->regs[REG_OP1] = (instr.op1.tipo << 24) | instr.op1.datos.registro.codReg;
        } else {
            int32_t valorConSigno = instr.op1.datos.valor;
            cpu->regs[REG_OP1] = (instr.op1.tipo << 24) | ((uint32_t)valorConSigno & 0x00FFFFFF);
        }

        // Codificar operando 2 en REG_OP2
        if (instr.op2.tipo == TIPO_MEMORIA) {
            int32_t offsetSigno = instr.op2.datos.memoria.offset;
            uint32_t valor = ((uint32_t)instr.op2.datos.memoria.codReg << 16) |
                             ((uint32_t)offsetSigno & 0x0000FFFF);
            cpu->regs[REG_OP2] = (instr.op2.tipo << 24) | (valor & 0x00FFFFFF);
        } else if (instr.op2.tipo == TIPO_REGISTRO) {
            cpu->regs[REG_OP2] = (instr.op2.tipo << 24) | instr.op2.datos.registro.codReg;
        } else {
            int32_t valorConSigno = instr.op2.datos.valor;
            cpu->regs[REG_OP2] = (instr.op2.tipo << 24) | ((uint32_t)valorConSigno & 0x00FFFFFF);
        }

        // Avanzar IP a la siguiente instrucción
        cpu->regs[REG_IP] += tamanoInstr;

        // === EXECUTE: Ejecutar instrucción ===
        if (tablaInstrucciones[instr.opcode] != NULL) {
            // Dispatch: llamar a la función que implementa esta instrucción
            uint32_t resultado = tablaInstrucciones[instr.opcode](cpu, &instr);
            
            // === MANEJO DE BREAKPOINT Y PASO A PASO ===
            // resultado == 3: activar modo paso a paso
            // resultado == 0: instrucción de salto o control
            // resultado == 1: instrucción normal
            
            if (resultado == 3) {
                // Syscall breakpoint: activar modo paso a paso
                pasoAPaso = 1;
            } else if (pasoAPaso && cpu->vmiFile) {
                // En modo paso a paso: pausar después de cada instrucción
                pasoAPaso = 0;
                uint32_t accion = sysBreakpoint(cpu);
                
                if (accion == 2) {
                    // Usuario presionó 'q': quit
                    cpu->ejecutando = 0;
                    break;
                } else if (accion == 1) {
                    // Usuario presionó Enter: continuar paso a paso
                    pasoAPaso = 1;
                }
                // accion == 0: Usuario presionó 'g': continuar normalmente
            }
        } else {
            terminarConError(VMX_ERROR_INVALID_INSTRUCTION, NULL);
        }
    }
}

// ==== ACTUALIZACIÓN DE CONDITION CODES ====

// Implementación de actualización de CC
void actualizarCC(CPU *cpu, uint32_t resultado) {
    // Limpiar todos los flags
    cpu->regs[REG_CC] = 0;
    
    // Activar flag Z si el resultado es cero
    if ((resultado & 0xFFFFFFFF) == 0) cpu->regs[REG_CC] |= CC_Z_MASK;
    
    // Flag N: activar si resultado < 0 (signed)
    if ((int32_t)resultado < 0) cpu->regs[REG_CC] |= CC_N_MASK;
}