; 1. Calcular e imprimir el promedio de una lista de números naturales
; que se ingresan por teclado.
; El fin de la lista se indica con un número negativo.

INICIO: XOR EFX, EFX    ;SETEAR EN 0 EL CONTADOR i=0
        XOR AC, AC    ;SETEAR EN 0 LA SUMA sum=0

LEE:    MOV EAX, 1
        MOV EDX, DS
        LDL ECX, 1
        LDH ECX, 4
        SYS 1

CMP:    MOV EAX, [EDX]  ;COPIO EL CONTENIDO DE EDX, QUE SERA EL NUMERO LEIDO EN EAX
        CMP EAX, -1
        JZ FIN  ;SI ES -1 TERMINO
        JN LEE  ;SI ES NEGATIVO LEO OTRA VEZ Y NO HAGO NADA CON LOS OTROS
        CMP EAX, 0
        JZ LEE  ;SI ES CERO, LEO

;SI LLEGO ESTA ACA ESTA TODO OK
ALGO:   ADD [EFX], 1    ;i++
        ADD AC, EAX    ;sum+=num
        JMP LEE

FIN:    ADD EDX, 4
        CMP EFX, 0
        JZ ERROR
        DIV AC, EFX ;sum/i
        MOV [EDX], AC
MUESTRA: MOV EAX, 1
        MOV EDX, DS
        LDH ECX, 4
        LDL ECX, 1
        SYS 2

ERROR:  STOP

