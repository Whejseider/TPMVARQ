#include <stdio.h>
#include "syscalls.h"
#include "../memory/memory.h"
#include <string.h>

/**
 * Ejecuta una llamada al sistema según el número especificado
 */
uint32_t ejecutarSyscall(CPU *cpu, uint32_t syscall) {
    switch (syscall) {
        case SYS_READ:
            sysRead(cpu);
            break;
        case SYS_WRITE:
            sysWrite(cpu);
            break;
        default:
            fprintf(stderr, "Llamada al sistema no implementada: %d\n", syscall);
            return 0;
    }
    return 1;
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
