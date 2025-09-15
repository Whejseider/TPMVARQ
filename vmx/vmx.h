/**
 * El Corazón de la máquina virtual
 * Se guardan todas las estructuras que utiliza y métodos principales para su inicialización y ejecución
 */

#pragma once

#include "stdint.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_SEGMENTOS 2
#define RAM 16384
#define CANTIDAD_INSTRUCCIONES 0x20

enum registros {
    REG_LAR = 0,
    REG_MAR,
    REG_MBR,
    REG_IP,
    REG_OPC,
    REG_OP1,
    REG_OP2,
    REG_RES7,
    REG_RES8,
    REG_RES9,
    REG_EAX,
    REG_EBX,
    REG_ECX,
    REG_EDX,
    REG_EEX,
    REG_EFX,
    REG_AC,
    REG_CC,
    REG_RES18,
    REG_RES19,
    REG_RES20,
    REG_RES21,
    REG_RES22,
    REG_RES23,
    REG_RES24,
    REG_RES25,
    REG_CS,
    REG_DS,
    REG_RES28,
    REG_RES29,
    REG_RES30,
    REG_RES31
};

/**
 * Operandos de la máquina virtual
 */
#define TIPO_NINGUNO 0
#define TIPO_REGISTRO 1
#define TIPO_INMEDIATO 2
#define TIPO_MEMORIA 3

/**
 * Códigos de operación
 * 26 operaciones (primera parte)
 */
#define OP_SYS  0x00
#define OP_JMP  0x01
#define OP_JZ   0x02
#define OP_JP   0x03
#define OP_JN   0x04
#define OP_JNZ  0x05
#define OP_JNP  0x06
#define OP_JNN  0x07
#define OP_NOT  0x08
#define OP_STOP 0x0F
#define OP_MOV  0x10
#define OP_ADD  0x11
#define OP_SUB  0x12
#define OP_MUL  0x13
#define OP_DIV  0x14
#define OP_CMP  0x15
#define OP_SHL  0x16
#define OP_SHR  0x17
#define OP_SAR  0x18
#define OP_AND  0x19
#define OP_OR   0x1A
#define OP_XOR  0x1B
#define OP_SWAP 0x1C
#define OP_LDL  0x1D
#define OP_LDH  0x1E
#define OP_RND  0x1F

/**
 * Máscaras para el Registro CC
 * CC_N_MASK - El bit más significativo es el indicador de
signo (bit N), que valdrá 1 cuando la última operación matemática o lógica haya dado por resultado un
valor negativo y 0 en cualquier otro caso.
 * CC_Z_MASK - El segundo bit más significativo es el indicador de cero (bit Z),
que valdrá 1 cuando la última operación matemática o lógica haya dado por resultado cero y 0 en
cualquier otro caso.
 */
#define CC_N_MASK 0x80000000  // Bit de signo
#define CC_Z_MASK 0x40000000  // Bit de cero

/**
 * Tabla de segmentos
 * 8 entradas de 32 bits
 * se divide en dos partes:
 * los primeros 2 bytes son para guardar la dirección física de comienzo del segmento (base)
 * los siguientes 2 bytes la cantidad de bytes que ocupa
 */
typedef struct {
    uint16_t base;
    uint16_t tamano;
} Tsegmento;

/**
 * CPU valga la redundancia
 * ejecutando - para cortar (o seguir) la ejecución
 */
typedef struct {
    uint32_t regs[32];
    uint8_t mem[RAM];
    Tsegmento segmentos[MAX_SEGMENTOS];
    int ejecutando;
} CPU;

/**
 * Estructura para manejar operandos
 * tipo - para ver si es registro, inmediato, memoria, ninguno
 */
typedef struct {
    uint8_t tipo;
    union {
        uint32_t valor;
        struct {
            uint16_t offset;
            uint8_t codReg;
        } memoria;
    } datos;
} Operando;

/**
 * Estructura para una instrucción
 */
typedef struct {
    uint8_t opcode;
    Operando op1, op2;
    uint16_t direccion; // Dirección donde está la instrucción
} Instruccion;

/**
 * Tipo de función para las instrucciones
 * Usado para punteros a funciones
 */
typedef uint32_t (*FuncionInstruccion)(CPU *cpu, Instruccion *instr);

/**
 * Tabla de instrucciones a funciones a punteros
 */
static FuncionInstruccion tablaInstrucciones[CANTIDAD_INSTRUCCIONES] = {0};

// ==== Funciones principales ====
void inicializaTablaSegmentos(CPU *cpu, uint16_t tamanoCodigo);

void inicializarRegistros(CPU *cpu);
/**
 * Ejecución principal de la VM
 * @param cpu
 */
void vmxRun(CPU *cpu);

// ==== Funciones auxiliares ====
void actualizarCC(CPU *cpu, uint32_t resultado);

void mostrarError(const char *mensaje);

// ==== Llamadas al sistema ====
// No se si dejarlo acá o en instrucción
/**
 * permite almacenar los datos leídos desde el teclado a partir de la posición de memoria
 * apuntada por EDX. El registro ECX indica la cantidad de celdas en los 2 bytes menos significativos y el
 * tamaño de las mismas en los 2 bytes más significativos.
 * @param cpu
 */
void sysRead(CPU *cpu);

/**
 * muestra en pantalla los valores contenidos a partir de la posición de memoria apuntada
 * por EDX. El registro ECX indica la cantidad de celdas en los 2 bytes menos significativos y el tamaño de las
 * mismas en los 2 bytes más significativos.
 * @param cpu
 */
void sysWrite(CPU *cpu);