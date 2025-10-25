/**
 * Módulo de llamadas al sistema para la máquina virtual VMX
 * 
 * Este módulo maneja todas las operaciones de entrada/salida
 * y comunicación con el sistema operativo host.
 */

#pragma once

#include "../vmx/cpu.h"

// ==== CONSTANTES DE LLAMADAS AL SISTEMA ====

#define SYS_READ  0x01    // Leer datos del teclado
#define SYS_WRITE 0x02    // Escribir datos a pantalla
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
 * Ejecuta una llamada al sistema según el número especificado
 * 
 * Dispatcher principal de syscalls. Soporta:
 * 
 * MV1:
 * - 0x01: READ  - Leer datos con formato (decimal, hex, octal, binario, char)
 * - 0x02: WRITE - Escribir datos con formato
 * 
 * MV2:
 * - 0x03: STRING_READ  - Leer string desde teclado
 * - 0x04: STRING_WRITE - Escribir string desde memoria
 * - 0x07: CLEAR_SCREEN - Limpiar consola
 * - 0x0F: BREAKPOINT    - Pausar ejecución y guardar estado
 * 
 * @param cpu estructura de la CPU
 * @param syscall número de la syscall a ejecutar (0x00-0x0F)
 * @return código de resultado: 0=abortar, 1=continuar, 3=paso_a_paso
 */
uint32_t ejecutarSyscall(CPU *cpu, uint32_t syscall);

/**
 * SYS 0x01: READ - Lee datos del teclado con formato y los almacena en memoria
 * 
 * Lee valores individuales desde teclado según el modo especificado.
 * Muestra un prompt [XXXX]: para cada valor (donde XXXX es la dirección física).
 * 
 * Configuración de registros (entrada):
 * - EAX: modo de entrada (combinación de flags)
 *   * 0x01: decimal
 *   * 0x02: carácter
 *   * 0x04: octal
 *   * 0x08: hexadecimal
 *   * 0x10: binario
 * - EDX: dirección lógica donde almacenar
 * - ECX: [16 bits tamaño | 16 bits cantidad]
 *   * Bits 31-16: tamaño de cada celda (1, 2 o 4 bytes)
 *   * Bits 15-0: cantidad de valores a leer
 * 
 * Ejemplo:
 *   EAX=0x01 (decimal), EDX=DS+0, ECX=0x00040002 (4 bytes, 2 valores)
 *   Prompt: [XXXX]: 100
 *   Prompt: [YYYY]: 200
 *   Resultado: memoria guarda 100 y 200 como enteros de 4 bytes
 * 
 * @param cpu estructura de la CPU
 */
void sysRead(CPU *cpu);

/**
 * SYS 0x02: WRITE - Muestra datos de memoria en pantalla con formato
 * 
 * Imprime valores desde memoria según el modo especificado.
 * Muestra un prompt [XXXX]: para cada valor (donde XXXX es la dirección física).
 * 
 * Configuración de registros (entrada):
 * - EAX: modo de salida (combinación de flags)
 *   * 0x01: decimal
 *   * 0x02: carácter (no imprimibles se muestran como '.')
 *   * 0x04: octal
 *   * 0x08: hexadecimal
 *   * 0x10: binario
 * - EDX: dirección lógica desde donde leer
 * - ECX: [16 bits tamaño | 16 bits cantidad]
 *   * Bits 31-16: tamaño de cada celda (1, 2 o 4 bytes)
 *   * Bits 15-0: cantidad de valores a mostrar
 * 
 * Ejemplo:
 *   EAX=0x0F (todos los formatos), EDX=DS+10, ECX=0x00020001
 *   Salida: [XXXX]: 0b0100000101100001 0x4161 0o40541 Aa 16737
 * 
 * @param cpu estructura de la CPU
 */
void sysWrite(CPU *cpu);

/**
 * SYS 0x03: STRING READ - Lee un string desde teclado (MV2)
 * 
 * Lee una línea completa de texto desde stdin y la almacena en memoria
 * como un string terminado en null ('\0').
 * 
 * Configuración de registros (entrada):
 * - EDX: dirección lógica donde almacenar el string
 * - ECX: longitud máxima a leer en bytes
 *   * Si ECX = 0 o > 65535, se usa 65535 por defecto
 * 
 * Registros modificados (salida):
 * - AC: longitud del string leído (sin contar el '\0')
 * 
 * Comportamiento:
 * - Muestra prompt "> " y espera entrada del usuario
 * - Lee hasta presionar Enter o alcanzar maxLongitud
 * - Remueve el '\n' del final si existe
 * - Almacena el string con '\0' terminal
 * - Seguro contra buffer overflow
 * 
 * Ejemplo:
 *   EDX = DS+100, ECX = 50
 *   Usuario ingresa: "Hola mundo"
 *   Resultado: memoria[100..110] = "Hola mundo\0", AC = 10
 * 
 * @param cpu estructura de la CPU
 */
void sysStringRead(CPU *cpu);

/**
 * SYS 0x04: STRING WRITE - Imprime un string desde memoria (MV2)
 * 
 * Lee un string null-terminated desde memoria y lo imprime en stdout.
 * 
 * Configuración de registros (entrada):
 * - EDX: dirección lógica del string a imprimir
 * - ECX: longitud máxima a imprimir (protección)
 *   * Si ECX = 0 o > 65535, se usa 65535 por defecto
 * 
 * Comportamiento:
 * - Lee caracteres byte por byte desde memoria
 * - Imprime hasta encontrar '\0' o alcanzar maxLongitud
 * - El '\0' NO se imprime
 * - Hace flush de stdout al finalizar
 * - Seguro contra strings sin null terminator
 * 
 * Ejemplo:
 *   EDX = KS+0 (apunta a "Hola\0"), ECX = 100
 *   Salida en pantalla: "Hola"
 * 
 * @param cpu estructura de la CPU
 */
void sysStringWrite(CPU *cpu);

/**
 * SYS 0x07: CLEAR SCREEN - Limpia la pantalla de la consola (MV2)
 * 
 * No requiere configuración de registros.
 * No modifica registros.
 * 
 * Comportamiento:
 * - Windows: Usa API de consola (GetStdHandle, FillConsoleOutput)
 * - Unix/Linux: Envía secuencias ANSI escape (\033[2J\033[H)
 * - Borra todo el contenido visible
 * - Mueve el cursor a la esquina superior izquierda
 */
void sysClearScreen(void);

/**
 * SYS 0x0F: BREAKPOINT - Pausa la ejecución para debugging (MV2)
 * 
 * Herramienta de depuración que:
 * 1. Guarda el estado completo de la VM en archivo .vmi
 * 2. Pausa la ejecución
 * 3. Espera una acción del usuario
 * 
 * Requisitos:
 * - Debe haberse especificado un archivo .vmi al ejecutar la VM
 * - Si no hay archivo .vmi, el breakpoint se ignora silenciosamente
 * 
 * Opciones del usuario:
 * - 'g' o 'G': GO - Continuar ejecución normal
 * - 'q' o 'Q': QUIT - Abortar ejecución (mantiene .vmi para reiniciar)
 * - Enter: Ejecutar siguiente instrucción (modo paso a paso)
 * - Otra entrada: Continuar normal
 * 
 * Uso típico:
 * - Colocar SYS 0x0F en puntos críticos del código
 * - Ejecutar: vmx programa.vmx estado.vmi
 * - En otro terminal: vmg estado.vmi (para ver estado)
 * - Continuar: vmx -i estado.vmi
 * 
 * @param cpu estructura de la CPU
 * @return 0=continuar, 1=paso_a_paso, 2=quit
 */
uint32_t sysBreakpoint(CPU *cpu);

// ==== FUNCIONES AUXILIARES ====

/**
 * Lee un valor desde teclado según el modo especificado
 * 
 * Función auxiliar para sysRead. Interpreta la entrada del usuario
 * según el flag activado en el modo.
 * 
 * @param modo máscara de modo (MODE_DECIMAL, MODE_HEX, etc.)
 * @return valor leído e interpretado como int32_t
 */
int32_t leerValorPorModo(uint32_t modo);

/**
 * Muestra un valor en pantalla según el modo especificado
 * 
 * Función auxiliar para sysWrite. Imprime el valor en uno o más
 * formatos según los flags activos.
 * 
 * @param valor valor a mostrar (interpretado según tamano)
 * @param modo máscara de modo de salida (puede combinar varios)
 * @param tamano tamaño real del valor (1, 2 o 4 bytes)
 */
void mostrarValorPorModo(int32_t valor, uint32_t modo, uint16_t tamano);
