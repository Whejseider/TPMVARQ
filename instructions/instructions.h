#pragma once

#include "../vmx/vmx.h"

/**
 * Lee la instrucción a ejecutar y carga los registros
 * REGISTRO OP1 y REGISTRO OP2 junto con sus valores
 * Y la dirección de memoria de la instrucción
 * @param cpu
 * @param direccion
 * @param instr
 * @return
 */
uint32_t leerInstruccion(CPU *cpu, uint32_t direccion, Instruccion *instr);
uint32_t obtenerValorOperando(CPU *cpu, Operando *op);
void establecerValorOperando(CPU *cpu, Operando *op, uint32_t valor);

// ==== INSTRUCCIONES CON 2 OPERANDOs ====
/**
 * asigna a un registro o posición de memoria un valor, que puede ser el contenido de otro registro,
 * posición de memoria o un valor inmediato.
 * MOV EAX,EDX ;Carga en EAX el valor del registro EDX
 * MOV EBX,[8] ;Carga en EBX 4 bytes desde la celda de memoria 8 hasta la 11
 * MOV [12],10 ;Carga desde la celda de memoria 12 hasta la 15 el valor decimal 10
 * @param cpu
 * @param instr
 * @return
 */
uint32_t instr_mov(CPU *cpu, Instruccion *instr);
/**
 * ADD, SUB, MUL, DIV: realizan las cuatro operaciones matemáticas básicas. El primer operando debe
 * ser de registro o memoria, ya que es donde se guarda el resultado. El resultado de estas instrucciones
 * afecta el valor del registro CC. El DIV tiene la particularidad de que además guarda el resto de la división
 * entera (módulo) en AC.
 * ADD EAX,2 ; incrementa EAX en 2
 * MUL EAX,[10] ; multiplica EAX por el valor de la celda 10, dejando el resultado en EAX
 * SUB [EBX+10],1 ; resta 1 al valor de la celda de 4 bytes apuntada por EBX+10
 * DIV ECX,7 ; divide el valor de ECX por 7, el resultado queda en ECX y el resto en AC
 * @param cpu
 * @param instr
 * @return
 */
uint32_t instr_add(CPU *cpu, Instruccion *instr);
/**
 * ADD, SUB, MUL, DIV: realizan las cuatro operaciones matemáticas básicas. El primer operando debe
 * ser de registro o memoria, ya que es donde se guarda el resultado. El resultado de estas instrucciones
 * afecta el valor del registro CC. El DIV tiene la particularidad de que además guarda el resto de la división
 * entera (módulo) en AC.
 * ADD EAX,2 ; incrementa EAX en 2
 * MUL EAX,[10] ; multiplica EAX por el valor de la celda 10, dejando el resultado en EAX
 * SUB [EBX+10],1 ; resta 1 al valor de la celda de 4 bytes apuntada por EBX+10
 * DIV ECX,7 ; divide el valor de ECX por 7, el resultado queda en ECX y el resto en AC
 * @param cpu
 * @param instr
 * @return
 */
uint32_t instr_sub(CPU *cpu, Instruccion *instr);
/**
 * ADD, SUB, MUL, DIV: realizan las cuatro operaciones matemáticas básicas. El primer operando debe
 * ser de registro o memoria, ya que es donde se guarda el resultado. El resultado de estas instrucciones
 * afecta el valor del registro CC. El DIV tiene la particularidad de que además guarda el resto de la división
 * entera (módulo) en AC.
 * ADD EAX,2 ; incrementa EAX en 2
 * MUL EAX,[10] ; multiplica EAX por el valor de la celda 10, dejando el resultado en EAX
 * SUB [EBX+10],1 ; resta 1 al valor de la celda de 4 bytes apuntada por EBX+10
 * DIV ECX,7 ; divide el valor de ECX por 7, el resultado queda en ECX y el resto en AC
 * @param cpu
 * @param instr
 * @return
 */
uint32_t instr_mul(CPU *cpu, Instruccion *instr);
/**
 * ADD, SUB, MUL, DIV: realizan las cuatro operaciones matemáticas básicas. El primer operando debe
 * ser de registro o memoria, ya que es donde se guarda el resultado. El resultado de estas instrucciones
 * afecta el valor del registro CC. El DIV tiene la particularidad de que además guarda el resto de la división
 * entera (módulo) en AC.
 * ADD EAX,2 ; incrementa EAX en 2
 * MUL EAX,[10] ; multiplica EAX por el valor de la celda 10, dejando el resultado en EAX
 * SUB [EBX+10],1 ; resta 1 al valor de la celda de 4 bytes apuntada por EBX+10
 * DIV ECX,7 ; divide el valor de ECX por 7, el resultado queda en ECX y el resto en AC
 * @param cpu
 * @param instr
 * @return
 */
uint32_t instr_div(CPU *cpu, Instruccion *instr);
/**
 * similar a la instrucción SUB, el segundo operando se resta del primero, pero éste no almacena el
 * resultado, solamente se modifican los bits N y Z del registro CC. Es útil para comparar dos valores y
 * generalmente se utiliza antes de una instrucción de salto condicional.
 * CMP EAX,[1000] ; compara los contenidos de EAX y la celda 1000
 * @param cpu
 * @param instr
 * @return
 */
uint32_t instr_cmp(CPU *cpu, Instruccion *instr);
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

/**
 * detiene la ejecución del programa
 * @param cpu
 * @param instr
 * @return
 */
uint32_t instr_stop(CPU *cpu, Instruccion *instr);