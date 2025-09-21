# Trabajo práctico - Máquina Virtual

La aplicación de la máquina virtual, está realizada en el lenguaje de programación **C11** utilizando el ide de JetBrains CLion.

Está compilada y/o desarrollada para el sistema operativo de **Windows 11 x64**.



## Requerimientos previos

- **CMAKE** (Versión mínima 4.0)
- **C Compiler**: GCC (Versión mínima 8.1.0)
- **Sistema Operativo**: Windows 11 x64
- **C Debugger**: GDB (Versión mínima 8.1.0)
## Instalación


1. Clonar el repositorio

```bash
  git clone https://github.com/Whejseider/TPMVARQ.git
  cd TPMVARQ
```

2. Compilar
```bash
  mkdir build
  cd build
  cmake .. -G "MinGW Makefiles"
  cmake --build .
```

## Ejecución

El programa se ejecuta con 2 parámetros, de los cuáles uno es opcional **[-d]**.

1. Ejecutar la terminal (CMD o Powershell)
2. Escribir la ruta del ejecutable **TPMVARQ.exe** encontrada en la carpeta build. Dejando un espacio, la ruta del archivo **.vmx** y con otros espacio de forma opcional el parámetro sin corchetes **[-d]**

Ejemplos:
```bash
C:\Users\xxxx\TPMVARQ\cmake-build-debug> .\TPMVARQ.exe .\sample.vmx -d
```

```bash
C:\Users\xxxx\xx> "Ruta al TPMVARQ.exe" "Ruta al archivo .vmx" -d
```
## Ejemplos

El programa **sample.vmx** muestra el factorial del número 3.

```bash
3! = 3.2.1 = 6
```

1. Ejecución con DISASSEMBLER [-d]
```bash
C:\Users\xxxx\TPMVARQ\cmake-build-debug> .\TPMVARQ.exe .\sample.vmx -d
```

SALIDA

```bash
=== INFORMACIÓN DEL PROGRAMA ===
Identificador: 'VMX25'
Versión: 1
Bytes tamaño del código (hex): 00 2D
Tamaño del código: 45

=== DISASSEMBLER ===
[0000] 50 1B 0D          | MOV EDX, DS
[0003] B0 00 01 0D 00 04 | MOV [EDX+4], 1
[0009] 90 00 03 0C       | MOV ECX, 3
[000D] 73 0C 0D 00 04    | MUL [EDX+4], ECX
[0012] 91 FF FF 0C       | ADD ECX, -1
[0016] 85 00 0D          | JNZ 13
[0019] 90 00 01 0A       | MOV EAX, 1
[001D] 9E 00 04 0C       | LDH ECX, 4
[0021] 9D 00 01 0C       | LDL ECX, 1
[0025] 91 00 04 0D       | ADD EDX, 4
[0029] 80 00 02          | SYS 2
[002C] 0F                | STOP

=== EJECUCIÓN ===
Ejecutando programa...
[10004]: 6

Programa terminado.
```

Donde se puede apreciar la información de cabecera del programa en el inicio, en el medio se muestra el DISASSEMBLER, y al final el resultado de la ejecución del programa.

2. Ahora ejecutando sin el parámetro opcional **[-d]
```bash
=== INFORMACIÓN DEL PROGRAMA ===
Identificador: 'VMX25'
Versión: 1
Bytes tamaño del código (hex): 00 2D
Tamaño del código: 45

Ejecutando programa...
[10004]: 6

Programa terminado.
```

Al no incluir el parámetro, no se muestra el DISASSEMBLER