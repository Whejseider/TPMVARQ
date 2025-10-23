#pragma once

#include "../vmx/cpu.h"

void mostrarDisassembler(CPU *cpu, uint16_t tamanoCodigo);
void mostrarMnemonico(uint8_t opcode);
void mostrarOperando(Operando *op, CPU *cpu);