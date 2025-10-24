#include "memory.h"
#include "../utils/utils.h"

/**
 * Traduce una dirección lógica a física usando la tabla de segmentos
 * Maneja la verificación de límites y actualiza registros de control
 */
uint32_t traducirDireccion(CPU *cpu, uint32_t direccionLogica, uint32_t tamano) {
    uint16_t segmento = (direccionLogica >> 16) & 0xFFFF;
    uint16_t offset = direccionLogica & 0xFFFF;

    if (segmento >= cpu->cantSegmentos) {
        terminarConError(VMX_ERROR_SEGMENT_FAULT, "segmento inválido");
    }

    uint32_t inicioAcceso = offset;
    uint32_t finAcceso = inicioAcceso + tamano - 1;
    uint32_t tamanoSegmento = cpu->segmentos[segmento].tamano;

    if (finAcceso >= tamanoSegmento) {
        terminarConError(VMX_ERROR_MEMORY_ACCESS, "acceso fuera de límites");
    }

    uint32_t direccionFisica = cpu->segmentos[segmento].base + offset;

    return direccionFisica;
}

// ==== FUNCIONES DE LECTURA DE MEMORIA ====

uint8_t leerMemoria8(CPU *cpu, uint32_t direccion) {
    uint32_t direccionFisica = traducirDireccion(cpu, direccion, 1);
    uint8_t valor = cpu->mem[direccionFisica];

    cpu->regs[REG_LAR] = direccion;
    cpu->regs[REG_MAR] = (1u << 16) | (direccionFisica & 0xFFFF);
    cpu->regs[REG_MBR] = (int32_t)(int8_t)valor;

    return valor;
}

uint16_t leerMemoria16(CPU *cpu, uint32_t direccion) {
    uint32_t direccionFisica = traducirDireccion(cpu, direccion, 2);

    uint8_t byteAlto = cpu->mem[direccionFisica];
    uint8_t byteBajo = cpu->mem[direccionFisica + 1];

    uint16_t valor = (uint16_t)((byteAlto << 8) | byteBajo);
    
    cpu->regs[REG_LAR] = direccion;
    cpu->regs[REG_MAR] = (2u << 16) | (direccionFisica & 0xFFFF);
    cpu->regs[REG_MBR] = (int32_t)(int16_t)valor;
    
    return valor;
}

uint32_t leerMemoria32(CPU *cpu, uint32_t direccion) {
    uint32_t direccionFisica = traducirDireccion(cpu, direccion, 4);

    uint32_t valor = 0;
    for (int i = 0; i < 4; i++) {
        valor = (valor << 8) | cpu->mem[direccionFisica + i];
    }

    cpu->regs[REG_LAR] = direccion;
    cpu->regs[REG_MAR] = (4u << 16) | (direccionFisica & 0xFFFF);
    cpu->regs[REG_MBR] = (int32_t)valor;

    return (uint32_t) valor;
}

// ==== FUNCIONES DE ESCRITURA DE MEMORIA ====

void escribirMemoria8(CPU *cpu, uint32_t direccion, uint8_t valor) {
    uint32_t direccionFisica = traducirDireccion(cpu, direccion, 1);

    cpu->regs[REG_MBR] = (int32_t) (int8_t) valor;
    cpu->regs[REG_LAR] = direccion;
    cpu->regs[REG_MAR] = (1u << 16) | (direccionFisica & 0xFFFF);
    cpu->mem[direccionFisica] = valor;
}

void escribirMemoria16(CPU *cpu, uint32_t direccion, uint16_t valor) {
    uint32_t direccionFisica = traducirDireccion(cpu, direccion, 2);

    cpu->regs[REG_MBR] = (int32_t) (int16_t) valor;
    cpu->regs[REG_LAR] = direccion;
    cpu->regs[REG_MAR] = (2u << 16) | (direccionFisica & 0xFFFF);

    cpu->mem[direccionFisica] = (uint8_t) ((valor >> 8) & 0xFF);
    cpu->mem[direccionFisica + 1] = (uint8_t) (valor & 0xFF);
}

void escribirMemoria32(CPU *cpu, uint32_t direccion, uint32_t valor) {
    uint32_t direccionFisica = traducirDireccion(cpu, direccion, 4);

    cpu->regs[REG_MBR] = (int32_t) valor;
    cpu->regs[REG_LAR] = direccion;
    cpu->regs[REG_MAR] = (4u << 16) | (direccionFisica & 0xFFFF);

    for (int i = 0; i < 4; i++) {
        cpu->mem[direccionFisica + i] = (uint8_t) ((valor >> ((3 - i) * 8)) & 0xFF);
    }
}