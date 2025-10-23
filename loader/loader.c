#include <stdio.h>
#include <string.h>
#include "loader.h"
#include "../vmx/vmx.h"
#include "../dissasembler/dissasembler.h"
#include "../utils/utils.h"

int cargarPrograma(const char *nombreArchivo, CPU *cpu, int mostrarDisasm) {
    if (!validarArchivo(nombreArchivo)) {
        mostrarErrorConCodigo(VMX_ERROR_FILE_NOT_FOUND, nombreArchivo);
        return 0;
    }

    FILE *archivo = fopen(nombreArchivo, "rb");

    char identificador[5];
    uint8_t version;
    uint8_t tamanoBytes[2];

    if (fread(identificador, 1, 5, archivo) != 5 ||
        fread(&version, 1, 1, archivo) != 1 ||
        fread(tamanoBytes, 1, 2, archivo) != 2) {
        mostrarErrorConCodigo(VMX_ERROR_INVALID_FORMAT, "No se pudo leer la cabecera");
        fclose(archivo);
        return 0;
    }

    uint16_t tamanoCodigo = (tamanoBytes[0] << 8) | tamanoBytes[1];

    printf("\n=== INFORMACIÓN DEL PROGRAMA ===\n");
    printf("Identificador: '%.5s'\n", identificador);
    printf("Versión: %d\n", version);
    printf("Bytes tamaño del código (hex): %02X %02X\n", tamanoBytes[0], tamanoBytes[1]);
    printf("Tamaño del código: %d\n\n", tamanoCodigo);

    if (!validarIdentificadorVMX(identificador)) {
        mostrarErrorConCodigo(VMX_ERROR_INVALID_FORMAT, "identificador incorrecto");
        fclose(archivo);
        return 0;
    }

    if (!validarVersionVMX(version)) {
        char buffer[8];
        snprintf(buffer, sizeof(buffer), "%u", version);
        mostrarErrorConCodigo(VMX_ERROR_INVALID_VERSION, buffer);
        fclose(archivo);
        return 0;
    }

    if (fread(cpu->mem, 1, tamanoCodigo, archivo) != tamanoCodigo) {
        mostrarErrorConCodigo(VMX_ERROR_INVALID_FORMAT, "No se pudo leer el código del programa");
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