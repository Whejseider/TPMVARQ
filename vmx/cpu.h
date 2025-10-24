#pragma once

#include <stdint.h>
#include <stddef.h>

#define MAX_SEGMENTOS 6
#define CANTIDAD_INSTRUCCIONES 0x20

#define OP_SYS  0x00
#define OP_JMP  0x01
#define OP_JZ   0x02
#define OP_JP   0x03
#define OP_JN   0x04
#define OP_JNZ  0x05
#define OP_JNP  0x06
#define OP_JNN  0x07
#define OP_NOT  0x08
#define OP_PUSH 0x0B
#define OP_POP  0x0C
#define OP_CALL 0x0D
#define OP_RET  0x0E
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

#define SEG_PS 0
#define SEG_KS 1
#define SEG_CS 2
#define SEG_DS 3
#define SEG_ES 4
#define SEG_SS 5

typedef struct {
    uint16_t tamanoPS;
    uint16_t tamanoKS;
    uint16_t tamanoCS;
    uint16_t tamanoDS;
    uint16_t tamanoES;
    uint16_t tamanoSS;
    uint16_t entryPoint;
} LayoutSegmentos;

enum registros {
    REG_LAR = 0,
    REG_MAR,
    REG_MBR,
    REG_IP,
    REG_OPC,
    REG_OP1,
    REG_OP2,
    REG_SP,
    REG_BP,
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
    REG_ES,
    REG_SS,
    REG_KS,
    REG_PS
};

#define TIPO_NINGUNO 0
#define TIPO_REGISTRO 1
#define TIPO_INMEDIATO 2
#define TIPO_MEMORIA 3

#define CC_N_MASK 0x80000000
#define CC_Z_MASK 0x40000000

typedef struct {
    uint16_t base;
    uint16_t tamano;
} Tsegmento;

typedef struct {
    uint32_t regs[32];
    uint8_t *mem;
    size_t tamMem;
    Tsegmento segmentos[MAX_SEGMENTOS];
    uint16_t cantSegmentos;
    int ejecutando;
    const char *vmiFile;
} CPU;

typedef struct {
    uint8_t tipo;
    uint8_t ancho;
    union {
        int32_t valor;
        struct {
            uint8_t codReg;
            uint8_t sector;
        } registro;
        struct {
            int16_t offset;
            uint8_t codReg;
            uint8_t tam;
        } memoria;
    } datos;
} Operando;

typedef struct {
    uint8_t opcode;
    Operando op1, op2;
    uint16_t direccion;
} Instruccion;

typedef uint32_t (*FuncionInstruccion)(CPU *cpu, Instruccion *instr);
