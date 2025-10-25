#pragma once

#include <stdint.h>
#include "../vmx/cpu.h"

/**
 * Cabecera de archivo .vmi (Virtual Machine Image)
 * 
 * Estructura de 8 bytes que identifica y describe el archivo:
 * - identificador: "VMI25" (5 bytes)
 * - version: 1 (1 byte) - formato de archivo
 * - tamMemKiB: tamaño en KiB (2 bytes)
 */
typedef struct {
    char identificador[5];  // "VMI25"
    uint8_t version;        // 1
    uint16_t tamMemKiB;     // Tamaño de memoria en KiB
} CabeceraVMI;

/**
 * Carga una imagen de estado de la máquina virtual desde un archivo .vmi
 * 
 * El archivo .vmi contiene un snapshot completo del estado de la VM:
 * 
 * Estructura del archivo:
 * 1. Header (8 bytes): identificador + versión + tamaño memoria
 * 2. Registros (128 bytes): 32 registros x 4 bytes
 * 3. Tabla de segmentos (32 bytes): 8 entradas x 4 bytes (base+tamaño)
 * 4. Memoria principal (variable): contenido completo de la RAM
 * 
 * Permite:
 * - Continuar ejecución desde un breakpoint
 * - Debugging mediante inspección de estado guardado
 * - Checkpoints de programas de larga ejecución
 * 
 * @param nombreArchivo ruta del archivo .vmi a cargar
 * @param cpu estructura CPU donde restaurar el estado
 * @return 1 si carga exitosa, 0 si error
 */
int cargarImagen(const char *nombreArchivo, CPU *cpu);

/**
 * Guarda el estado actual de la máquina virtual en un archivo .vmi
 * 
 * Crea un snapshot completo que incluye:
 * - Todos los registros (IP, SP, flags, registros de propósito general)
 * - Tabla completa de descriptores de segmentos
 * - Contenido completo de la memoria RAM
 * 
 * El archivo generado puede:
 * - Cargarse posteriormente para continuar ejecución (vmx archivo.vmi)
 * - Inspeccionarse con el debugger (vmg archivo.vmi)
 * - Usarse para análisis del estado del programa
 * 
 * Se usa automáticamente en:
 * - Breakpoints (SYS 0x0F)
 * - Final de ejecución si se especificó archivo .vmi
 * 
 * @param nombreArchivo ruta donde guardar el archivo .vmi
 * @param cpu estructura CPU con el estado a guardar
 * @return 1 si guardado exitoso, 0 si error
 */
int guardarImagen(const char *nombreArchivo, CPU *cpu);
