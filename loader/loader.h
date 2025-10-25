/**
 * loader.h - Cargador de programas VMX
 * 
 * Se encarga de:
 * - Leer y validar archivos .vmx (programas compilados)
 * - Construir la memoria con todos los segmentos
 * - Crear el Param Segment con argc/argv
 * - Inicializar la CPU para comenzar ejecución
 */

#pragma once

#include <stdint.h>
#include "../vmx/cpu.h"

// ==== CONSTANTES DE MEMORIA ====

#define MEM_DEF_KIB 16                         // Memoria por defecto en KiB
#define MEM_DEF_BYTES ((size_t)MEM_DEF_KIB * 1024)  // Memoria por defecto en bytes

// ==== FORMATOS DE ARCHIVO VMX ====

/**
 * Cabecera de archivo VMX versión 1 (MV1)
 * 
 * Estructura de 8 bytes:
 * - identificador: "VMX25" (5 bytes)
 * - version: 1 (1 byte)
 * - tamanoCodigo: bytes de código (2 bytes)
 * 
 * Formato simple: solo contiene código ejecutable.
 */
typedef struct {
    char identificador[5];  // "VMX25"
    uint8_t version;        // 1
    uint16_t tamanoCodigo;  // Tamaño del código
} CabeceraVMX_V1;

/**
 * Cabecera de archivo VMX versión 2 (MV2)
 * 
 * Estructura de 16 bytes:
 * - identificador: "VMX25" (5 bytes)
 * - version: 2 (1 byte)
 * - Tamaños de segmentos (5 × 2 bytes):
 *   * tamanoCS: Code Segment
 *   * tamanoDS: Data Segment
 *   * tamanoES: Extra Segment
 *   * tamanoSS: Stack Segment
 *   * tamanoKS: Konstant Segment
 * - entryPoint: offset de inicio en CS (2 bytes)
 * 
 * Después del header:
 * - CS bytes: código ejecutable
 * - DS bytes: datos inicializados
 * - ES bytes: datos extra
 * - KS bytes: constantes
 * 
 * Nota: SS no tiene contenido inicial (se crea vacío)
 */
typedef struct {
    char identificador[5];  // "VMX25"
    uint8_t version;        // 2
    uint16_t tamanoCS;      // Tamaño Code Segment
    uint16_t tamanoDS;      // Tamaño Data Segment
    uint16_t tamanoES;      // Tamaño Extra Segment
    uint16_t tamanoSS;      // Tamaño Stack Segment
    uint16_t tamanoKS;      // Tamaño Konstant Segment
    uint16_t entryPoint;    // Punto de entrada en CS
} CabeceraVMX_V2;

/**
 * Carga un programa desde un archivo .vmx y prepara la CPU para ejecutarlo
 * 
 * Proceso completo:
 * 1. Validar y abrir archivo .vmx
 * 2. Leer cabecera y determinar versión (V1 o V2)
 * 3. Leer contenido de cada segmento desde el archivo
 * 4. Construir Param Segment (PS) con argc y argv si hay parámetros
 * 5. Crear tabla de descriptores de segmentos
 * 6. Inicializar registros de la CPU
 * 7. Opcionalmente mostrar disassembler del código
 * 
 * Memoria necesaria:
 * - Se calcula: PS + KS + CS + DS + ES + SS
 * - Si no hay memoria suficiente, se aborta
 * 
 * Param Segment (si hay parámetros):
 * - Strings de parámetros null-terminated consecutivos
 * - Array de punteros (direcciones lógicas) a cada string
 * - Formato argc/argv compatible con C
 * 
 * @param nombreArchivo ruta del archivo .vmx
 * @param cpu estructura de la CPU (debe tener memoria alocada)
 * @param mostrarDisasm 1 para mostrar disassembler, 0 para no mostrar
 * @param params array de strings con parámetros para el programa
 * @param paramCount cantidad de parámetros en el array
 * @return 1 si carga exitosa, 0 si error
 */
int cargarPrograma(const char *nombreArchivo, CPU *cpu, int mostrarDisasm, char **params, uint16_t paramCount);

