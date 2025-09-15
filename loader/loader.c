#include <stdio.h>
#include <string.h>
#include "loader.h"
#include "../dissasembler/dissasembler.h"

int cargarPrograma(const char *nombreArchivo, CPU *cpu, int mostrarDisasm) {
    FILE *archivo = fopen(nombreArchivo, "rb");
    if (!archivo) {
        mostrarError("Error: No se pudo abrir el archivo");
        mostrarError(nombreArchivo);
        return 0;
    }

    char identificador[5];
    uint8_t version;
    uint8_t tamanoBytes[2];

    if (fread(identificador, 1, 5, archivo) != 5 ||
        fread(&version, 1, 1, archivo) != 1 ||
        fread(tamanoBytes, 1, 2, archivo) != 2) {
        mostrarError("Error: No se pudo leer la cabecera");
        fclose(archivo);
        return 0;
    }

    uint16_t tamanoCodigo = (tamanoBytes[0] << 8) | tamanoBytes[1];

    printf("\n=== INFORMACIÓN DEL PROGRAMA ===\n");
    printf("Identificador: '%.5s'\n", identificador);
    printf("Versión: %d\n", version);
    printf("Bytes tamaño del código (hex): %02X %02X\n", tamanoBytes[0], tamanoBytes[1]);
    printf("Tamaño del código: %d\n\n", tamanoCodigo);

    if (strncmp(identificador, "VMX25", 5) != 0) {
        mostrarError("Error: Archivo no válido - identificador incorrecto");
        fclose(archivo);
        return 0;
    }

    if (version != 1) {
        mostrarError("Error: Versión no soportada:");
        char buffer[8];
        snprintf(buffer, sizeof(buffer), "%u", version);
        mostrarError(buffer);
        fclose(archivo);
        return 0;
    }

    if (fread(cpu->mem, 1, tamanoCodigo, archivo) != tamanoCodigo) {
        mostrarError("Error: No se pudo leer el código del programa");
        fclose(archivo);
        return 0;
    }

    fclose(archivo);

    inicializaTablaSegmentos(cpu, tamanoCodigo);
    inicializarRegistros(cpu);

    if (mostrarDisasm) {
        printf("=== DISASSEMBLER ===\n");
        mostrarDisassembler(cpu, tamanoCodigo);
        printf("\n=== EJECUCIÓN ===\n");
    }

    return 1;
}