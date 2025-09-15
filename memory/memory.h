#pragma once

#include "../vmx/vmx.h"

/**
 * Traduce una dirección de memoria lógica a una dirección de memoria física
 * @param cpu
 * @param direccionLogica
 * @param tamano - cantidad de bytes a leer
 * @return
 */
uint32_t traducirDireccion(CPU *cpu, uint32_t direccionLogica, uint32_t tamano);

/**
 * Todo lo de memoria lo podría hacer en una sola función, para más adelante!!!!
 * @param cpu
 * @param direccion
 * @return
 */
uint8_t leerMemoria8(CPU *cpu, uint32_t direccion);
void escribirMemoria8(CPU *cpu, uint32_t direccion, uint8_t valor);

uint16_t leerMemoria16(CPU *cpu, uint32_t direccion);
void escribirMemoria16(CPU *cpu, uint32_t direccion, uint16_t valor);

uint32_t leerMemoria32(CPU *cpu, uint32_t direccion);
void escribirMemoria32(CPU *cpu, uint32_t direccion, uint32_t valor);
