#pragma once

#include <stdint.h>

#include "cpu.h"

void inicializaTablaSegmentos(CPU *cpu, LayoutSegmentos *layout);
void inicializarRegistros(CPU *cpu, LayoutSegmentos *layout, uint16_t argc, uint32_t argvPtr);
void vmxRun(CPU *cpu);

void actualizarCC(CPU *cpu, uint32_t resultado);

void sysRead(CPU *cpu);
void sysWrite(CPU *cpu);