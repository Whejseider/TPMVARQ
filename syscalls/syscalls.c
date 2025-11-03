#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "syscalls.h"
#include "../memory/memory.h"
#include "../image/image.h"

#ifdef _WIN32
#include <windows.h>
#endif

// Implementación del dispatcher de syscalls
uint32_t ejecutarSyscall(CPU *cpu, uint32_t syscall) {
    switch (syscall) {
        case SYS_READ:
            sysRead(cpu);
            return 1;
        case SYS_WRITE:
            sysWrite(cpu);
            return 1;
        case SYS_STRING_READ:
            sysStringRead(cpu);
            return 1;
        case SYS_STRING_WRITE:
            sysStringWrite(cpu);
            return 1;
        case SYS_CLEAR_SCREEN:
            sysClearScreen();
            return 1;
        case SYS_BREAKPOINT: {
            uint32_t accion = sysBreakpoint(cpu);
            if (accion == 2) {
                cpu->ejecutando = 0;
                return 0;
            }
            if (accion == 1) {
                return 3;
            }
            return 1;
        }
        default:
            fprintf(stderr, "Llamada al sistema no implementada: %u\n", syscall);
            return 0;
    }
}

// Implementación de SYS 0x01: READ
void sysRead(CPU *cpu) {
    uint32_t modo = cpu->regs[REG_EAX];
    uint32_t direccion = cpu->regs[REG_EDX];
    uint32_t config = cpu->regs[REG_ECX];
    uint16_t cantidad = config & 0xFFFF;
    uint16_t tamano = (config >> 16) & 0xFFFF;

    for (int i = 0; i < cantidad; i++) {
        uint32_t direccionLogica = direccion + (i * tamano);
        uint32_t direccionFisica = traducirDireccion(cpu, direccionLogica, tamano);
        printf("[%04X]: ", direccionFisica);

        int32_t valor = leerValorPorModo(modo);

        if (tamano == 1) {
            escribirMemoria8(cpu, direccionLogica, (uint8_t) valor);
        } else if (tamano == 2) {
            escribirMemoria16(cpu, direccionLogica, (uint16_t) valor);
        } else if (tamano == 4) {
            escribirMemoria32(cpu, direccionLogica, (uint32_t) valor);
        }
    }
}

// Implementación de SYS 0x02: WRITE
void sysWrite(CPU *cpu) {
    uint32_t modo = cpu->regs[REG_EAX];
    uint32_t direccion = cpu->regs[REG_EDX];
    uint32_t config = cpu->regs[REG_ECX];
    uint16_t cantidad = config & 0xFFFF;
    uint16_t tamano = (config >> 16) & 0xFFFF;

    for (int i = 0; i < cantidad; i++) {
        uint32_t direccionLogica = direccion + (i * tamano);
        uint32_t direccionFisica = traducirDireccion(cpu, direccionLogica, tamano);
        int32_t valor = 0;

        if (tamano == 1) {
            valor = leerMemoria8(cpu, direccionLogica);
        } else if (tamano == 2) {
            int16_t valor16_t = (int16_t) leerMemoria16(cpu, direccionLogica);
            valor = (int32_t) valor16_t;
        } else if (tamano == 4) {
            valor = (int32_t) leerMemoria32(cpu, direccionLogica);
        }

        printf("[%04X]: ", direccionFisica);
        mostrarValorPorModo(valor, modo, tamano);
        printf("\n");
    }
}

// Implementación de SYS 0x03: STRING READ (MV2)
void sysStringRead(CPU *cpu) {
    // Obtener parámetros de registros
    uint32_t destino = cpu->regs[REG_EDX];        // Dónde guardar
    uint32_t maxLongitud = cpu->regs[REG_ECX];   // Límite de lectura
    
    // Validar/ajustar límite
    if (maxLongitud == 0 || maxLongitud > 65535) {
        maxLongitud = 65535;  // Límite por defecto
    }

    // Alocar buffer temporal para la lectura
    char *buffer = (char *) malloc(maxLongitud + 1);
    if (!buffer) {
        fprintf(stderr, "No se pudo alocar memoria para string read\n");
        return;
    }

    // Mostrar prompt y leer desde stdin
    printf("> ");
    if (!fgets(buffer, (int) maxLongitud + 1, stdin)) {
        buffer[0] = '\0';  // Error de lectura, string vacío
    }

    // Remover salto de línea si existe
    size_t len = strcspn(buffer, "\n");
    buffer[len] = '\0';

    // Copiar string a memoria byte por byte (incluyendo '\0' final)
    for (size_t i = 0; i <= len; ++i) {
        escribirMemoria8(cpu, destino + (uint32_t)i, (uint8_t) buffer[i]);
    }

    // Liberar buffer temporal
    free(buffer);

    cpu->regs[REG_AC] = (uint32_t) len;
}

// Implementación de SYS 0x04: STRING WRITE
void sysStringWrite(CPU *cpu) {
    uint32_t origen = cpu->regs[REG_EDX];
    uint32_t maxLongitud = cpu->regs[REG_ECX];
    if (maxLongitud == 0 || maxLongitud > 65535) {
        maxLongitud = 65535;
    }

    // Leer e imprimir caracteres hasta '\0' o límite
    for (uint32_t i = 0; i < maxLongitud; ++i) {
        uint8_t c = leerMemoria8(cpu, origen + i);
        
        // Terminar al encontrar \0
        if (c == '\0') {
            break;
        }
        
        // Imprimir carácter
        putchar((int) c);
    }
    
    // Forzar salida inmediata
    fflush(stdout);
}

// Implementación de SYS 0x07: CLEAR SCREEN (MV2)
void sysClearScreen(void) {
#ifdef _WIN32
    // === LIMPIEZA EN WINDOWS ===
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count;
    DWORD cellCount;
    COORD homeCoords = {0, 0};  // Posición (0,0) = esquina superior izquierda

    // Validar handle de consola
    if (hConsole == INVALID_HANDLE_VALUE) return;

    // Obtener información del buffer de consola
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return;
    
    // Calcular cantidad total de celdas de caracteres
    cellCount = csbi.dwSize.X * csbi.dwSize.Y;

    // Llenar toda la consola con espacios
    FillConsoleOutputCharacter(hConsole, (TCHAR) ' ', cellCount, homeCoords, &count);
    
    // Restaurar atributos de color
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, cellCount, homeCoords, &count);
    
    // Mover cursor a la esquina superior izquierda
    SetConsoleCursorPosition(hConsole, homeCoords);
#else
    // === LIMPIEZA EN UNIX/LINUX ===
    // \033[2J = limpiar pantalla completa
    // \033[H  = mover cursor a home (0,0)
    printf("\033[2J\033[H");
#endif
}

// Implementación de SYS 0x0F: BREAKPOINT (MV2)
uint32_t sysBreakpoint(CPU *cpu) {
    // Verificar que se especificó archivo .vmi
    if (!cpu->vmiFile) {
        return 0;
    }

    // Guardar estado actual en archivo .vmi
    if (!guardarImagen(cpu->vmiFile, cpu)) {
        fprintf(stderr, "[BREAKPOINT] Error al guardar %s\n", cpu->vmiFile);
        return 0;
    }

    // Mostrar mensaje y opciones
    printf("\n[BREAKPOINT] Estado guardado en %s\n", cpu->vmiFile);
    printf("Opciones: 'g' (go), 'q' (quit), Enter (paso a paso): ");
    fflush(stdout);

    // Leer respuesta del usuario
    char buffer[16];
    if (!fgets(buffer, sizeof(buffer), stdin)) {
        return 0;  // Error de lectura, continuar
    }

    // Procesar respuesta
    if (buffer[0] == 'g' || buffer[0] == 'G') {
        // GO: Continuar ejecución normal
        return 0;
    } else if (buffer[0] == 'q' || buffer[0] == 'Q') {
        // QUIT: Abortar ejecución
        return 2;
    } else if (buffer[0] == '\n') {
        // ENTER: Activar modo paso a paso
        return 1;
    }

    // Cualquier otra entrada: continuar normal
    return 0;
}

// Función auxiliar: lee valor desde teclado según modo
int32_t leerValorPorModo(uint32_t modo) {
    int32_t valor = 0;

    if (modo & MODE_BINARY) {
        char binStr[33];
        printf("Ingrese un numero en binario: ");
        scanf("%32s", binStr);
        valor = strtol(binStr, NULL, 2);
    } else if (modo & MODE_HEX) {
        printf("Ingrese un numero en hexadecimal: ");
        scanf("%x", (unsigned int *) &valor);
    } else if (modo & MODE_OCTAL) {
        printf("Ingrese un numero en octal: ");
        scanf("%o", (unsigned int *) &valor);
    } else if (modo & MODE_CHAR) {
        char c;
        printf("Ingrese un caracter: ");
        scanf(" %c", &c);
        valor = (int32_t) c;
    } else if (modo & MODE_DECIMAL) {
        printf("Ingrese un numero en decimal: ");
        scanf("%d", &valor);
    }

    return valor;
}

// Función auxiliar: muestra valor en uno o más formatos
void mostrarValorPorModo(int32_t valor, uint32_t modo, uint16_t tamano) {
    if (modo & MODE_BINARY) {
        if (modo & 0x0F) printf(" ");
        printf("0b");
        for (int bit = 31; bit >= 0; bit--) {
            printf("%d", (valor >> bit) & 1);
        }
    }
    if (modo & MODE_HEX) {
        if (modo & 0x07) printf(" ");
        printf("0x%X", (unsigned int) valor);
    }
    if (modo & MODE_OCTAL) {
        if (modo & 0x03) printf(" ");
        printf("0o%o", (unsigned int) valor);
    }
    if (modo & MODE_CHAR) {
        if (modo & 0x01) printf(" ");
        printf(" '");
        for (int byte = (tamano - 1) * 8; byte >= 0; byte -= 8) {
            char c = (valor >> byte) & 0xFF;
            if (c >= 32 && c <= 126) {
                printf("%c", c);
            } else {
                printf(".");
            }
        }
        printf("'");
    }
    if (modo & MODE_DECIMAL) {
        printf(" %d", valor);
    }
}
