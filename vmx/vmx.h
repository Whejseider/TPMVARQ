#pragma once

#include <stdint.h>

#include "cpu.h"

void inicializaTablaSegmentos(CPU *cpu, uint16_t tamanoCodigo);
void inicializarRegistros(CPU *cpu);
void vmxRun(CPU *cpu);

void actualizarCC(CPU *cpu, uint32_t resultado);

void sysRead(CPU *cpu);
void sysWrite(CPU *cpu);