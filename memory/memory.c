#include "memory.h"

uint32_t traducirDireccion(CPU *cpu, uint32_t direccionLogica, uint32_t tamano) {
    uint16_t segmento = (direccionLogica >> 16) & 0xFFFF;
    int16_t offset = (int16_t) (direccionLogica & 0xFFFF);

    if (segmento >= MAX_SEGMENTOS) {
        mostrarError("Fallo de segmento: segmento inválido");
        exit(1);
    }

    int32_t inicioAcceso = offset;
    int32_t finAcceso = inicioAcceso + (int32_t) tamano;
    int32_t tamanoSegmento = (int32_t) cpu->segmentos[segmento].tamano;

    if (inicioAcceso < 0 || finAcceso > tamanoSegmento || inicioAcceso >= tamanoSegmento) {
        mostrarError("Fallo de segmento: acceso fuera de límites");
        exit(1);
    }

    uint32_t direccionFisica = cpu->segmentos[segmento].base + (uint32_t) offset;

    cpu->regs[REG_LAR] = direccionLogica;
    cpu->regs[REG_MAR] = (tamano << 16) | (direccionFisica & 0xFFFF);

    return direccionFisica;
}

// ==== ESCRITURA Y LECTURA DE MEMORIA
uint8_t leerMemoria8(CPU *cpu, uint32_t direccion) {
    uint32_t direccionFisica = traducirDireccion(cpu, direccion, 1);
    uint8_t valor = cpu->mem[direccionFisica];

    cpu->regs[REG_MBR] = (int32_t) (int8_t) valor;
    return valor;
}

void escribirMemoria8(CPU *cpu, uint32_t direccion, uint8_t valor) {
    uint32_t direccionFisica = traducirDireccion(cpu, direccion, 1);

    cpu->regs[REG_MBR] = (int32_t) (int8_t) valor;
    cpu->mem[direccionFisica] = valor;
}

uint16_t leerMemoria16(CPU *cpu, uint32_t direccion) {
    uint32_t direccionFisica = traducirDireccion(cpu, direccion, 2);

    uint8_t byteAlto = cpu->mem[direccionFisica];
    uint8_t byteBajo = cpu->mem[direccionFisica + 1];

    int16_t valor = (int16_t) ((byteAlto << 8) | byteBajo);

    cpu->regs[REG_MBR] = (int32_t) valor;
    return (uint16_t) valor;
}

void escribirMemoria16(CPU *cpu, uint32_t direccion, uint16_t valor) {
    uint32_t direccionFisica = traducirDireccion(cpu, direccion, 2);

    cpu->regs[REG_MBR] = (int32_t) (int16_t) valor;

    cpu->mem[direccionFisica] = (uint8_t) ((valor >> 8) & 0xFF);
    cpu->mem[direccionFisica + 1] = (uint8_t) (valor & 0xFF);
}

uint32_t leerMemoria32(CPU *cpu, uint32_t direccion) {
    uint32_t direccionFisica = traducirDireccion(cpu, direccion, 4);

    int32_t valor = 0;
    for (int i = 0; i < 4; i++) {
        valor |= ((int32_t) cpu->mem[direccionFisica + i]) << ((3 - i) * 8);
    }

    cpu->regs[REG_MBR] = valor;
    return (uint32_t) valor;
}

void escribirMemoria32(CPU *cpu, uint32_t direccion, uint32_t valor) {
    uint32_t direccionFisica = traducirDireccion(cpu, direccion, 4);

    cpu->regs[REG_MBR] = (int32_t) valor;

    for (int i = 0; i < 4; i++) {
        cpu->mem[direccionFisica + i] = (uint8_t) ((valor >> ((3 - i) * 8)) & 0xFF);
    }
}