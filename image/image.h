#pragma once

#include <stdint.h>
#include "../vmx/cpu.h"

// Header para archivos de imagen .vmi
typedef struct {
    char identificador[5];  // "VMI25"
    uint8_t version;        // 1
    uint16_t tamMemKiB;     // Tamaño de memoria en KiB
} CabeceraVMI;

/**
 * Carga una imagen de la máquina virtual desde un archivo .vmi
 * @param nombreArchivo ruta del archivo .vmi
 * @param cpu estructura CPU donde cargar la imagen
 * @return 1 si tuvo éxito, 0 si falló
 */
int cargarImagen(const char *nombreArchivo, CPU *cpu);

/**
 * Guarda el estado actual de la máquina virtual en un archivo .vmi
 * @param nombreArchivo ruta del archivo .vmi
 * @param cpu estructura CPU con el estado a guardar
 * @return 1 si tuvo éxito, 0 si falló
 */
int guardarImagen(const char *nombreArchivo, CPU *cpu);
