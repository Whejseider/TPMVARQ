; 1. Calcular e imprimir el promedio de una lista de números naturales
; que se ingresan por teclado.
; El fin de la lista se indica con un número negativo.

IINICIO: XOR EFX, EFX    ; contador i = 0
        XOR EBX, EBX    ; suma = 0

LEE:    MOV EAX, 1
        MOV EDX, DS
        LDL ECX, 1
        LDH ECX, 4
        SYS 1

VERIFICA:
        MOV EAX, [EDX]
        CMP EAX, 0
        JN FIN          ; cualquier negativo corta la lista
        JZ LEE          ; el 0 no cuenta, sigo leyendo

ALGO:   ADD EFX, 1      ; i++
        ADD EBX, EAX    ; suma += num
        JMP LEE

FIN:    CMP EFX, 0
        JZ ERROR
        DIV EBX, EFX    ; ebx = suma / i (el resto va a AC, no me importa)
        ADD EDX, 4
        MOV [EDX], EBX  ; guardo el promedio en la celda 4

MUESTRA: MOV EAX, 1
        LDH ECX, 4
        LDL ECX, 1
        SYS 2           ; edx ya apunta a donde está el promedio

ERROR:  STOP
