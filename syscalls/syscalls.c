#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "syscalls.h"
#include "../memory/memory.h"
#include "../image/image.h"

#ifdef _WIN32
#include <windows.h>
#endif

/**
 * Ejecuta una llamada al sistema según el número especificado
 */
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
            } else if (accion == 1) {
                return 3;
            }
            return 1;
        }
        default:
            fprintf(stderr, "Llamada al sistema no implementada: %u\n", syscall);
            return 0;
    }
}

/**
 * Lee datos del teclado y los almacena en memoria
 * 
 * Registros utilizados:
 * - EAX: modo de entrada (decimal, hex, octal, binario, char)
 * - EDX: dirección de memoria donde almacenar
 * - ECX: cantidad (bits bajos) y tamaño (bits altos)
 */
void sysRead(CPU *cpu) {
    uint32_t modo = cpu->regs[REG_EAX];
    uint32_t direccion = cpu->regs[REG_EDX];
    uint32_t config = cpu->regs[REG_ECX];
    uint16_t cantidad = config & 0xFFFF;
    uint16_t tamano = (config >> 16) & 0xFFFF;

    for (int i = 0; i < cantidad; i++) {
        uint32_t direccionLogica = direccion + (i * tamano);
        printf("[%04X]: ", direccionLogica);

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

/**
 * Muestra datos de memoria en pantalla
 * 
 * Registros utilizados:
 * - EAX: modo de salida (decimal, hex, octal, binario, char)
 * - EDX: dirección de memoria a mostrar
 * - ECX: cantidad (bits bajos) y tamaño (bits altos)
 */
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

void sysStringRead(CPU *cpu) {
    uint32_t destino = cpu->regs[REG_EDX];
    uint32_t maxLongitud = cpu->regs[REG_ECX];
    if (maxLongitud == 0 || maxLongitud > 65535) {
        maxLongitud = 65535;
    }

    char *buffer = (char *) malloc(maxLongitud + 1);
    if (!buffer) {
        fprintf(stderr, "No se pudo alocar memoria para string read\n");
        return;
    }

    printf("> ");
    if (!fgets(buffer, (int) maxLongitud + 1, stdin)) {
        buffer[0] = '\0';
    }

    size_t len = strcspn(buffer, "\n");
    buffer[len] = '\0';

    for (size_t i = 0; i <= len; ++i) {
        escribirMemoria8(cpu, destino + (uint32_t)i, (uint8_t) buffer[i]);
    }

    free(buffer);
    cpu->regs[REG_AC] = (uint32_t) len;
}

void sysStringWrite(CPU *cpu) {
    uint32_t origen = cpu->regs[REG_EDX];
    uint32_t maxLongitud = cpu->regs[REG_ECX];
    if (maxLongitud == 0 || maxLongitud > 65535) {
        maxLongitud = 65535;
    }

    for (uint32_t i = 0; i < maxLongitud; ++i) {
        uint8_t c = leerMemoria8(cpu, origen + i);
        if (c == '\0') {
            break;
        }
        putchar((int) c);
    }
    fflush(stdout);
}

void sysClearScreen(void) {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count;
    DWORD cellCount;
    COORD homeCoords = {0, 0};

    if (hConsole == INVALID_HANDLE_VALUE) return;

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return;
    cellCount = csbi.dwSize.X * csbi.dwSize.Y;

    FillConsoleOutputCharacter(hConsole, (TCHAR) ' ', cellCount, homeCoords, &count);
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, cellCount, homeCoords, &count);
    SetConsoleCursorPosition(hConsole, homeCoords);
#else
    printf("\033[2J\033[H");
#endif
}

uint32_t sysBreakpoint(CPU *cpu) {
    if (!cpu->vmiFile) {
        return 0;
    }

    if (!guardarImagen(cpu->vmiFile, cpu)) {
        fprintf(stderr, "[BREAKPOINT] Error al guardar %s\n", cpu->vmiFile);
        return 0;
    }

    printf("\n[BREAKPOINT] Estado guardado en %s\n", cpu->vmiFile);
    printf("Opciones: 'g' (go), 'q' (quit), Enter (paso a paso): ");
    fflush(stdout);

    char buffer[16];
    if (!fgets(buffer, sizeof(buffer), stdin)) {
        return 0;
    }

    if (buffer[0] == 'g' || buffer[0] == 'G') {
        return 0;
    } else if (buffer[0] == 'q' || buffer[0] == 'Q') {
        return 2;
    } else if (buffer[0] == '\n') {
        return 1;
    }

    return 0;
}

/**
 * Lee un valor según el modo especificado
 */
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

/**
 * Muestra un valor según el modo especificado
 */
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
