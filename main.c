#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

#include "vmx/vmx.h"
#include "loader/loader.h"
#include "image/image.h"

/**
 * Programa principal de la máquina virtual VMX
 * 
 * Uso: vmx [archivo.vmx] [archivo.vmi] [m=M] [-d] [-p param1 param2 ... paramN]
 * -d: muestra el desensamblador antes de ejecutar
 * 
 * @param argc número de argumentos
 * @param argv array de argumentos (programa, archivo, opcionalmente -d)
 * @return código de salida (0 = éxito, 1 = error)
 */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s [archivo.vmx] [archivo.vmi] [m=M] [-d] [-p param1 ... paramN]\n", argv[0]);
        printf("  -d: Mostrar disassembler\n");
        printf("  m=M: Configurar memoria (KiB)\n");
        printf("  -p: Lista de parámetros para el programa\n");
        return 1;
    }

    int mostrarDisasm = 0;
    const char *vmxFile = NULL;
    const char *vmiFile = NULL;
    size_t memoriaKiB = 16;
    char **paramList = NULL;
    uint16_t paramCount = 0;

    for (int i = 1; i < argc; ++i) {
        char *arg = argv[i];

        if (strcmp(arg, "-d") == 0) {
            mostrarDisasm = 1;
            continue;
        }

        if (strncmp(arg, "m=", 2) == 0) {
            char *endptr = NULL;
            long value = strtol(arg + 2, &endptr, 10);
            if (endptr == arg + 2 || value <= 0 || value > INT_MAX) {
                printf("Memoria inválida: %s\n", arg);
                return 1;
            }
            memoriaKiB = (size_t) value;
            continue;
        }

        if (strcmp(arg, "-p") == 0) {
            if (i + 1 < argc) {
                paramList = &argv[i + 1];
                paramCount = (uint16_t)(argc - (i + 1));
            }
            break;
        }

        size_t len = strlen(arg);
        if (len > 4 && strcmp(arg + len - 4, ".vmx") == 0) {
            vmxFile = arg;
        } else if (len > 4 && strcmp(arg + len - 4, ".vmi") == 0) {
            vmiFile = arg;
        } else {
            printf("Parámetro desconocido: %s\n", arg);
            return 1;
        }
    }

    if (!vmxFile && !vmiFile) {
        printf("Debe especificar al menos un archivo .vmx o .vmi\n");
        return 1;
    }

    CPU cpu;
    memset(&cpu, 0, sizeof(CPU));

    int ejecucionExitosa = 0;

    if (vmxFile) {
        cpu.tamMem = memoriaKiB * 1024;
        cpu.mem = (uint8_t *) malloc(cpu.tamMem);
        if (!cpu.mem) {
            printf("No se pudo reservar memoria para la VM\n");
            return 1;
        }

        if (!cargarPrograma(vmxFile, &cpu, mostrarDisasm, paramList, paramCount)) {
            free(cpu.mem);
            return 1;
        }

        ejecucionExitosa = 1;
    } else if (vmiFile) {
        if (!cargarImagen(vmiFile, &cpu)) {
            return 1;
        }

        ejecucionExitosa = 1;
    }

    if (!ejecucionExitosa) {
        printf("No se pudo inicializar la máquina virtual\n");
        if (cpu.mem) free(cpu.mem);
        return 1;
    }

    printf("Ejecutando programa...\n\n");
    vmxRun(&cpu);
    printf("\nPrograma terminado.\n");

    if (vmiFile && vmxFile) {
        guardarImagen(vmiFile, &cpu);
    }

    if (cpu.mem) {
        free(cpu.mem);
    }

    return 0;
}