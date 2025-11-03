#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

#include "vmx/vmx.h"
#include "loader/loader.h"
#include "image/image.h"
#include "disassembler/disassembler.h"

/**
 * Programa principal de la máquina virtual VMX - MV2
 * 
 * Esta es la función de entrada de la máquina virtual. Se encarga de:
 * 1. Parsear los argumentos de línea de comandos
 * 2. Cargar el programa (.vmx) o la imagen (.vmi)
 * 3. Configurar la memoria
 * 4. Ejecutar el programa
 * 5. Guardar el estado final si corresponde
 * 
 * Uso: vmx [archivo.vmx] [archivo.vmi] [m=M] [-d] [-p param1 param2 ... paramN]
 * 
 * Parámetros de línea de comandos:
 * - archivo.vmx: programa compilado a ejecutar (opcional si se provee .vmi)
 * - archivo.vmi: archivo de imagen para guardar/cargar estado (opcional)
 * - m=M: tamaño de memoria en KiB (por defecto MEM_DEF_KIB)
 * - -d: flag para mostrar el desensamblador antes de ejecutar
 * - -p: flag seguido de lista de parámetros para el Param Segment
 * 
 * @param argc número de argumentos de línea de comandos
 * @param argv array de strings con los argumentos
 * @return 0 si la ejecución fue exitosa, 1 si hubo error
 */
int main(const int argc, char *argv[]) {
    // Validar que se proveyeron argumentos mínimos
    if (argc < 2) {
        printf("Uso: %s [archivo.vmx] [archivo.vmi] [m=M] [-d] [-p param1 ... paramN]\n", argv[0]);
        printf("  -d: Mostrar disassembler\n");
        printf("  m=M: Configurar memoria (KiB)\n");
        printf("  -p: Lista de parámetros para el programa\n");
        return 1;
    }

    // === INICIALIZAR VARIABLES DE CONFIGURACIÓN ===
    int mostrarDisasm = 0;           // Flag para mostrar disassembler (-d)
    const char *vmxFile = NULL;      // Ruta al archivo .vmx (programa)
    const char *vmiFile = NULL;      // Ruta al archivo .vmi (imagen)
    size_t memoriaKiB = MEM_DEF_KIB; // Tamaño de memoria en KiB (por defecto)
    char **paramList = NULL;         // Lista de parámetros para Param Segment
    uint16_t paramCount = 0;         // Cantidad de parámetros

    // === PARSEAR ARGUMENTOS DE LÍNEA DE COMANDOS ===
    // Recorrer todos los argumentos (excepto argv[0] que es el nombre del programa)
    for (int i = 1; i < argc; ++i) {
        char *arg = argv[i];

        // Flag -d: activar disassembler
        if (strcmp(arg, "-d") == 0) {
            mostrarDisasm = 1;
            continue;
        }

        // Parámetro m=M: configurar tamaño de memoria
        if (strncmp(arg, "m=", 2) == 0) {
            char *endptr = NULL;
            const long value = strtol(arg + 2, &endptr, 10);
            // Validar que el valor sea numérico y positivo
            if (endptr == arg + 2 || value <= 0 || value > INT_MAX) {
                printf("Memoria inválida: %s\n", arg);
                return 1;
            }
            memoriaKiB = (size_t) value;
            continue;
        }

        // Flag -p: capturar todos los parámetros restantes para el programa
        if (strcmp(arg, "-p") == 0) {
            if (i + 1 < argc) {
                // Apuntar al primer parámetro después de -p
                paramList = &argv[i + 1];
                // Contar cuántos parámetros quedan
                paramCount = (uint16_t)(argc - (i + 1));
            }
            break;  // -p debe ser el último flag, todo lo que sigue son parámetros
        }

        // Identificar archivos por extensión
        const size_t len = strlen(arg);
        if (len > 4 && strcmp(arg + len - 4, ".vmx") == 0) {
            vmxFile = arg;  // Archivo de programa
        } else if (len > 4 && strcmp(arg + len - 4, ".vmi") == 0) {
            vmiFile = arg;  // Archivo de imagen
        } else {
            printf("Parámetro desconocido: %s\n", arg);
            return 1;
        }
    }

    // Validar que se especificó al menos un archivo de entrada
    if (!vmxFile && !vmiFile) {
        printf("Debe especificar al menos un archivo .vmx o .vmi\n");
        return 1;
    }

    // === INICIALIZAR ESTRUCTURA DE LA CPU ===
    CPU cpu = {0};
    cpu.vmiFile = vmiFile;         // Guardar ruta del archivo .vmi para breakpoints

    int ejecucionExitosa = 0;

    // === CARGAR PROGRAMA O IMAGEN ===
    if (vmxFile) {
        // Caso 1: Cargar desde archivo .vmx (programa compilado)
        
        // Alocar memoria principal según el tamaño especificado
        cpu.tamMem = memoriaKiB * 1024;  // Convertir KiB a bytes
        cpu.mem = (uint8_t *) malloc(cpu.tamMem);
        if (!cpu.mem) {
            printf("No se pudo reservar memoria para la VM\n");
            return 1;
        }

        // Cargar el programa, configurar segmentos e inicializar registros
        // Si hay parámetros (-p), se construye el Param Segment
        if (!cargarPrograma(vmxFile, &cpu, mostrarDisasm, paramList, paramCount)) {
            free(cpu.mem);
            return 1;
        }

        ejecucionExitosa = 1;
    } else if (vmiFile) {
        // Caso 2: Cargar desde archivo .vmi (imagen de estado)
        // Esto permite continuar la ejecución desde un breakpoint previo
        if (!cargarImagen(vmiFile, &cpu)) {
            return 1;
        }

        // Mostrar disassembler si se solicitó
        if (mostrarDisasm) {
            // Obtener tamaño de CS desde la tabla de segmentos
            uint32_t selectorCS = cpu.regs[REG_CS];
            uint16_t indiceCS = (uint16_t)(selectorCS >> 16);
            if (indiceCS < cpu.cantSegmentos && cpu.segmentos[indiceCS].tamano > 0) {
                printf("=== DISASSEMBLER ===\n");
                mostrarDisassembler(&cpu, cpu.segmentos[indiceCS].tamano);
                printf("\n=== EJECUCIÓN ===\n");
            }
        }

        ejecucionExitosa = 1;
    }

    // Verificar que la inicialización fue exitosa TODO
    if (!ejecucionExitosa) {
        printf("No se pudo inicializar la máquina virtual\n");
        if (cpu.mem) free(cpu.mem);
        return 1;
    }

    // === EJECUTAR PROGRAMA ===
    printf("Ejecutando programa...\n\n");
    vmxRun(&cpu);  // Bucle principal de fetch-decode-execute
    printf("\nPrograma terminado.\n");

    // === GUARDAR ESTADO FINAL ===
    // Si se especificaron ambos archivos (.vmx y .vmi), guardar el estado final
    if (vmiFile && vmxFile) {
        guardarImagen(vmiFile, &cpu);
    }

    // === LIBERAR RECURSOS ===
    if (cpu.mem) {
        free(cpu.mem);
    }

    return 0;
}