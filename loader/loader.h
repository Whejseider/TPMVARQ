#pragma once

#include <stdint.h>
#include "../vmx/cpu.h"

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

typedef struct {
    uint16_t tamanoPS;
    uint16_t tamanoKS;
    uint16_t tamanoCS;
    uint16_t tamanoDS;
    uint16_t tamanoES;
    uint16_t tamanoSS;
} LayoutSegmentos;

/**
 * Carga los datos del programa desde el archivo .vmx
 * @param nombreArchivo
 * @param cpu
 * @param mostrarDisasm - parámetro opcional [-d] que muestra el código en assembler
 * @return
 */
int cargarPrograma(const char *nombreArchivo, CPU *cpu, int mostrarDisasm);

