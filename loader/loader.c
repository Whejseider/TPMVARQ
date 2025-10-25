#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "loader.h"
#include "../vmx/vmx.h"
#include "../dissasembler/dissasembler.h"
#include "../utils/utils.h"

static int leerUint16BE(FILE *archivo, uint16_t *out) {
    uint8_t buffer[2];
    if (fread(buffer, 1, 2, archivo) != 2) {
        return 0;
    }
    *out = (uint16_t)((buffer[0] << 8) | buffer[1]);
    return 1;
}

int cargarPrograma(const char *nombreArchivo, CPU *cpu, int mostrarDisasm, char **params, uint16_t paramCount) {
    if (!validarArchivo(nombreArchivo)) {
        mostrarErrorConCodigo(VMX_ERROR_FILE_NOT_FOUND, nombreArchivo);
        return 0;
    }

    FILE *archivo = fopen(nombreArchivo, "rb");

    if (cpu->mem == NULL || cpu->tamMem == 0) {
        cpu->mem = (uint8_t *) malloc(MEM_DEF_BYTES);
        if (!cpu->mem) {
            mostrarErrorConCodigo(VMX_ERROR_MEMORY_ACCESS, "No se pudo alocar memoria");
            fclose(archivo);
            return 0;
        }
        cpu->tamMem = MEM_DEF_BYTES;
    }

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

    LayoutSegmentos layout = {0};
    layout.tamanoCS = (uint16_t)((tamanoBytes[0] << 8) | tamanoBytes[1]);

    size_t totalStrings = 0;
    uint32_t *paramOffsets = NULL;
    uint32_t argvOffset = 0;

    if (paramCount > 0) {
        paramOffsets = (uint32_t *) malloc(sizeof(uint32_t) * paramCount);
        if (!paramOffsets) {
            mostrarErrorConCodigo(VMX_ERROR_MEMORY_ACCESS, "No se pudo alocar memoria para parámetros");
            fclose(archivo);
            return 0;
        }

        for (uint16_t i = 0; i < paramCount; ++i) {
            totalStrings += strlen(params[i]) + 1;
        }

        size_t totalParamBytes = totalStrings + (size_t)paramCount * 4;
        if (totalParamBytes > UINT16_MAX) {
            free(paramOffsets);
            mostrarErrorConCodigo(VMX_ERROR_MEMORY_ACCESS, "Segmento de parámetros demasiado grande");
            fclose(archivo);
            return 0;
        }

        layout.tamanoPS = (uint16_t) totalParamBytes;
    }

    printf("\n=== INFORMACIÓN DEL PROGRAMA ===\n");
    printf("Identificador: '%.5s'\n", identificador);
    printf("Versión: %d\n", version);
    printf("Bytes tamaño del código (hex): %02X %02X\n", tamanoBytes[0], tamanoBytes[1]);
    printf("Tamaño del código: %d\n\n", layout.tamanoCS);

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
        if (paramOffsets) free(paramOffsets);
        return 0;
    }

    if (version == 1) {
        layout.tamanoKS = 0;
        layout.tamanoES = 0;
        layout.tamanoSS = 0;
        size_t restante = (cpu->tamMem > (size_t)layout.tamanoPS + layout.tamanoCS)
                          ? cpu->tamMem - ((size_t)layout.tamanoPS + layout.tamanoCS)
                          : 0;
        layout.tamanoDS = (uint16_t)(restante > UINT16_MAX ? UINT16_MAX : restante);
        layout.entryPoint = 0;
    } else {
        if (!leerUint16BE(archivo, &layout.tamanoDS) ||
            !leerUint16BE(archivo, &layout.tamanoES) ||
            !leerUint16BE(archivo, &layout.tamanoSS) ||
            !leerUint16BE(archivo, &layout.tamanoKS) ||
            !leerUint16BE(archivo, &layout.entryPoint)) {
            mostrarErrorConCodigo(VMX_ERROR_INVALID_FORMAT, "No se pudo leer la cabecera MV2");
            fclose(archivo);
            if (paramOffsets) free(paramOffsets);
            return 0;
        }
    }

    uint16_t tamanos[MAX_SEGMENTOS] = {
            layout.tamanoPS,
            layout.tamanoKS,
            layout.tamanoCS,
            layout.tamanoDS,
            layout.tamanoES,
            layout.tamanoSS
    };

    size_t totalProceso = 0;
    for (int i = 0; i < MAX_SEGMENTOS; ++i) {
        totalProceso += tamanos[i];
    }

    if (totalProceso > cpu->tamMem) {
        mostrarErrorConCodigo(VMX_ERROR_MEMORY_ACCESS, "Memoria insuficiente para segmentos");
        fclose(archivo);
        if (paramOffsets) free(paramOffsets);
        return 0;
    }

    uint32_t bases[MAX_SEGMENTOS];
    uint32_t cursor = 0;
    for (int i = 0; i < MAX_SEGMENTOS; ++i) {
        if (tamanos[i] == 0) {
            bases[i] = UINT32_MAX;
            continue;
        }
        bases[i] = cursor;
        cursor += tamanos[i];
    }

    memset(cpu->mem, 0, cpu->tamMem);

    if (layout.tamanoPS > 0 && bases[SEG_PS] != UINT32_MAX) {
        uint32_t offset = 0;
        for (uint16_t i = 0; i < paramCount; ++i) {
            size_t len = strlen(params[i]);
            memcpy(cpu->mem + bases[SEG_PS] + offset, params[i], len);
            cpu->mem[bases[SEG_PS] + offset + len] = '\0';
            paramOffsets[i] = offset;
            offset += (uint32_t)(len + 1);
        }
        argvOffset = offset;
    }

    if (layout.tamanoCS > 0) {
        if (bases[SEG_CS] == UINT32_MAX ||
            fread(cpu->mem + bases[SEG_CS], 1, layout.tamanoCS, archivo) != layout.tamanoCS) {
            mostrarErrorConCodigo(VMX_ERROR_INVALID_FORMAT, "No se pudo leer el código del programa");
            fclose(archivo);
            if (paramOffsets) free(paramOffsets);
            return 0;
        }
    }

    if (version == 2 && layout.tamanoKS > 0) {
        if (bases[SEG_KS] == UINT32_MAX ||
            fread(cpu->mem + bases[SEG_KS], 1, layout.tamanoKS, archivo) != layout.tamanoKS) {
            mostrarErrorConCodigo(VMX_ERROR_INVALID_FORMAT, "No se pudo leer el segmento de constantes");
            fclose(archivo);
            if (paramOffsets) free(paramOffsets);
            return 0;
        }
    }

    fclose(archivo);

    inicializaTablaSegmentos(cpu, &layout);

    uint16_t argc = paramCount;
    uint32_t argvPtr = 0xFFFFFFFF;

    if (layout.tamanoPS > 0 && bases[SEG_PS] != UINT32_MAX) {
        int psIndex = -1;
        for (uint16_t idx = 0; idx < cpu->cantSegmentos; ++idx) {
            if (cpu->segmentos[idx].base == bases[SEG_PS]) {
                psIndex = idx;
                break;
            }
        }

        if (psIndex != -1) {
            for (uint16_t i = 0; i < paramCount; ++i) {
                uint32_t valor = ((uint32_t) psIndex << 16) | (paramOffsets[i] & 0xFFFF);
                uint32_t destino = bases[SEG_PS] + argvOffset + (uint32_t)i * 4;
                cpu->mem[destino] = (uint8_t)((valor >> 24) & 0xFF);
                cpu->mem[destino + 1] = (uint8_t)((valor >> 16) & 0xFF);
                cpu->mem[destino + 2] = (uint8_t)((valor >> 8) & 0xFF);
                cpu->mem[destino + 3] = (uint8_t)(valor & 0xFF);
            }

            argvPtr = ((uint32_t) psIndex << 16) | (argvOffset & 0xFFFF);
        }
    }

    if (paramOffsets) {
        free(paramOffsets);
    }

    inicializarRegistros(cpu, &layout, argc, argvPtr);

    if (mostrarDisasm && layout.tamanoCS > 0) {
        printf("=== DISASSEMBLER ===\n");
        mostrarDisassembler(cpu, layout.tamanoCS);
        printf("\n=== EJECUCIÓN ===\n");
    }

    return 1;
}