#pragma once

#include <stdint.h>
#include "../vmx/cpu.h"

// Constantes de memoria por defecto
#define MEM_DEF_KIB 16
#define MEM_DEF_BYTES ((size_t)MEM_DEF_KIB * 1024)

typedef struct {
    char identificador[5];
    uint8_t version;
    uint16_t tamanoCodigo;
} CabeceraVMX_V1;

typedef struct {
    char identificador[5];
    uint8_t version;
    uint16_t tamanoCS;
    uint16_t tamanoDS;
    uint16_t tamanoES;
    uint16_t tamanoSS;
    uint16_t tamanoKS;
    uint16_t entryPoint;
} CabeceraVMX_V2;

/**
 * Carga los datos del programa desde el archivo .vmx
 * @param nombreArchivo
 * @param cpu
 * @param mostrarDisasm - parámetro opcional [-d] que muestra el código en assembler
 * @param params lista de parámetros para la subrutina principal
 * @param paramCount cantidad de parámetros
 * @return
 */
int cargarPrograma(const char *nombreArchivo, CPU *cpu, int mostrarDisasm, char **params, uint16_t paramCount);

