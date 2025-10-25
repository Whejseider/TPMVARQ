#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "loader.h"
#include "../vmx/vmx.h"
#include "../disassembler/disassembler.h"
#include "../utils/utils.h"

// ==== FUNCIONES AUXILIARES ====

// Lee un uint16 del archivo en el formato correcto
static int leerUint16BE(FILE *archivo, uint16_t *out) {
    uint8_t buffer[2];
    if (fread(buffer, 1, 2, archivo) != 2) {
        return 0;
    }
    *out = (uint16_t)((buffer[0] << 8) | buffer[1]);
    return 1;
}

// ==== FUNCIÓN PRINCIPAL DE CARGA ====

int cargarPrograma(const char *nombreArchivo, CPU *cpu, int mostrarDisasm, char **params, uint16_t paramCount) {
    // === 1. VALIDAR Y ABRIR ARCHIVO ===
    if (!validarArchivo(nombreArchivo)) {
        mostrarErrorConCodigo(VMX_ERROR_FILE_NOT_FOUND, nombreArchivo);
        return 0;
    }

    FILE *archivo = fopen(nombreArchivo, "rb");

    // === 2. ASEGURAR QUE HAY MEMORIA DISPONIBLE ===
    // Si no hay memoria alocada, usar tamaño por defecto
    if (cpu->mem == NULL || cpu->tamMem == 0) {
        cpu->mem = (uint8_t *) malloc(MEM_DEF_BYTES);
        if (!cpu->mem) {
            mostrarErrorConCodigo(VMX_ERROR_MEMORY_ACCESS, "No se pudo alocar memoria");
            fclose(archivo);
            return 0;
        }
        cpu->tamMem = MEM_DEF_BYTES;
    }

    // === 3. LEER CABECERA INICIAL (8 BYTES) ===
    // Todos los archivos VMX tienen: identificador(5) + version(1) + tamaño(2)
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

    // === 4. PREPARAR LAYOUT DE SEGMENTOS ===
    LayoutSegmentos layout = {0};
    // Convertir bytes a uint16 (byte alto primero)
    layout.tamanoCS = (uint16_t)((tamanoBytes[0] << 8) | tamanoBytes[1]);

    // === 5. CONSTRUIR PARAM SEGMENT (SI HAY PARÁMETROS) ===
    // Estructura: [string1\0][string2\0]...[ptr1][ptr2]...
    size_t totalStrings = 0;
    uint32_t *paramOffsets = NULL;
    uint32_t argvOffset = 0;  // Offset donde comienza el array de punteros

    if (paramCount > 0) {
        // Alocar memoria para guardar offsets de cada string
        paramOffsets = (uint32_t *) malloc(sizeof(uint32_t) * paramCount);
        if (!paramOffsets) {
            mostrarErrorConCodigo(VMX_ERROR_MEMORY_ACCESS, "No se pudo alocar memoria para parámetros");
            fclose(archivo);
            return 0;
        }

        // Calcular espacio total necesario para los strings
        for (uint16_t i = 0; i < paramCount; ++i) {
            totalStrings += strlen(params[i]) + 1;  // +1 para null terminator
        }

        // Calcular tamaño total: strings + array de punteros (4 bytes cada uno)
        size_t totalParamBytes = totalStrings + (size_t)paramCount * 4;
        if (totalParamBytes > UINT16_MAX) {
            free(paramOffsets);
            mostrarErrorConCodigo(VMX_ERROR_MEMORY_ACCESS, "Segmento de parámetros demasiado grande");
            fclose(archivo);
            return 0;
        }

        layout.tamanoPS = (uint16_t) totalParamBytes;
    }

    // === 6. MOSTRAR INFORMACIÓN DEL ARCHIVO ===
    // Mostrar información básica del archivo cargado
    printf("\n=== INFORMACIÓN DEL PROGRAMA ===");
    printf("Identificador: '%.5s'\n", identificador);
    printf("Versión: %d\n", version);
    printf("Bytes tamaño del código (hex): %02X %02X\n", tamanoBytes[0], tamanoBytes[1]);
    printf("Tamaño del código: %d\n\n", layout.tamanoCS);

    // === 7. VALIDAR CABECERA ===
    // Verificar que la cabecera sea válida
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

    // === 8. LEER RESTO DE LA CABECERA SEGÚN VERSIÓN ===
    if (version == 1) {
        // MV1: Solo tiene CS, el resto se calcula o es 0
        layout.tamanoKS = 0;
        layout.tamanoES = 0;
        layout.tamanoSS = 0;
        size_t restante = (cpu->tamMem > (size_t)layout.tamanoPS + layout.tamanoCS)
                          ? cpu->tamMem - ((size_t)layout.tamanoPS + layout.tamanoCS)
                          : 0;
        layout.tamanoDS = (uint16_t)(restante > UINT16_MAX ? UINT16_MAX : restante);
        layout.entryPoint = 0;  // Siempre empieza en offset 0
    } else {
        // MV2: Leer tamaños de todos los segmentos y entry point
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

    // === 9. PREPARAR ARRAY DE TAMAÑOS EN ORDEN ===
    // Orden: PS, KS, CS, DS, ES, SS
    uint16_t tamanos[MAX_SEGMENTOS] = {
            layout.tamanoPS,
            layout.tamanoKS,
            layout.tamanoCS,
            layout.tamanoDS,
            layout.tamanoES,
            layout.tamanoSS
    };

    // === 10. VERIFICAR QUE CABEN TODOS LOS SEGMENTOS EN MEMORIA ===
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

    // === 11. CALCULAR DIRECCIONES BASE DE CADA SEGMENTO ===
    // Los segmentos se colocan consecutivamente en memoria
    uint32_t bases[MAX_SEGMENTOS];
    uint32_t cursor = 0;  // Dirección física actual
    for (int i = 0; i < MAX_SEGMENTOS; ++i) {
        if (tamanos[i] == 0) {
            // Segmento vacío: marcar como no existente
            bases[i] = UINT32_MAX;
            continue;
        }
        // Asignar base y avanzar cursor
        bases[i] = cursor;
        cursor += tamanos[i];
    }

    // === 12. LIMPIAR MEMORIA ===
    memset(cpu->mem, 0, cpu->tamMem);

    // === 13. CONSTRUIR PARAM SEGMENT ===
    // Primero los strings, luego el array de punteros
    if (layout.tamanoPS > 0 && bases[SEG_PS] != UINT32_MAX) {
        uint32_t offset = 0;  // Offset actual dentro de PS
        // Copiar todos los strings null-terminated
        for (uint16_t i = 0; i < paramCount; ++i) {
            size_t len = strlen(params[i]);
            // Copiar string a memoria
            memcpy(cpu->mem + bases[SEG_PS] + offset, params[i], len);
            cpu->mem[bases[SEG_PS] + offset + len] = '\0';  // Null terminator
            paramOffsets[i] = offset;  // Guardar offset para construir punteros después
            offset += (uint32_t)(len + 1);
        }
        argvOffset = offset;  // Aquí comienza el array de punteros
    }

    // === 14. CARGAR CODE SEGMENT DESDE ARCHIVO ===
    if (layout.tamanoCS > 0) {
        if (bases[SEG_CS] == UINT32_MAX ||
            fread(cpu->mem + bases[SEG_CS], 1, layout.tamanoCS, archivo) != layout.tamanoCS) {
            mostrarErrorConCodigo(VMX_ERROR_INVALID_FORMAT, "No se pudo leer el código del programa");
            fclose(archivo);
            if (paramOffsets) free(paramOffsets);
            return 0;
        }
    }

    // === 15. CARGAR KONSTANT SEGMENT DESDE ARCHIVO (SOLO MV2) ===
    if (version == 2 && layout.tamanoKS > 0) {
        if (bases[SEG_KS] == UINT32_MAX ||
            fread(cpu->mem + bases[SEG_KS], 1, layout.tamanoKS, archivo) != layout.tamanoKS) {
            mostrarErrorConCodigo(VMX_ERROR_INVALID_FORMAT, "No se pudo leer el segmento de constantes");
            fclose(archivo);
            if (paramOffsets) free(paramOffsets);
            return 0;
        }
    }

    fclose(archivo);  // Ya no necesitamos el archivo

    // === 16. CREAR TABLA DE DESCRIPTORES DE SEGMENTOS ===
    inicializaTablaSegmentos(cpu, &layout);

    // === 17. PREPARAR ARGC Y ARGV PARA LA PILA INICIAL ===
    uint16_t argc = paramCount;
    uint32_t argvPtr = 0xFFFFFFFF;  // Null pointer por defecto

    // Construir array de punteros a los strings en PS
    if (layout.tamanoPS > 0 && bases[SEG_PS] != UINT32_MAX) {
        // Buscar el índice del segmento PS en la tabla de descriptores
        int psIndex = -1;
        for (uint16_t idx = 0; idx < cpu->cantSegmentos; ++idx) {
            if (cpu->segmentos[idx].base == bases[SEG_PS]) {
                psIndex = idx;
                break;
            }
        }

        if (psIndex != -1) {
            // Escribir array de punteros (direcciones lógicas [segmento:offset])
            for (uint16_t i = 0; i < paramCount; ++i) {
                // Construir dirección lógica: [índice_segmento:offset]
                uint32_t valor = ((uint32_t) psIndex << 16) | (paramOffsets[i] & 0xFFFF);
                uint32_t destino = bases[SEG_PS] + argvOffset + (uint32_t)i * 4;
                // Escribir puntero en formato 4 bytes
                cpu->mem[destino] = (uint8_t)((valor >> 24) & 0xFF);
                cpu->mem[destino + 1] = (uint8_t)((valor >> 16) & 0xFF);
                cpu->mem[destino + 2] = (uint8_t)((valor >> 8) & 0xFF);
                cpu->mem[destino + 3] = (uint8_t)(valor & 0xFF);
            }

            // Dirección lógica del array de punteros
            argvPtr = ((uint32_t) psIndex << 16) | (argvOffset & 0xFFFF);
        }
    }

    // === 18. LIBERAR MEMORIA TEMPORAL ===
    if (paramOffsets) {
        free(paramOffsets);
    }

    // === 19. INICIALIZAR REGISTROS Y PILA ===
    inicializarRegistros(cpu, &layout, argc, argvPtr);

    // === 20. MOSTRAR DISASSEMBLER (SI SE SOLICITÓ) ===
    if (mostrarDisasm && layout.tamanoCS > 0) {
        printf("=== DISASSEMBLER ===\n");
        mostrarDisassembler(cpu, layout.tamanoCS);
        printf("\n=== EJECUCIÓN ===\n");
    }

    return 1;
}