#include <string.h>
#include "vmx/vmx.h"
#include "loader/loader.h"

/**
 * Programa principal de la máquina virtual VMX
 * 
 * Uso: programa archivo.vmx [-d]
 * -d: muestra el desensamblador antes de ejecutar
 * 
 * @param argc número de argumentos
 * @param argv array de argumentos (programa, archivo, opcionalmente -d)
 * @return código de salida (0 = éxito, 1 = error)
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