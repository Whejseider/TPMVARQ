# Trabajo Práctico - Máquina Virtual (MV1 & MV2) 💻

Emulador de máquina virtual desarrollado en **C11** para la materia Arquitectura de Computadoras de la Facultad de Ingeniería, UNMDP. Esta versión implementa las especificaciones de la Máquina Virtual Parte I (MV1) y Parte II (MV2).

---

## ✨ Características Principales

* **Emulación Completa:** Ejecuta programas compilados en formato `.vmx` (versiones 1 y 2).
* **Gestión de Memoria:**
    * Memoria principal de tamaño variable (configurable al ejecutar).
    * Soporte para hasta 6 segmentos: **Param, Const, Code, Data, Extra, Stack**.
* **Arquitectura de Registros:** Emula 32 registros de 32 bits, incluyendo registros de propósito general, punteros de segmento (CS, DS, ES, SS, KS, PS), pila (SP, BP) y control.
* **Set de Instrucciones Ampliado (MV2):**
    * Instrucciones de manejo de pila: `PUSH`, `POP`, `CALL`, `RET`.
    * Modificadores de tamaño para operandos de memoria (`b`, `w`, `l`).
    * Acceso a sectores de registros de propósito general (ej. `AX`, `AL`, `AH`).
* **Llamadas al Sistema (Syscalls):** Incluye operaciones de I/O (`READ`, `WRITE`), manejo de strings (`STRING_READ`, `STRING_WRITE`), limpieza de pantalla (`CLEAR_SCREEN`) y debugging (`BREAKPOINT`).
* **Persistencia y Debugging:**
    * Capacidad de guardar y cargar el estado completo de la VM en archivos de imagen (`.vmi`).
    * Funcionalidad de `BREAKPOINT` para pausar, guardar estado y continuar (`go`, `quit`, `step-by-step`).
* **Manejo de Parámetros:** Soporte para pasar parámetros a los programas ejecutados a través del segmento `Param` (estilo `argc`/`argv`).
* **Disassembler:** Opción para visualizar el código Assembler correspondiente al `.vmx` cargado, incluyendo constantes string y punto de entrada.

---

## 🔧 Requerimientos Previos

* **Sistema Operativo:** Windows 11 x64
* **Compilador C:** GCC (versión mínima 8.1.0)
* **Build System:** CMAKE (versión mínima 4.0)
* **(Opcional) Debugger C:** GDB (versión mínima 8.1.0)

---

## 🛠️ Compilación

1.  Clonar el repositorio (si aplica).
2.  Crear y navegar a un directorio `build`:
    ```bash
    mkdir build
    cd build
    ```
3.  Configurar el proyecto con CMake usando MinGW Makefiles:
    ```bash
    cmake .. -G "MinGW Makefiles"
    ```
4.  Compilar el proyecto:
    ```bash
    cmake --build .
    ```
    O directamente con `mingw32-make`.

---

## ▶️ Ejecución

La máquina virtual se ejecuta desde la consola (`CMD` o `Powershell`) con la siguiente sintaxis general:

```bash
.\TPMVARQ.exe [archivo.vmx] [archivo.vmi] [m=M] [-d] [-p param1 param2 ... paramN]
```

**Parámetros:**

* `archivo.vmx` (Opcional si se usa `.vmi`): Ruta al programa compilado a ejecutar.
* `archivo.vmi` (Opcional): Ruta al archivo de imagen para guardar estado (con `BREAKPOINT`) o para cargar un estado previo y continuar la ejecución.
* `m=M` (Opcional): Especifica el tamaño de la memoria principal en KiB. (Ej: `m=32` para 32 KiB). Si se omite, el valor por defecto es 16 KiB.
* `-d` (Opcional): Muestra el desensamblador del código cargado antes de la ejecución.
* `-p param1 param2 ...` (Opcional): Indica que los siguientes argumentos son parámetros para el programa. Deben ir al final del comando. Si no se especifica un `.vmx`, estos parámetros se ignoran.

**Modos de Ejecución:**

1.  **Ejecutar un programa nuevo:**
    ```bash
    .\TPMVARQ.exe programa.vmx
    ```
2.  **Ejecutar mostrando disassembler:**
    ```bash
    .\TPMVARQ.exe programa.vmx -d
    ```
3.  **Ejecutar con memoria específica y parámetros:**
    ```bash
    .\TPMVARQ.exe programa.vmx m=64 -p arg1 "argumento 2" 123
    ```
4.  **Ejecutar y habilitar breakpoints (guardando en `estado.vmi`):**
    ```bash
    .\TPMVARQ.exe programa.vmx estado.vmi
    ```
5.  **Continuar ejecución desde una imagen guardada:**
    ```bash
    .\TPMVARQ.exe estado.vmi
    ```

---

## 📄 Información para la Entrega

* **Sistema Operativo:** Desarrollado y compilado para **Windows 11 x64**.
* **Enlace al Commit:** [Click aquí](https://github.com/Whejseider/TPMVARQ/commit/d35976a49144ec10c7c440ad0ce532867627a9be)