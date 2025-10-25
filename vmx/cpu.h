/**
 * cpu.h - Definiciones de arquitectura de la máquina virtual
 * 
 * Este archivo contiene todas las definiciones fundamentales de la arquitectura:
 * - Opcodes de instrucciones
 * - Registros de la CPU
 * - Estructuras de datos (CPU, segmentos, operandos)
 * - Constantes del sistema
 */

#pragma once

#include <stdint.h>

// ==== CONSTANTES DE ARQUITECTURA ====

#define MAX_SEGMENTOS 8              // Máximo de segmentos en la tabla de descriptores
#define CANTIDAD_INSTRUCCIONES 0x20  // Total de instrucciones soportadas (32)

// ==== OPCODES DE INSTRUCCIONES ====

// Control y syscalls
#define OP_SYS  0x00  // Llamada al sistema
#define OP_STOP 0x0F  // Detener ejecución

// Saltos condicionales
#define OP_JMP  0x01  // Salto incondicional
#define OP_JZ   0x02  // Saltar si Zero (resultado = 0)
#define OP_JP   0x03  // Saltar si Positive (resultado > 0)
#define OP_JN   0x04  // Saltar si Negative (resultado < 0)
#define OP_JNZ  0x05  // Saltar si Not Zero
#define OP_JNP  0x06  // Saltar si Not Positive
#define OP_JNN  0x07  // Saltar si Not Negative

// Pila y subrutinas
#define OP_PUSH 0x0B  // Empujar a pila
#define OP_POP  0x0C  // Sacar de pila
#define OP_CALL 0x0D  // Llamar subrutina
#define OP_RET  0x0E  // Retornar de subrutina

// Movimiento de datos
#define OP_MOV  0x10  // Mover datos
#define OP_SWAP 0x1C  // Intercambiar operandos
#define OP_LDL  0x1D  // Cargar Low (bits 0-15)
#define OP_LDH  0x1E  // Cargar High (bits 16-31)

// Operaciones aritméticas
#define OP_ADD  0x11  // Suma
#define OP_SUB  0x12  // Resta
#define OP_MUL  0x13  // Multiplicación
#define OP_DIV  0x14  // División (quotient en AC, remainder en RES9)
#define OP_CMP  0x15  // Comparación (actualiza CC)

// Operaciones lógicas y de bits
#define OP_NOT  0x08  // NOT bit a bit
#define OP_AND  0x19  // AND bit a bit
#define OP_OR   0x1A  // OR bit a bit
#define OP_XOR  0x1B  // XOR bit a bit
#define OP_SHL  0x16  // Shift Left lógico
#define OP_SHR  0x17  // Shift Right lógico
#define OP_SAR  0x18  // Shift Arithmetic Right (mantiene signo)

// Utilidades
#define OP_RND  0x1F  // Random (genera número aleatorio)

// ==== ÍNDICES DE SEGMENTOS ====

#define SEG_PS 0  // Param Segment (argc, argv)
#define SEG_KS 1  // Konstant Segment (constantes de solo lectura)
#define SEG_CS 2  // Code Segment (instrucciones)
#define SEG_DS 3  // Data Segment (datos inicializados)
#define SEG_ES 4  // Extra Segment (datos adicionales)
#define SEG_SS 5  // Stack Segment (pila)

// ==== ESTRUCTURAS DE CONFIGURACIÓN ====

/**
 * Layout de segmentos leído desde el archivo .vmx
 * 
 * Describe cómo está organizada la memoria del programa:
 * - Tamaños de cada segmento en bytes
 * - Punto de entrada (offset dentro de CS donde comienza la ejecución)
 * 
 * Los segmentos se cargan consecutivamente en memoria.
 * Segmentos con tamaño 0 no se crean.
 */
typedef struct {
    uint16_t tamanoPS;    // Tamaño del Param Segment
    uint16_t tamanoKS;    // Tamaño del Constant Segment
    uint16_t tamanoCS;    // Tamaño del Code Segment
    uint16_t tamanoDS;    // Tamaño del Data Segment
    uint16_t tamanoES;    // Tamaño del Extra Segment
    uint16_t tamanoSS;    // Tamaño del Stack Segment
    uint16_t entryPoint;  // Offset de inicio en CS
} LayoutSegmentos;

// ==== REGISTROS DE LA CPU ====

/**
 * Índices de los 32 registros de la CPU
 * 
 * Cada registro es de 32 bits (4 bytes).
 * Total: 128 bytes de registros.
 * 
 * Categorías:
 * - Registros de control de memoria (LAR, MAR, MBR)
 * - Registros de ejecución (IP, OPC, OP1, OP2)
 * - Registros de pila (SP, BP)
 * - Registros de propósito general (EAX-EFX)
 * - Registros de resultado (AC, CC)
 * - Registros de segmento (CS, DS, ES, SS, KS, PS)
 */
enum registros {
    // Registros de control de memoria
    REG_LAR = 0,   // Logical Address Register (última dirección lógica accedida)
    REG_MAR,       // Memory Address Register [16 bits tamaño | 16 bits dir_física]
    REG_MBR,       // Memory Buffer Register (último valor leído/escrito)
    
    // Registros de ejecución
    REG_IP,        // Instruction Pointer (dirección de próxima instrucción)
    REG_OPC,       // Opcode actual en ejecución
    REG_OP1,       // Operando 1 decodificado
    REG_OP2,       // Operando 2 decodificado
    
    // Registros de pila
    REG_SP,        // Stack Pointer (tope de la pila)
    REG_BP,        // Base Pointer (base del stack frame)
    REG_RES9,      // Reservado (usado para remainder en DIV)
    
    // Registros de propósito general
    REG_EAX,       // Accumulator (operaciones aritméticas)
    REG_EBX,       // Base
    REG_ECX,       // Counter
    REG_EDX,       // Data
    REG_EEX,       // Extended E
    REG_EFX,       // Extended F
    
    // Registros de resultado
    REG_AC,        // Accumulator (resultado de operaciones)
    REG_CC,        // Condition Codes (flags N y Z)
    
    // Registros reservados
    REG_RES18,
    REG_RES19,
    REG_RES20,
    REG_RES21,
    REG_RES22,
    REG_RES23,
    REG_RES24,
    REG_RES25,
    
    // Registros de segmento (contienen índice del segmento en bits altos)
    REG_CS,        // Code Segment register
    REG_DS,        // Data Segment register
    REG_ES,        // Extra Segment register
    REG_SS,        // Stack Segment register
    REG_KS,        // Konstant Segment register
    REG_PS         // Param Segment register
};

// ==== TIPOS DE OPERANDOS ====

#define TIPO_NINGUNO 0    // Sin operando
#define TIPO_REGISTRO 1   // Operando es un registro (ej: EAX)
#define TIPO_INMEDIATO 2  // Operando es un valor inmediato (ej: 0x1234)
#define TIPO_MEMORIA 3    // Operando es dirección de memoria (ej: [DS+10])

// ==== MÁSCARAS DE CONDITION CODES ====

#define CC_N_MASK 0x80000000  // Negative flag (bit 31 de CC)
#define CC_Z_MASK 0x40000000  // Zero flag (bit 30 de CC)

// ==== ESTRUCTURAS DE DATOS ====

/**
 * Descriptor de segmento en la tabla de descriptores
 * 
 * Cada segmento tiene:
 * - base: dirección física donde comienza en memoria
 * - tamano: longitud del segmento en bytes
 * 
 * La traducción de direcciones usa: dir_física = base + offset
 */
typedef struct {
    uint16_t base;    // Dirección física base
    uint16_t tamano;  // Tamaño en bytes
} Tsegmento;

/**
 * Estructura principal de la CPU
 * 
 * Contiene todo el estado de la máquina virtual:
 * - 32 registros de 32 bits cada uno
 * - Puntero a memoria principal (RAM)
 * - Tabla de descriptores de segmentos
 * - Estado de ejecución
 * - Archivo .vmi para breakpoints
 */
typedef struct {
    uint32_t regs[32];                   // Banco de registros
    uint8_t *mem;                        // Memoria principal (RAM)
    size_t tamMem;                       // Tamaño de memoria en bytes
    Tsegmento segmentos[MAX_SEGMENTOS];  // Tabla de descriptores de segmentos
    uint16_t cantSegmentos;              // Cantidad de segmentos activos
    int ejecutando;                      // Flag: 1=ejecutando, 0=detenido
    const char *vmiFile;                 // Archivo .vmi para guardar estado
} CPU;

/**
 * Operando de una instrucción
 * 
 * Un operando puede ser:
 * - TIPO_NINGUNO: no hay operando
 * - TIPO_REGISTRO: registro (ej: EAX, EBX)
 * - TIPO_INMEDIATO: valor constante (ej: 0x1234)
 * - TIPO_MEMORIA: dirección [registro + offset] (ej: [DS+10])
 * 
 * El campo 'ancho' indica el tamaño: 1, 2 o 4 bytes
 */
typedef struct {
    uint8_t tipo;   // TIPO_REGISTRO, TIPO_INMEDIATO, TIPO_MEMORIA, TIPO_NINGUNO
    uint8_t ancho;  // Tamaño: 1, 2 o 4 bytes
    union {
        int32_t valor;  // Para TIPO_INMEDIATO
        struct {
            uint8_t codReg;  // Código del registro
            uint8_t sector;  // Sector del registro (B, W, L, H)
        } registro;
        struct {
            int16_t offset;  // Desplazamiento (con signo)
            uint8_t codReg;  // Registro base
            uint8_t tam;     // Tamaño del acceso
        } memoria;
    } datos;
} Operando;

/**
 * Instrucción decodificada
 * 
 * Representa una instrucción completa después del fetch y decode:
 * - opcode: código de operación (OP_ADD, OP_MOV, etc.)
 * - op1, op2: operandos decodificados
 * - direccion: dirección en memoria donde se encontró la instrucción
 */
typedef struct {
    uint8_t opcode;      // Código de operación
    Operando op1, op2;   // Operandos
    uint16_t direccion;  // Dirección de la instrucción
} Instruccion;

/**
 * Tipo de función para implementar una instrucción
 * 
 * Cada instrucción se implementa como una función con esta firma.
 * Recibe el estado de la CPU y la instrucción decodificada.
 * 
 * @return 1 para continuar ejecución, 0 para detener
 */
typedef uint32_t (*FuncionInstruccion)(CPU *cpu, Instruccion *instr);
