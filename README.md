# TPMVARQ — Máquina virtual MV 2026 Parte 1

- Sistema operativo: Linux (Arch Linux / CachyOS)

## Requerimientos previos

- CMake 3.10 o superior
- Compilador C (gcc o clang), estándar C11
- No se usan librerías externas

## Compilar

```
cmake -B build
cmake --build build
```

Genera el ejecutable `build/TPMVARQ`.

## Ejecutar

```
./build/TPMVARQ [archivo.vmx] [-d]
```

- `archivo.vmx`: programa a ejecutar
- `-d (opcional)`: mostrar el desensamblador antes de ejecutar
