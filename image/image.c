#include <stdio.h>
#include <string.h>
#include "image.h"
#include "../utils/utils.h"

int cargarImagen(const char *nombreArchivo, CPU *cpu) {
    if (!validarArchivo(nombreArchivo)) {
        mostrarErrorConCodigo(VMX_ERROR_FILE_NOT_FOUND, nombreArchivo);
        return 0;
    }

    FILE *archivo = fopen(nombreArchivo, "rb");
    if (!archivo) {
        mostrarErrorConCodigo(VMX_ERROR_FILE_NOT_FOUND, nombreArchivo);
        return 0;
    }

    // Leer cabecera
    CabeceraVMI cabecera;
    if (fread(&cabecera.identificador, 1, 5, archivo) != 5 ||
        fread(&cabecera.version, 1, 1, archivo) != 1 ||
        fread(&cabecera.tamMemKiB, 1, 2, archivo) != 2) {
        mostrarErrorConCodigo(VMX_ERROR_INVALID_FORMAT, "No se pudo leer la cabecera VMI");
        fclose(archivo);
        return 0;
    }

    // Validar identificador
    if (strncmp(cabecera.identificador, "VMI25", 5) != 0) {
        mostrarErrorConCodigo(VMX_ERROR_INVALID_FORMAT, "Identificador VMI incorrecto");
        fclose(archivo);
        return 0;
    }

    // Validar versión
    if (cabecera.version != 1) {
        mostrarErrorConCodigo(VMX_ERROR_INVALID_VERSION, "Versión VMI no soportada");
        fclose(archivo);
        return 0;
    }

    // Convertir tamaño de memoria (big endian)
    uint16_t tamMemKiB = (cabecera.tamMemKiB >> 8) | ((cabecera.tamMemKiB & 0xFF) << 8);
    size_t tamMem = (size_t)tamMemKiB * 1024;

    printf("\n=== CARGANDO IMAGEN VMI ===\n");
    printf("Identificador: '%.5s'\n", cabecera.identificador);
    printf("Versión: %u\n", cabecera.version);
    printf("Tamaño de memoria: %u KiB (%zu bytes)\n\n", tamMemKiB, tamMem);

    // Alocar memoria si es necesario
    if (cpu->mem == NULL || cpu->tamMem != tamMem) {
        if (cpu->mem != NULL) {
            free(cpu->mem);
        }
        cpu->mem = (uint8_t*)malloc(tamMem);
        if (!cpu->mem) {
            mostrarErrorConCodigo(VMX_ERROR_MEMORY_ACCESS, "No se pudo alocar memoria");
            fclose(archivo);
            return 0;
        }
        cpu->tamMem = tamMem;
    }

    // Leer registros (32 registros × 4 bytes = 128 bytes)
    if (fread(cpu->regs, 4, 32, archivo) != 32) {
        mostrarErrorConCodigo(VMX_ERROR_INVALID_FORMAT, "No se pudieron leer los registros");
        fclose(archivo);
        return 0;
    }

    // Leer tabla de segmentos (6 segmentos × 4 bytes = 24 bytes, pero se reservan 32)
    uint8_t tablaSegBuffer[32];
    if (fread(tablaSegBuffer, 1, 32, archivo) != 32) {
        mostrarErrorConCodigo(VMX_ERROR_INVALID_FORMAT, "No se pudo leer la tabla de segmentos");
        fclose(archivo);
        return 0;
    }

    // Decodificar tabla de segmentos (big endian)
    for (int i = 0; i < MAX_SEGMENTOS; i++) {
        uint16_t base = (tablaSegBuffer[i*4] << 8) | tablaSegBuffer[i*4 + 1];
        uint16_t tamano = (tablaSegBuffer[i*4 + 2] << 8) | tablaSegBuffer[i*4 + 3];
        cpu->segmentos[i].base = base;
        cpu->segmentos[i].tamano = tamano;
    }

    // Leer memoria principal
    if (fread(cpu->mem, 1, tamMem, archivo) != tamMem) {
        mostrarErrorConCodigo(VMX_ERROR_INVALID_FORMAT, "No se pudo leer la memoria");
        fclose(archivo);
        return 0;
    }

    fclose(archivo);
    cpu->ejecutando = 1;

    printf("Imagen cargada exitosamente.\n");
    return 1;
}

int guardarImagen(const char *nombreArchivo, CPU *cpu) {
    FILE *archivo = fopen(nombreArchivo, "wb");
    if (!archivo) {
        mostrarErrorConCodigo(VMX_ERROR_FILE_NOT_FOUND, nombreArchivo);
        return 0;
    }

    // Preparar cabecera
    CabeceraVMI cabecera;
    memcpy(cabecera.identificador, "VMI25", 5);
    cabecera.version = 1;
    uint16_t tamMemKiB = (uint16_t)(cpu->tamMem / 1024);
    // Big endian
    cabecera.tamMemKiB = (tamMemKiB >> 8) | ((tamMemKiB & 0xFF) << 8);

    // Escribir cabecera
    if (fwrite(&cabecera.identificador, 1, 5, archivo) != 5 ||
        fwrite(&cabecera.version, 1, 1, archivo) != 1 ||
        fwrite(&cabecera.tamMemKiB, 1, 2, archivo) != 2) {
        mostrarErrorConCodigo(VMX_ERROR_INVALID_FORMAT, "No se pudo escribir la cabecera VMI");
        fclose(archivo);
        return 0;
    }

    // Escribir registros
    if (fwrite(cpu->regs, 4, 32, archivo) != 32) {
        mostrarErrorConCodigo(VMX_ERROR_INVALID_FORMAT, "No se pudieron escribir los registros");
        fclose(archivo);
        return 0;
    }

    // Escribir tabla de segmentos (big endian, 32 bytes)
    uint8_t tablaSegBuffer[32] = {0};
    for (int i = 0; i < MAX_SEGMENTOS; i++) {
        tablaSegBuffer[i*4] = (cpu->segmentos[i].base >> 8) & 0xFF;
        tablaSegBuffer[i*4 + 1] = cpu->segmentos[i].base & 0xFF;
        tablaSegBuffer[i*4 + 2] = (cpu->segmentos[i].tamano >> 8) & 0xFF;
        tablaSegBuffer[i*4 + 3] = cpu->segmentos[i].tamano & 0xFF;
    }
    if (fwrite(tablaSegBuffer, 1, 32, archivo) != 32) {
        mostrarErrorConCodigo(VMX_ERROR_INVALID_FORMAT, "No se pudo escribir la tabla de segmentos");
        fclose(archivo);
        return 0;
    }

    // Escribir memoria
    if (fwrite(cpu->mem, 1, cpu->tamMem, archivo) != cpu->tamMem) {
        mostrarErrorConCodigo(VMX_ERROR_INVALID_FORMAT, "No se pudo escribir la memoria");
        fclose(archivo);
        return 0;
    }

    fclose(archivo);
    printf("\nImagen guardada en: %s\n", nombreArchivo);
    return 1;
}
