/**
 * Módulo de llamadas al sistema para la máquina virtual VMX
 * 
 * Este módulo maneja todas las operaciones de entrada/salida
 * y comunicación con el sistema operativo host.
 */

#pragma once

#include "../vmx/cpu.h"

// ==== CONSTANTES DE LLAMADAS AL SISTEMA ====

#define SYS_READ  0x01    // Leer datos del teclado (legacy)
#define SYS_WRITE 0x02    // Escribir datos a pantalla (legacy)
#define SYS_STRING_READ  0x03    // Leer string desde teclado hacia memoria
#define SYS_STRING_WRITE 0x04    // Escribir string desde memoria
#define SYS_CLEAR_SCREEN 0x07    // Limpiar pantalla
#define SYS_BREAKPOINT   0x0F    // Generar snapshot .vmi

// ==== MÁSCARAS DE MODO DE ENTRADA/SALIDA ====

#define MODE_DECIMAL  0x01    // Modo decimal
#define MODE_CHAR     0x02    // Modo carácter
#define MODE_OCTAL    0x04    // Modo octal
#define MODE_HEX      0x08    // Modo hexadecimal
#define MODE_BINARY   0x10    // Modo binario

// ==== FUNCIONES PRINCIPALES ====

/**
 * Ejecuta una llamada al sistema
 * 
 * @param cpu estructura de la CPU
 * @param syscall número de la llamada al sistema
 * @return resultado de la ejecución
 */
uint32_t ejecutarSyscall(CPU *cpu, uint32_t syscall);

/**
 * Lee datos del teclado y los almacena en memoria
 * 
 * Registros utilizados:
 * - EAX: modo de entrada (decimal, hex, octal, binario, char)
 * - EDX: dirección de memoria donde almacenar
 * - ECX: cantidad (bits bajos) y tamaño (bits altos)
 * 
 * @param cpu estructura de la CPU
 */
void sysRead(CPU *cpu);

/**
 * Muestra datos de memoria en pantalla
 * 
 * Registros utilizados:
 * - EAX: modo de salida (decimal, hex, octal, binario, char)
 * - EDX: dirección de memoria a mostrar
 * - ECX: cantidad (bits bajos) y tamaño (bits altos)
 * 
 * @param cpu estructura de la CPU
 */
void sysWrite(CPU *cpu);

/**
 * Lee un string desde el teclado y lo almacena en memoria
 * 
 * @param cpu estructura de la CPU
 */
void sysStringRead(CPU *cpu);

/**
 * Muestra un string desde memoria en pantalla
 * 
 * @param cpu estructura de la CPU
 */
void sysStringWrite(CPU *cpu);

/**
 * Limpia la pantalla
 */
void sysClearScreen(void);

/**
 * Genera un snapshot .vmi
 * 
 * @param cpu estructura de la CPU
 */
void sysBreakpoint(CPU *cpu);

// ==== FUNCIONES AUXILIARES ====

/**
 * Lee un valor según el modo especificado
 * 
 * @param modo máscara de modo de entrada
 * @return valor leído del usuario
 */
int32_t leerValorPorModo(uint32_t modo);

/**
 * Muestra un valor según el modo especificado
 * 
 * @param valor valor a mostrar
 * @param modo máscara de modo de salida
 * @param tamano tamaño en bytes del valor
 */
void mostrarValorPorModo(int32_t valor, uint32_t modo, uint16_t tamano);
