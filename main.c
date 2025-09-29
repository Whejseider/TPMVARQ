#include <string.h>
#include "vmx/vmx.h"
#include "loader/loader.h"

/**
 * Programa principal
 * Admite 2 parámetros, el archivo y el flag [-d] (sin corchetes)
 * @param argc cantidad de argumentos
 * @param argv en dónde están ubicados los argumentos (incluye el programa .exe,
 * así que serian si se incluye el [-d] 3 parámetros)
 * @return
 */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s archivo.vmx [-d]\n", argv[0]);
        printf("  -d: Mostrar disassembler\n");
        return 1;
    }

    int mostrarDisasm = 0;
    if (argc > 2 && strcmp(argv[2], "-d") == 0) {
        mostrarDisasm = 1;
    }

    CPU cpu;
    memset(&cpu, 0, sizeof(CPU));

    if (!cargarPrograma(argv[1], &cpu, mostrarDisasm)) {
        return 1;
    }

    printf("Ejecutando programa...\n\n");
    vmxRun(&cpu);
    printf("\nPrograma terminado.\n");

    return 0;
}