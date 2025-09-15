#pragma once

#include <stdint.h>
#include "../vmx/vmx.h"

typedef struct {
    char identificador[5];  // "VMX25"
    uint8_t version;        // 1
    uint16_t tamanoCodigo;  // Tamaño del código
} CabeceraVMX;

/**
 * Carga los datos del programa desde el archivo .vmx
 * @param nombreArchivo
 * @param cpu
 * @param mostrarDisasm - parámetro opcional [-d] que muestra el código en assembler
 * @return
 */
int cargarPrograma(const char *nombreArchivo, CPU *cpu, int mostrarDisasm);

