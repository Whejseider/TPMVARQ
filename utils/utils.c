#include "utils.h"

// ==== FUNCIONES DE MANEJO DE ERRORES ====

void mostrarError(const char *mensaje) {
    fprintf(stderr, "Error: %s\n", mensaje);
}

void mostrarErrorConCodigo(vmx_error_t codigo, const char *mensaje) {
    const char *codigoStr = "Error desconocido";
    
    switch (codigo) {
        case VMX_ERROR_FILE_NOT_FOUND:
            codigoStr = "Archivo no encontrado";
            break;
        case VMX_ERROR_INVALID_FORMAT:
            codigoStr = "Formato de archivo inválido";
            break;
        case VMX_ERROR_MEMORY_ACCESS:
            codigoStr = "Error de acceso a memoria";
            break;
        case VMX_ERROR_INVALID_INSTRUCTION:
            codigoStr = "Instrucción inválida";
            break;
        case VMX_ERROR_DIVISION_BY_ZERO:
            codigoStr = "División por cero";
            break;
        case VMX_ERROR_SEGMENT_FAULT:
            codigoStr = "Fallo de segmento";
            break;
        case VMX_ERROR_INVALID_VERSION:
            codigoStr = "Versión no soportada";
            break;
        case VMX_ERROR_STACK_OVERFLOW:
            codigoStr = "Stack overflow";
            break;
        case VMX_ERROR_STACK_UNDERFLOW:
            codigoStr = "Stack underflow";
            break;
        default:
            codigoStr = "Error desconocido";
            break;
    }
    
    fprintf(stderr, "Error (%s): %s\n", codigoStr, mensaje ? mensaje : "");
}

void terminarConError(vmx_error_t codigo, const char *mensaje) {
    mostrarErrorConCodigo(codigo, mensaje);
    exit(1);
}

// ==== FUNCIONES DE VALIDACIÓN ====

int validarArchivo(const char *nombreArchivo) {
    if (!nombreArchivo) {
        return 0;
    }
    
    FILE *archivo = fopen(nombreArchivo, "rb");
    if (!archivo) {
        return 0;
    }
    
    fclose(archivo);
    return 1;
}

int validarIdentificadorVMX(const char *identificador) {
    if (!identificador) {
        return 0;
    }
    
    return strncmp(identificador, "VMX25", 5) == 0;
}

int validarVersionVMX(uint8_t version) {
    // Soporta versión 1 (MV1) y versión 2 (MV2)
    return version == 1 || version == 2;
}

// ==== FUNCIONES DE CONVERSIÓN ====

void hexToString(uint32_t valor, char *buffer, size_t tamano) {
    if (!buffer || tamano < 9) {
        return;
    }
    
    snprintf(buffer, tamano, "0x%08X", valor);
}

void decimalToString(int32_t valor, char *buffer, size_t tamano) {
    if (!buffer || tamano < 12) {
        return;
    }
    
    snprintf(buffer, tamano, "%d", valor);
}

// ==== FUNCIONES DE DEBUG ====

void debugRegistro(const char *nombreRegistro, uint32_t valor) {
    if (!nombreRegistro) {
        return;
    }
    
    char hexBuffer[16];
    char decBuffer[16];
    
    hexToString(valor, hexBuffer, sizeof(hexBuffer));
    decimalToString((int32_t)valor, decBuffer, sizeof(decBuffer));
    
    printf("DEBUG [%s]: %s (%s)\n", nombreRegistro, hexBuffer, decBuffer);
}

void debugMemoria(uint32_t direccion, uint32_t valor, uint32_t tamano) {
    char dirBuffer[16];
    char valBuffer[16];
    
    hexToString(direccion, dirBuffer, sizeof(dirBuffer));
    hexToString(valor, valBuffer, sizeof(valBuffer));
    
    printf("DEBUG MEM [%s]: %s (tamaño: %u)\n", dirBuffer, valBuffer, tamano);
}
