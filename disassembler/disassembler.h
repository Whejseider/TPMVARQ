/**
 * dissasembler.h - Desensamblador de código MV2
 * 
 * Traduce el código binario en memoria a lenguaje ensamblador
 * legible para humanos. Útil para debugging y comprensión del programa.
 * 
 * Muestra:
 * - Dirección de cada instrucción
 * - Bytes en hexadecimal
 * - Mnemónico (ADD, MOV, JMP, etc.)
 * - Operandos decodificados
 */

#pragma once

#include "../vmx/cpu.h"

/**
 * Muestra el desensamblado completo del Code Segment
 * 
 * Recorre todo el código binario en CS y lo muestra en formato:
 * [dirección]: bytes_hex  MNEMONICO operando1, operando2
 * 
 * Ejemplo de salida:
 *   [0000]: 10 50 01     MOV EAX, 1
 *   [0003]: 11 50 02     ADD EAX, 2
 *   [0006]: 0F           STOP
 * 
 * Se activa con el flag -d en la línea de comandos.
 * 
 * @param cpu estructura de la CPU (contiene memoria y segmentos)
 * @param tamanoCodigo bytes totales de código a desensamblar
 */
void mostrarDisassembler(CPU *cpu, uint16_t tamanoCodigo);

/**
 * Imprime el mnemónico de una instrucción dado su opcode
 * 
 * Convierte el código numérico a su representación textual:
 * - 0x10 -> "MOV"
 * - 0x11 -> "ADD"
 * - 0x01 -> "JMP"
 * - etc.
 * 
 * @param opcode código de operación (0x00-0x1F)
 */
void mostrarMnemonico(uint8_t opcode);

/**
 * Imprime un operando en formato legible
 * 
 * Muestra el operando según su tipo:
 * - TIPO_REGISTRO: nombre del registro (EAX, EBX, etc.)
 * - TIPO_INMEDIATO: valor en decimal/hexadecimal
 * - TIPO_MEMORIA: [registro+offset] o [dirección]
 * - TIPO_NINGUNO: no imprime nada
 * 
 * Ejemplos:
 *   EAX
 *   0x1234
 *   [DS+10]
 *   [EBX+8]
 * 
 * @param op puntero al operando a mostrar
 * @param cpu estructura de la CPU (para resolver registros de segmento)
 */
void mostrarOperando(Operando *op, CPU *cpu);