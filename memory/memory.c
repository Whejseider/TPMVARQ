#include "memory.h"
#include "../utils/utils.h"

// Implementación de traducción de direcciones lógicas a físicas
uint32_t traducirDireccion(CPU *cpu, uint32_t direccionLogica, uint32_t tamano) {
    // Extraer índice de segmento (bits altos) y offset (bits bajos)
    uint16_t segmento = (direccionLogica >> 16) & 0xFFFF;
    uint16_t offset = direccionLogica & 0xFFFF;

    // Validar que el índice de segmento exista en la tabla
    if (segmento >= cpu->cantSegmentos) {
        terminarConError(VMX_ERROR_SEGMENT_FAULT, "segmento inválido");
    }

    // Calcular rango del acceso a memoria
    uint32_t inicioAcceso = offset;
    uint32_t finAcceso = inicioAcceso + tamano - 1;  // Último byte accedido
    uint32_t tamanoSegmento = cpu->segmentos[segmento].tamano;

    // Verificar que el acceso completo esté dentro de los límites del segmento
    if (finAcceso >= tamanoSegmento) {
        terminarConError(VMX_ERROR_MEMORY_ACCESS, "acceso fuera de límites");
    }

    // Calcular dirección física: base del segmento + offset
    uint32_t direccionFisica = cpu->segmentos[segmento].base + offset;

    return direccionFisica;
}

// ==== FUNCIONES DE LECTURA DE MEMORIA ====

// Implementación de lectura de 1 byte
uint8_t leerMemoria8(CPU *cpu, uint32_t direccion) {
    // Traducir dirección lógica a física
    uint32_t direccionFisica = traducirDireccion(cpu, direccion, 1);
    
    // Leer el byte desde memoria
    uint8_t valor = cpu->mem[direccionFisica];

    cpu->regs[REG_LAR] = direccion;  // Logical Address Register
    cpu->regs[REG_MAR] = (1u << 16) | (direccionFisica & 0xFFFF);  // [tamaño|dir_física]
    cpu->regs[REG_MBR] = (int32_t)(int8_t)valor;  // Memory Buffer Register

    return valor;
}

// Implementación de lectura de 2 bytes
uint16_t leerMemoria16(CPU *cpu, uint32_t direccion) {
    // Traducir y verificar que hay espacio para 2 bytes
    uint32_t direccionFisica = traducirDireccion(cpu, direccion, 2);

    // Leer bytes en orden
    uint8_t byteAlto = cpu->mem[direccionFisica];
    uint8_t byteBajo = cpu->mem[direccionFisica + 1];

    uint16_t valor = (uint16_t)((byteAlto << 8) | byteBajo);
    
    // Actualizar registros de control
    cpu->regs[REG_LAR] = direccion; // Logical Address Register
    cpu->regs[REG_MAR] = (2u << 16) | (direccionFisica & 0xFFFF); // [tamaño|dir_física]
    cpu->regs[REG_MBR] = (int32_t)(int16_t)valor;  // Memory Buffer Register
    
    return valor;
}

// Implementación de lectura de 4 bytes
uint32_t leerMemoria32(CPU *cpu, uint32_t direccion) {
    // Traducir y verificar que hay espacio para 4 bytes
    uint32_t direccionFisica = traducirDireccion(cpu, direccion, 4);

    // Leer 4 bytes
    uint32_t valor = 0;
    for (int i = 0; i < 4; i++) {
        valor = (valor << 8) | cpu->mem[direccionFisica + i];
    }

    // Actualizar registros de control
    cpu->regs[REG_LAR] = direccion; // Logical Address Register
    cpu->regs[REG_MAR] = (4u << 16) | (direccionFisica & 0xFFFF); // [tamaño|dir_física]
    cpu->regs[REG_MBR] = (int32_t)valor; // Memory Buffer Register

    return (uint32_t) valor;
}

// ==== FUNCIONES DE ESCRITURA DE MEMORIA ====

// Implementación de escritura de 1 byte
void escribirMemoria8(CPU *cpu, uint32_t direccion, uint8_t valor) {
    // Traducir dirección lógica a física
    uint32_t direccionFisica = traducirDireccion(cpu, direccion, 1);

    // Actualizar registros de control ANTES de escribir
    cpu->regs[REG_MBR] = (int32_t) (int8_t) valor;  // con extensión de signo
    cpu->regs[REG_LAR] = direccion;
    cpu->regs[REG_MAR] = (1u << 16) | (direccionFisica & 0xFFFF);
    
    // Escribir en memoria
    cpu->mem[direccionFisica] = valor;
}

// Implementación de escritura de 2 bytes
void escribirMemoria16(CPU *cpu, uint32_t direccion, uint16_t valor) {
    // Traducir dirección
    uint32_t direccionFisica = traducirDireccion(cpu, direccion, 2);

    // Actualizar registros de control
    cpu->regs[REG_MBR] = (int32_t) (int16_t) valor;  // Con extensión de signo
    cpu->regs[REG_LAR] = direccion;
    cpu->regs[REG_MAR] = (2u << 16) | (direccionFisica & 0xFFFF);

    // Escribir
    cpu->mem[direccionFisica] = (uint8_t) ((valor >> 8) & 0xFF);   // Byte alto
    cpu->mem[direccionFisica + 1] = (uint8_t) (valor & 0xFF);      // Byte bajo
}

// Implementación de escritura de 4 bytes
void escribirMemoria32(CPU *cpu, uint32_t direccion, uint32_t valor) {
    // Traducir dirección
    uint32_t direccionFisica = traducirDireccion(cpu, direccion, 4);

    // Actualizar registros de control
    cpu->regs[REG_MBR] = (int32_t) valor;
    cpu->regs[REG_LAR] = direccion;
    cpu->regs[REG_MAR] = (4u << 16) | (direccionFisica & 0xFFFF);

    for (int i = 0; i < 4; i++) {
        cpu->mem[direccionFisica + i] = (uint8_t) ((valor >> ((3 - i) * 8)) & 0xFF);
    }
}