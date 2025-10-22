/**
 * Módulo de utilidades comunes para la máquina virtual VMX
 * 
 * Este módulo contiene funciones auxiliares compartidas
 * entre diferentes módulos del sistema.
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// ==== CÓDIGOS DE ERROR ====

typedef enum {
    VMX_SUCCESS = 0,
    VMX_ERROR_FILE_NOT_FOUND,
    VMX_ERROR_INVALID_FORMAT,
    VMX_ERROR_MEMORY_ACCESS,
    VMX_ERROR_INVALID_INSTRUCTION,
    VMX_ERROR_DIVISION_BY_ZERO,
    VMX_ERROR_SEGMENT_FAULT,
    VMX_ERROR_INVALID_VERSION,
    VMX_ERROR_UNKNOWN
} vmx_error_t;

// ==== FUNCIONES DE MANEJO DE ERRORES ====

/**
 * Muestra un mensaje de error en stderr
 * 
 * @param mensaje mensaje de error a mostrar
 */
void mostrarError(const char *mensaje);

/**
 * Muestra un mensaje de error con código específico
 * 
 * @param codigo código de error VMX
 * @param mensaje mensaje adicional (opcional, puede ser NULL)
 */
void mostrarErrorConCodigo(vmx_error_t codigo, const char *mensaje);

/**
 * Termina el programa con un código de error específico
 * 
 * @param codigo código de error VMX
 * @param mensaje mensaje de error (opcional, puede ser NULL)
 */
void terminarConError(vmx_error_t codigo, const char *mensaje);

// ==== FUNCIONES DE VALIDACIÓN ====

/**
 * Valida que un archivo existe y es legible
 * 
 * @param nombreArchivo ruta del archivo a validar
 * @return 1 si es válido, 0 si no
 */
int validarArchivo(const char *nombreArchivo);

/**
 * Valida que un identificador de archivo VMX es correcto
 * 
 * @param identificador identificador a validar
 * @return 1 si es válido, 0 si no
 */
int validarIdentificadorVMX(const char *identificador);

/**
 * Valida que una versión de archivo VMX es soportada
 * 
 * @param version versión a validar
 * @return 1 si es válida, 0 si no
 */
int validarVersionVMX(uint8_t version);

// ==== FUNCIONES DE CONVERSIÓN ====

/**
 * Convierte un valor hexadecimal a string
 * 
 * @param valor valor a convertir
 * @param buffer buffer donde guardar el resultado
 * @param tamano tamaño del buffer
 */
void hexToString(uint32_t valor, char *buffer, size_t tamano);

/**
 * Convierte un valor decimal a string
 * 
 * @param valor valor a convertir
 * @param buffer buffer donde guardar el resultado
 * @param tamano tamaño del buffer
 */
void decimalToString(int32_t valor, char *buffer, size_t tamano);

// ==== FUNCIONES DE DEBUG ====

/**
 * Muestra información de debug sobre un registro
 * 
 * @param nombreRegistro nombre del registro
 * @param valor valor del registro
 */
void debugRegistro(const char *nombreRegistro, uint32_t valor);

/**
 * Muestra información de debug sobre memoria
 * 
 * @param direccion dirección de memoria
 * @param valor valor en esa dirección
 * @param tamano tamaño del acceso
 */
void debugMemoria(uint32_t direccion, uint32_t valor, uint32_t tamano);
