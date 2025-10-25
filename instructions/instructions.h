/**
 * instructions.h - Implementaciones de todas las instrucciones
 * 
 * Contiene todas las 32 instrucciones del set de instrucciones:
 * - Movimiento de datos (MOV, SWAP, LDL, LDH)
 * - Aritméticas (ADD, SUB, MUL, DIV)
 * - Lógicas (AND, OR, XOR, NOT)
 * - Desplazamientos (SHL, SHR, SAR)
 * - Control de flujo (JMP, JZ, JP, JN, JNZ, JNP, JNN)
 * - Pila y subrutinas (PUSH, POP, CALL, RET)
 * - Sistema (SYS, STOP)
 * - Utilidades (CMP, RND)
 */

#pragma once

#include "../vmx/cpu.h"

// ==== FUNCIONES AUXILIARES ====

/**
 * Lee y decodifica una instrucción desde memoria
 * 
 * Decodifica el opcode y los operandos, actualiza OP1 y OP2.
 * 
 * @param cpu estructura de la CPU
 * @param direccion dirección lógica de la instrucción
 * @param instr estructura donde almacenar la instrucción decodificada
 * @return bytes leídos
 */
uint32_t leerInstruccion(CPU *cpu, uint32_t direccion, Instruccion *instr);

/**
 * Obtiene el valor de un operando (registro, memoria o inmediato)
 */
uint32_t obtenerValorOperando(CPU *cpu, Operando *op);

/**
 * Establece el valor de un operando (registro o memoria)
 */
void establecerValorOperando(CPU *cpu, Operando *op, uint32_t valor);

// ==== MOVIMIENTO DE DATOS ====

/** MOV op1, op2 - Copia valor de op2 a op1 */
uint32_t instr_mov(CPU *cpu, Instruccion *instr);

/** SWAP op1, op2 - Intercambia valores de op1 y op2 */
uint32_t instr_swap(CPU *cpu, Instruccion *instr);

/** LDL op1, op2 - Carga bits 0-15 de op1 con bits 0-15 de op2 */
uint32_t instr_ldl(CPU *cpu, Instruccion *instr);

/** LDH op1, op2 - Carga bits 16-31 de op1 con bits 0-15 de op2 */
uint32_t instr_ldh(CPU *cpu, Instruccion *instr);

/** RND op1, op2 - Genera número aleatorio entre 0 y op2, lo guarda en op1 */
uint32_t instr_rnd(CPU *cpu, Instruccion *instr);

// ==== OPERACIONES ARITMÉTICAS ====

/** ADD op1, op2 - op1 = op1 + op2 (actualiza CC) */
uint32_t instr_add(CPU *cpu, Instruccion *instr);

/** SUB op1, op2 - op1 = op1 - op2 (actualiza CC) */
uint32_t instr_sub(CPU *cpu, Instruccion *instr);

/** MUL op1, op2 - op1 = op1 * op2 (actualiza CC) */
uint32_t instr_mul(CPU *cpu, Instruccion *instr);

/** DIV op1, op2 - op1 = op1 / op2, resto en RES9 (actualiza CC) */
uint32_t instr_div(CPU *cpu, Instruccion *instr);

/** CMP op1, op2 - Compara op1 - op2, actualiza CC (no guarda resultado) */
uint32_t instr_cmp(CPU *cpu, Instruccion *instr);

// ==== OPERACIONES LÓGICAS Y DESPLAZAMIENTOS ====

/** NOT op1 - op1 = ~op1 (inversión bit a bit, actualiza CC) */
uint32_t instr_not(CPU *cpu, Instruccion *instr);

/** AND op1, op2 - op1 = op1 & op2 (actualiza CC) */
uint32_t instr_and(CPU *cpu, Instruccion *instr);

/** OR op1, op2 - op1 = op1 | op2 (actualiza CC) */
uint32_t instr_or(CPU *cpu, Instruccion *instr);

/** XOR op1, op2 - op1 = op1 ^ op2 (actualiza CC) */
uint32_t instr_xor(CPU *cpu, Instruccion *instr);
/**
 * SHL, SHR, SAR: realizan desplazamientos de los bits almacenados en un registro o una posición de
 * memoria y afectan al registro CC. SHL y SHR efectuan corrimientos a la izquierda y a la derecha
 * (respectivamente) y los bits que quedan libres se completan con ceros. SAR también desplaza a la
 * derecha, pero los bits de la izquierda propagan el bit anterior. Es decir, si el contenido es un número
 * negativo, el resultado también lo será, porque agrega unos. Si es un número positivo, agrega ceros.
 * SHL EAX,1 ; corre los 32 bits de EAX una posición a la izquierda
   ; (equivale a multiplicar EAX por 2)
 * SHR [200],EBX ; corre a la derecha los bits de la celda 200,
   ; la cantidad de veces indicada en EBX
 * SAR [50],2 ; corre los bits de la celda de 4 bytes que comienza en 50 dos posiciones
   ; a la derecha, pero conservando el signo (equivale a dividir [50] por 4)
 * @param cpu
 * @param instr
 * @return
 */
uint32_t instr_shl(CPU *cpu, Instruccion *instr);
/**
 * SHL, SHR, SAR: realizan desplazamientos de los bits almacenados en un registro o una posición de
 * memoria y afectan al registro CC. SHL y SHR efectuan corrimientos a la izquierda y a la derecha
 * (respectivamente) y los bits que quedan libres se completan con ceros. SAR también desplaza a la
 * derecha, pero los bits de la izquierda propagan el bit anterior. Es decir, si el contenido es un número
 * negativo, el resultado también lo será, porque agrega unos. Si es un número positivo, agrega ceros.
 * SHL EAX,1 ; corre los 32 bits de EAX una posición a la izquierda
   ; (equivale a multiplicar EAX por 2)
 * SHR [200],EBX ; corre a la derecha los bits de la celda 200,
   ; la cantidad de veces indicada en EBX
 * SAR [50],2 ; corre los bits de la celda de 4 bytes que comienza en 50 dos posiciones
   ; a la derecha, pero conservando el signo (equivale a dividir [50] por 4)
 * @param cpu
 * @param instr
 * @return
 */
uint32_t instr_shr(CPU *cpu, Instruccion *instr);
/**
 * SHL, SHR, SAR: realizan desplazamientos de los bits almacenados en un registro o una posición de
 * memoria y afectan al registro CC. SHL y SHR efectuan corrimientos a la izquierda y a la derecha
 * (respectivamente) y los bits que quedan libres se completan con ceros. SAR también desplaza a la
 * derecha, pero los bits de la izquierda propagan el bit anterior. Es decir, si el contenido es un número
 * negativo, el resultado también lo será, porque agrega unos. Si es un número positivo, agrega ceros.
 * SHL EAX,1 ; corre los 32 bits de EAX una posición a la izquierda
   ; (equivale a multiplicar EAX por 2)
 * SHR [200],EBX ; corre a la derecha los bits de la celda 200,
   ; la cantidad de veces indicada en EBX
 * SAR [50],2 ; corre los bits de la celda de 4 bytes que comienza en 50 dos posiciones
   ; a la derecha, pero conservando el signo (equivale a dividir [50] por 4)
 * @param cpu
 * @param instr
 * @return
 */
uint32_t instr_sar(CPU *cpu, Instruccion *instr);
/**
 * AND, OR, XOR: efectúan las operaciones lógicas básicas bit a bit entre los operandos y afectan al
 * registro CC. El resultado se almacena en el primer operando.
 * AND EAX,EBX ; efectua el AND entre EAX y EBX, el resultado queda en EAX
 * @param cpu
 * @param instr
 * @return
 */
uint32_t instr_and(CPU *cpu, Instruccion *instr);
/**
 * AND, OR, XOR: efectúan las operaciones lógicas básicas bit a bit entre los operandos y afectan al
 * registro CC. El resultado se almacena en el primer operando.
 * AND EAX,EBX ; efectua el AND entre EAX y EBX, el resultado queda en EAX
 * @param cpu
 * @param instr
 * @return
 */
uint32_t instr_or(CPU *cpu, Instruccion *instr);
/**
 * AND, OR, XOR: efectúan las operaciones lógicas básicas bit a bit entre los operandos y afectan al
 * registro CC. El resultado se almacena en el primer operando.
 * AND EAX,EBX ; efectua el AND entre EAX y EBX, el resultado queda en EAX
 * @param cpu
 * @param instr
 * @return
 */
uint32_t instr_xor(CPU *cpu, Instruccion *instr);
/**
 * intercambia los valores de los operandos (ambos deben ser registros y/o celdas de memoria)
 * @param cpu
 * @param instr
 * @return
 */
uint32_t instr_swap(CPU *cpu, Instruccion *instr);
/**
 * carga los 2 bytes menos significativos del primer operando, con los 2 bytes menos significativos
 * del segundo operando. Esta instrucción está especialmente pensada para poder cargar un inmediato de
 * 16 bits, aunque también se puede utilizar con otro tipo de operando
 * @param cpu
 * @param instr
 * @return
 */
uint32_t instr_ldl(CPU *cpu, Instruccion *instr);
/**
* carga los 2 bytes más significativos del primer operando, con los 2 bytes menos significativos del
* segundo operando. Esta instrucción está especialmente pensada para poder cargar un inmediato de 16
* bits, aunque también se puede utilizar con otro tipo de operando.
* @param cpu
* @param instr
* @return
*/
uint32_t instr_ldh(CPU *cpu, Instruccion *instr);
/**
 * carga en el primer operando un número aleatorio entre 0 y el valor del segundo operando
 * @param cpu
 * @param instr
 * @return
 */
// ==== INSTRUCCIONES CON 1 OPERANDO ====
uint32_t instr_rnd(CPU *cpu, Instruccion *instr);
/**
 * efectúa un salto incondicional a la celda del segmento de código indicada en el operando.
 * JMP 0 ;asigna al registro IP la dirección de memoria 0 donde se almacenó la instrucción 1
 * @param cpu
 * @param instr
 * @return
 */
uint32_t instr_jmp(CPU *cpu, Instruccion *instr);
/**
 * Instrucciones de salto condicionales: JZ, JP, JN, JNZ, JNP, JNN
 *
 * Estas instrucciones realizan saltos condicionales en función de los bits del registro CC.
 * Requieren un único operando que indica el desplazamiento dentro del segmento de código.
 *
 * ┌─────────────┬─────┬─────┬─────────────────────────┬─────────────┬─────┬─────┬───────────────────────┐
 * │ Instrucción │ BitN│ BitZ│ Condición               │ Instrucción │ BitN│ BitZ│ Condición             │
 * ├─────────────┼─────┼─────┼─────────────────────────┼─────────────┼─────┼─────┼───────────────────────┤
 * │ JZ (== 0)   │  0  │  1  │ Cero                    │ JNZ (!= 0)  │ 1/0 │ 0/0 │ Negativo o positivo   │
 * │ JP (> 0)    │  0  │  0  │ Positivo                │ JNP (<= 0)  │ 1/0 │ 0/1 │ Negativo o cero       │
 * │ JN (< 0)    │  1  │  0  │ Negativo                │ JNN (>= 0)  │ 0/0 │ 1/0 │ Cero o positivo       │
 * └─────────────┴─────┴─────┴─────────────────────────┴─────────────┴─────┴─────┴───────────────────────┘
 *
 * Ejemplos:
 *  JP 2      ; Salta a la celda indicada por CS+2 si los bits N y Z de CC son cero (>0)
 *  JN EBX    ; Salta a la celda indicada en EBX si el bit N de CC está en 1 (<0)
 *  JZ [8]    ; Salta a la celda 8 si el bit Z de CC es 1 (==0)
 *  JNZ fin   ; Salta a la celda con rótulo "fin" si el bit Z de CC está en 0 (!=0)
 *  JNP fin   ; Salta a la celda con rótulo "fin" si el bit N o el bit Z están en 1 (<=0)
 */
uint32_t instr_jz(CPU *cpu, Instruccion *instr);
/**
 * Instrucciones de salto condicionales: JZ, JP, JN, JNZ, JNP, JNN
 *
 * Estas instrucciones realizan saltos condicionales en función de los bits del registro CC.
 * Requieren un único operando que indica el desplazamiento dentro del segmento de código.
 *
 * ┌─────────────┬─────┬─────┬─────────────────────────┬─────────────┬─────┬─────┬───────────────────────┐
 * │ Instrucción │ BitN│ BitZ│ Condición               │ Instrucción │ BitN│ BitZ│ Condición             │
 * ├─────────────┼─────┼─────┼─────────────────────────┼─────────────┼─────┼─────┼───────────────────────┤
 * │ JZ (== 0)   │  0  │  1  │ Cero                    │ JNZ (!= 0)  │ 1/0 │ 0/0 │ Negativo o positivo   │
 * │ JP (> 0)    │  0  │  0  │ Positivo                │ JNP (<= 0)  │ 1/0 │ 0/1 │ Negativo o cero       │
 * │ JN (< 0)    │  1  │  0  │ Negativo                │ JNN (>= 0)  │ 0/0 │ 1/0 │ Cero o positivo       │
 * └─────────────┴─────┴─────┴─────────────────────────┴─────────────┴─────┴─────┴───────────────────────┘
 *
 * Ejemplos:
 *  JP 2      ; Salta a la celda indicada por CS+2 si los bits N y Z de CC son cero (>0)
 *  JN EBX    ; Salta a la celda indicada en EBX si el bit N de CC está en 1 (<0)
 *  JZ [8]    ; Salta a la celda 8 si el bit Z de CC es 1 (==0)
 *  JNZ fin   ; Salta a la celda con rótulo "fin" si el bit Z de CC está en 0 (!=0)
 *  JNP fin   ; Salta a la celda con rótulo "fin" si el bit N o el bit Z están en 1 (<=0)
 */
uint32_t instr_jp(CPU *cpu, Instruccion *instr);
/**
 * Instrucciones de salto condicionales: JZ, JP, JN, JNZ, JNP, JNN
 *
 * Estas instrucciones realizan saltos condicionales en función de los bits del registro CC.
 * Requieren un único operando que indica el desplazamiento dentro del segmento de código.
 *
 * ┌─────────────┬─────┬─────┬─────────────────────────┬─────────────┬─────┬─────┬───────────────────────┐
 * │ Instrucción │ BitN│ BitZ│ Condición               │ Instrucción │ BitN│ BitZ│ Condición             │
 * ├─────────────┼─────┼─────┼─────────────────────────┼─────────────┼─────┼─────┼───────────────────────┤
 * │ JZ (== 0)   │  0  │  1  │ Cero                    │ JNZ (!= 0)  │ 1/0 │ 0/0 │ Negativo o positivo   │
 * │ JP (> 0)    │  0  │  0  │ Positivo                │ JNP (<= 0)  │ 1/0 │ 0/1 │ Negativo o cero       │
 * │ JN (< 0)    │  1  │  0  │ Negativo                │ JNN (>= 0)  │ 0/0 │ 1/0 │ Cero o positivo       │
 * └─────────────┴─────┴─────┴─────────────────────────┴─────────────┴─────┴─────┴───────────────────────┘
 *
 * Ejemplos:
 *  JP 2      ; Salta a la celda indicada por CS+2 si los bits N y Z de CC son cero (>0)
 *  JN EBX    ; Salta a la celda indicada en EBX si el bit N de CC está en 1 (<0)
 *  JZ [8]    ; Salta a la celda 8 si el bit Z de CC es 1 (==0)
 *  JNZ fin   ; Salta a la celda con rótulo "fin" si el bit Z de CC está en 0 (!=0)
 *  JNP fin   ; Salta a la celda con rótulo "fin" si el bit N o el bit Z están en 1 (<=0)
 */
uint32_t instr_jn(CPU *cpu, Instruccion *instr);
/**
 * Instrucciones de salto condicionales: JZ, JP, JN, JNZ, JNP, JNN
 *
 * Estas instrucciones realizan saltos condicionales en función de los bits del registro CC.
 * Requieren un único operando que indica el desplazamiento dentro del segmento de código.
 *
 * ┌─────────────┬─────┬─────┬─────────────────────────┬─────────────┬─────┬─────┬───────────────────────┐
 * │ Instrucción │ BitN│ BitZ│ Condición               │ Instrucción │ BitN│ BitZ│ Condición             │
 * ├─────────────┼─────┼─────┼─────────────────────────┼─────────────┼─────┼─────┼───────────────────────┤
 * │ JZ (== 0)   │  0  │  1  │ Cero                    │ JNZ (!= 0)  │ 1/0 │ 0/0 │ Negativo o positivo   │
 * │ JP (> 0)    │  0  │  0  │ Positivo                │ JNP (<= 0)  │ 1/0 │ 0/1 │ Negativo o cero       │
 * │ JN (< 0)    │  1  │  0  │ Negativo                │ JNN (>= 0)  │ 0/0 │ 1/0 │ Cero o positivo       │
 * └─────────────┴─────┴─────┴─────────────────────────┴─────────────┴─────┴─────┴───────────────────────┘
 *
 * Ejemplos:
 *  JP 2      ; Salta a la celda indicada por CS+2 si los bits N y Z de CC son cero (>0)
 *  JN EBX    ; Salta a la celda indicada en EBX si el bit N de CC está en 1 (<0)
 *  JZ [8]    ; Salta a la celda 8 si el bit Z de CC es 1 (==0)
 *  JNZ fin   ; Salta a la celda con rótulo "fin" si el bit Z de CC está en 0 (!=0)
 *  JNP fin   ; Salta a la celda con rótulo "fin" si el bit N o el bit Z están en 1 (<=0)
 */
uint32_t instr_jnz(CPU *cpu, Instruccion *instr);
/**
 * Instrucciones de salto condicionales: JZ, JP, JN, JNZ, JNP, JNN
 *
 * Estas instrucciones realizan saltos condicionales en función de los bits del registro CC.
 * Requieren un único operando que indica el desplazamiento dentro del segmento de código.
 *
 * ┌─────────────┬─────┬─────┬─────────────────────────┬─────────────┬─────┬─────┬───────────────────────┐
 * │ Instrucción │ BitN│ BitZ│ Condición               │ Instrucción │ BitN│ BitZ│ Condición             │
 * ├─────────────┼─────┼─────┼─────────────────────────┼─────────────┼─────┼─────┼───────────────────────┤
 * │ JZ (== 0)   │  0  │  1  │ Cero                    │ JNZ (!= 0)  │ 1/0 │ 0/0 │ Negativo o positivo   │
 * │ JP (> 0)    │  0  │  0  │ Positivo                │ JNP (<= 0)  │ 1/0 │ 0/1 │ Negativo o cero       │
 * │ JN (< 0)    │  1  │  0  │ Negativo                │ JNN (>= 0)  │ 0/0 │ 1/0 │ Cero o positivo       │
 * └─────────────┴─────┴─────┴─────────────────────────┴─────────────┴─────┴─────┴───────────────────────┘
 *
 * Ejemplos:
 *  JP 2      ; Salta a la celda indicada por CS+2 si los bits N y Z de CC son cero (>0)
 *  JN EBX    ; Salta a la celda indicada en EBX si el bit N de CC está en 1 (<0)
 *  JZ [8]    ; Salta a la celda 8 si el bit Z de CC es 1 (==0)
 *  JNZ fin   ; Salta a la celda con rótulo "fin" si el bit Z de CC está en 0 (!=0)
 *  JNP fin   ; Salta a la celda con rótulo "fin" si el bit N o el bit Z están en 1 (<=0)
 */
uint32_t instr_jnp(CPU *cpu, Instruccion *instr);
/**
 * Instrucciones de salto condicionales: JZ, JP, JN, JNZ, JNP, JNN
 *
 * Estas instrucciones realizan saltos condicionales en función de los bits del registro CC.
 * Requieren un único operando que indica el desplazamiento dentro del segmento de código.
 *
 * ┌─────────────┬─────┬─────┬─────────────────────────┬─────────────┬─────┬─────┬───────────────────────┐
 * │ Instrucción │ BitN│ BitZ│ Condición               │ Instrucción │ BitN│ BitZ│ Condición             │
 * ├─────────────┼─────┼─────┼─────────────────────────┼─────────────┼─────┼─────┼───────────────────────┤
 * │ JZ (== 0)   │  0  │  1  │ Cero                    │ JNZ (!= 0)  │ 1/0 │ 0/0 │ Negativo o positivo   │
 * │ JP (> 0)    │  0  │  0  │ Positivo                │ JNP (<= 0)  │ 1/0 │ 0/1 │ Negativo o cero       │
 * │ JN (< 0)    │  1  │  0  │ Negativo                │ JNN (>= 0)  │ 0/0 │ 1/0 │ Cero o positivo       │
 * └─────────────┴─────┴─────┴─────────────────────────┴─────────────┴─────┴─────┴───────────────────────┘
 *
 * Ejemplos:
 *  JP 2      ; Salta a la celda indicada por CS+2 si los bits N y Z de CC son cero (>0)
 *  JN EBX    ; Salta a la celda indicada en EBX si el bit N de CC está en 1 (<0)
 *  JZ [8]    ; Salta a la celda 8 si el bit Z de CC es 1 (==0)
 *  JNZ fin   ; Salta a la celda con rótulo "fin" si el bit Z de CC está en 0 (!=0)
 *  JNP fin   ; Salta a la celda con rótulo "fin" si el bit N o el bit Z están en 1 (<=0)
 */
uint32_t instr_jnn(CPU *cpu, Instruccion *instr);
/**
 * efectúa la negación bit a bit del operando y afecta al registro CC.
 * NOT [15] ; invierte cada bit del contenido de la posición de memoria 15
 * @param cpu
 * @param instr
 * @return
 */
uint32_t instr_not(CPU *cpu, Instruccion *instr);
/**
 * ejecuta la llamada al sistema indicada por el valor del operando.
 * @param cpu
 * @param instr
 * @return
 */
uint32_t instr_sys(CPU *cpu, Instruccion *instr);

// ==== INSTRUCIONES SIN OPERANDO ====

/** STOP - Detiene la ejecución del programa */
uint32_t instr_stop(CPU *cpu, Instruccion *instr);

// ==== PILA Y SUBRUTINAS ====

/**
 * PUSH op1 - Empuja op1 a la pila
 * 
 * Operación:
 * 1. SP = SP - 4
 * 2. [SS:SP] = op1
 * 
 * La pila crece hacia direcciones menores.
 */
uint32_t instr_push(CPU *cpu, Instruccion *instr);

/**
 * POP op1 - Saca valor de la pila y lo guarda en op1
 * 
 * Operación:
 * 1. op1 = [SS:SP]
 * 2. SP = SP + 4
 */
uint32_t instr_pop(CPU *cpu, Instruccion *instr);

/**
 * CALL op1 - Llama a subrutina en dirección op1
 * 
 * Operación:
 * 1. Empuja dirección de retorno (IP actual) a la pila
 * 2. IP = [CS:op1]
 * 
 * Permite retornar con RET.
 */
uint32_t instr_call(CPU *cpu, Instruccion *instr);

/**
 * RET - Retorna de subrutina
 * 
 * Operación:
 * 1. IP = [SS:SP] (saca dirección de retorno de la pila)
 * 2. SP = SP + 4
 * 
 * Si la dirección de retorno es 0xFFFFFFFF, detiene el programa.
 */
uint32_t instr_ret(CPU *cpu, Instruccion *instr);