#pragma once

#include "../vmx/cpu.h"

/**
 * Traduce una dirección lógica a física usando la tabla de descriptores de segmentos
 * 
 * Las direcciones lógicas en VMX tienen el formato:
 * - Bits 31-16: índice del segmento en la tabla de descriptores
 * - Bits 15-0:  offset (desplazamiento) dentro del segmento
 * 
 * Proceso de traducción:
 * 1. Extraer índice de segmento y offset
 * 2. Validar que el índice existe en la tabla
 * 3. Obtener base y tamaño del segmento
 * 4. Verificar que el acceso no exceda los límites
 * 5. Calcular: dirección_física = base + offset
 * 
 * Esta función NO actualiza LAR, MAR, MBR (lo hacen las funciones de lectura/escritura)
 * 
 * @param cpu estructura de la CPU
 * @param direccionLogica dirección lógica [segmento:offset]
 * @param tamano cantidad de bytes a acceder (para verificar límites)
 * @return dirección física en la memoria principal
 */
uint32_t traducirDireccion(CPU *cpu, uint32_t direccionLogica, uint32_t tamano);

/**
 * Lee 1 byte desde memoria (con signo extendido a 32 bits)
 * 
 * Actualiza registros de control:
 * - LAR: dirección lógica accedida
 * - MAR: [16 bits tamaño | 16 bits dirección_física]
 * - MBR: valor leído con extensión de signo
 * 
 * @param cpu estructura de la CPU
 * @param direccion dirección lógica a leer
 * @return byte leído
 */
uint8_t leerMemoria8(CPU *cpu, uint32_t direccion);

/**
 * Escribe 1 byte en memoria
 * Actualiza LAR, MAR, MBR antes de escribir
 */
void escribirMemoria8(CPU *cpu, uint32_t direccion, uint8_t valor);

/**
 * Lee 2 bytes desde memoria
 * 
 * Formato: [byte_alto | byte_bajo]
 * Actualiza LAR, MAR, MBR
 * Extiende signo a 32 bits
 * 
 * @return word de 16 bits
 */
uint16_t leerMemoria16(CPU *cpu, uint32_t direccion);

/**
 * Escribe 2 bytes en memoria
 * Almacenamiento: [byte_alto | byte_bajo]
 */
void escribirMemoria16(CPU *cpu, uint32_t direccion, uint16_t valor);

/**
 * Lee 4 bytes desde memoria
 * 
 * Formato: [byte3 | byte2 | byte1 | byte0] (del más al menos significativo)
 * Actualiza LAR, MAR, MBR
 * 
 * @return long de 32 bits
 */
uint32_t leerMemoria32(CPU *cpu, uint32_t direccion);

/**
 * Escribe 4 bytes en memoria
 * Almacenamiento: byte más significativo primero
 */
void escribirMemoria32(CPU *cpu, uint32_t direccion, uint32_t valor);
