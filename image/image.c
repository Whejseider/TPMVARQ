#include <stdio.h>
#include <string.h>
#include "image.h"
#include "../utils/utils.h"

// Implementación de carga de imagen .vmi
int cargarImagen(const char *nombreArchivo, CPU *cpu) {
    // === VALIDAR Y ABRIR ARCHIVO ===
    if (!validarArchivo(nombreArchivo)) {
        mostrarErrorConCodigo(VMX_ERROR_FILE_NOT_FOUND, nombreArchivo);
        return 0;
    }

    FILE *archivo = fopen(nombreArchivo, "rb");
    if (!archivo) {
        mostrarErrorConCodigo(VMX_ERROR_FILE_NOT_FOUND, nombreArchivo);
        return 0;
    }

    // === LEER Y VALIDAR CABECERA ===
    CabeceraVMI cabecera;
    
    // Leer los 8 bytes de la cabecera
    if (fread(&cabecera.identificador, 1, 5, archivo) != 5 ||
        fread(&cabecera.version, 1, 1, archivo) != 1 ||
        fread(&cabecera.tamMemKiB, 1, 2, archivo) != 2) {
        mostrarErrorConCodigo(VMX_ERROR_INVALID_FORMAT, "No se pudo leer la cabecera VMI");
        fclose(archivo);
        return 0;
    }

    // Validar identificador "VMI25"
    if (strncmp(cabecera.identificador, "VMI25", 5) != 0) {
        mostrarErrorConCodigo(VMX_ERROR_INVALID_FORMAT, "Identificador VMI incorrecto");
        fclose(archivo);
        return 0;
    }

    // Validar versión (por ahora sólo versión 1)
    if (cabecera.version != 1) {
        mostrarErrorConCodigo(VMX_ERROR_INVALID_VERSION, "Versión VMI no soportada");
        fclose(archivo);
        return 0;
    }

    // Convertir tamaño de memoria de big endian a little endian
    uint16_t tamMemKiB = (cabecera.tamMemKiB >> 8) | ((cabecera.tamMemKiB & 0xFF) << 8);
    size_t tamMem = (size_t)tamMemKiB * 1024;

    // Mostrar información del archivo
    printf("\n=== CARGANDO IMAGEN VMI ===\n");
    printf("Identificador: '%.5s'\n", cabecera.identificador);
    printf("Versión: %u\n", cabecera.version);
    printf("Tamaño de memoria: %u KiB (%zu bytes)\n\n", tamMemKiB, tamMem);

    // === ALOCAR/REALOCAR MEMORIA PRINCIPAL ===
    // Verificar si necesitamos crear o redimensionar la memoria
    if (cpu->mem == NULL || cpu->tamMem != tamMem) {
        // Liberar memoria anterior si existía
        if (cpu->mem != NULL) {
            free(cpu->mem);
        }
        
        // Alocar nueva memoria con el tamaño del archivo
        cpu->mem = (uint8_t*)malloc(tamMem);
        if (!cpu->mem) {
            mostrarErrorConCodigo(VMX_ERROR_MEMORY_ACCESS, "No se pudo alocar memoria");
            fclose(archivo);
            return 0;
        }
        cpu->tamMem = tamMem;
    }

    // === LEER REGISTROS ===
    // 32 registros × 4 bytes = 128 bytes totales
    if (fread(cpu->regs, 4, 32, archivo) != 32) {
        mostrarErrorConCodigo(VMX_ERROR_INVALID_FORMAT, "No se pudieron leer los registros");
        fclose(archivo);
        return 0;
    }

    // === LEER TABLA DE SEGMENTOS ===
    // Se reservan 32 bytes (8 entradas × 4 bytes)
    // aunque MV2 solo usa 6 segmentos
    uint8_t tablaSegBuffer[32];
    if (fread(tablaSegBuffer, 1, 32, archivo) != 32) {
        mostrarErrorConCodigo(VMX_ERROR_INVALID_FORMAT, "No se pudo leer la tabla de segmentos");
        fclose(archivo);
        return 0;
    }

    // Decodificar cada entrada de la tabla
    // Formato de cada entrada: [2 bytes base | 2 bytes tamaño]
    for (int i = 0; i < MAX_SEGMENTOS; i++) {
        // Leer base
        uint16_t base = (tablaSegBuffer[i*4] << 8) | tablaSegBuffer[i*4 + 1];
        // Leer tamaño
        uint16_t tamano = (tablaSegBuffer[i*4 + 2] << 8) | tablaSegBuffer[i*4 + 3];
        
        cpu->segmentos[i].base = base;
        cpu->segmentos[i].tamano = tamano;
    }

    // === LEER CONTENIDO DE MEMORIA PRINCIPAL ===
    // Leer todos los bytes de la RAM desde el archivo
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

// Implementación de guardado de imagen .vmi
int guardarImagen(const char *nombreArchivo, CPU *cpu) {
    // === ABRIR ARCHIVO PARA ESCRITURA BINARIA ===
    FILE *archivo = fopen(nombreArchivo, "wb");
    if (!archivo) {
        mostrarErrorConCodigo(VMX_ERROR_FILE_NOT_FOUND, nombreArchivo);
        return 0;
    }

    CabeceraVMI cabecera;

    memcpy(cabecera.identificador, "VMI25", 5);
    cabecera.version = 1;
    
    // Tamaño de memoria en KiB
    uint16_t tamMemKiB = (uint16_t)(cpu->tamMem / 1024);
    cabecera.tamMemKiB = (tamMemKiB >> 8) | ((tamMemKiB & 0xFF) << 8);

    // === ESCRIBIR CABECERA ===
    // Escribir los 8 bytes de la cabecera
    if (fwrite(&cabecera.identificador, 1, 5, archivo) != 5 ||
        fwrite(&cabecera.version, 1, 1, archivo) != 1 ||
        fwrite(&cabecera.tamMemKiB, 1, 2, archivo) != 2) {
        mostrarErrorConCodigo(VMX_ERROR_INVALID_FORMAT, "No se pudo escribir la cabecera VMI");
        fclose(archivo);
        return 0;
    }

    // === ESCRIBIR REGISTROS ===
    // Escribir los 32 registros x 4 bytes = 128 bytes
    if (fwrite(cpu->regs, 4, 32, archivo) != 32) {
        mostrarErrorConCodigo(VMX_ERROR_INVALID_FORMAT, "No se pudieron escribir los registros");
        fclose(archivo);
        return 0;
    }

    // === ESCRIBIR TABLA DE SEGMENTOS ===
    uint8_t tablaSegBuffer[32] = {0};
    for (int i = 0; i < MAX_SEGMENTOS; i++) {
        // Base (2 bytes)
        tablaSegBuffer[i*4]     = (cpu->segmentos[i].base >> 8) & 0xFF;
        tablaSegBuffer[i*4 + 1] = cpu->segmentos[i].base & 0xFF;
        
        // Tamaño (2 bytes)
        tablaSegBuffer[i*4 + 2] = (cpu->segmentos[i].tamano >> 8) & 0xFF;
        tablaSegBuffer[i*4 + 3] = cpu->segmentos[i].tamano & 0xFF;
    }

    // Escribir los 32 bytes de la tabla
    if (fwrite(tablaSegBuffer, 1, 32, archivo) != 32) {
        mostrarErrorConCodigo(VMX_ERROR_INVALID_FORMAT, "No se pudo escribir la tabla de segmentos");
        fclose(archivo);
        return 0;
    }

    // === ESCRIBIR MEMORIA PRINCIPAL ===
    // Volcar todo el contenido de la RAM al archivo
    if (fwrite(cpu->mem, 1, cpu->tamMem, archivo) != cpu->tamMem) {
        mostrarErrorConCodigo(VMX_ERROR_INVALID_FORMAT, "No se pudo escribir la memoria");
        fclose(archivo);
        return 0;
    }

    fclose(archivo);
    printf("\nImagen guardada en: %s\n", nombreArchivo);
    return 1;
}
