/**
 * vmx.h - Núcleo de la máquina virtual
 * 
 * Contiene las funciones principales para:
 * - Inicialización de la CPU y segmentos
 * - Ciclo fetch-decode-execute
 * - Actualización de condition codes
 */

#pragma once

#include <stdint.h>
#include "cpu.h"

/**
 * Inicializa la tabla de descriptores de segmentos
 * 
 * Crea los segmentos especificados en el layout y los coloca
 * consecutivamente en memoria. Calcula las direcciones base de cada uno.
 * 
 * Proceso:
 * 1. Lee los tamaños desde layout
 * 2. Asigna direcciones base consecutivas
 * 3. Crea descriptores en cpu->segmentos[]
 * 4. Solo crea segmentos con tamaño > 0
 * 
 * @param cpu estructura de la CPU
 * @param layout configuración de tamaños de segmentos
 */
void inicializaTablaSegmentos(CPU *cpu, LayoutSegmentos *layout);

/**
 * Inicializa los registros de la CPU para comenzar ejecución
 * 
 * Configura:
 * - Registros de segmento (CS, DS, ES, SS, KS, PS)
 * - IP apuntando al entry point en CS
 * - SP apuntando al tope de SS
 * - Pila inicial con: dirección retorno (0xFFFFFFFF), argc, argv
 * 
 * La pila crece hacia direcciones menores.
 * 
 * @param cpu estructura de la CPU
 * @param layout configuración de segmentos
 * @param argc cantidad de parámetros en Param Segment
 * @param argvPtr dirección lógica del array argv en PS
 */
void inicializarRegistros(CPU *cpu, LayoutSegmentos *layout, uint16_t argc, uint32_t argvPtr);

/**
 * Ciclo principal de ejecución de la máquina virtual
 * 
 * Implementa el ciclo fetch-decode-execute:
 * 1. FETCH: Leer instrucción desde memoria en IP
 * 2. DECODE: Decodificar operandos y actualizar OP1, OP2
 * 3. EXECUTE: Ejecutar la instrucción usando tabla de funciones
 * 4. Avanzar IP a siguiente instrucción
 * 
 * Se ejecuta hasta que:
 * - Se encuentra instrucción STOP
 * - IP sale del Code Segment
 * - Ocurre un error fatal
 * - cpu->ejecutando se pone en 0
 * 
 * @param cpu estructura de la CPU
 */
void vmxRun(CPU *cpu);

/**
 * Actualiza los Condition Codes (registro CC) según el resultado
 * 
 * Actualiza los flags:
 * - N (Negative): bit 31 = 1 si resultado < 0
 * - Z (Zero): bit 30 = 1 si resultado == 0
 * 
 * Usado después de operaciones aritméticas y CMP.
 * 
 * @param cpu estructura de la CPU
 * @param resultado valor a evaluar (interpretado como signed)
 */
void actualizarCC(CPU *cpu, uint32_t resultado);