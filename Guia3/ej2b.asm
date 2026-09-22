; . Dada una lista de ceros y unos que se ingresan por teclado, imprimir el valor decimal
; equivalente. El fin de la lista se indica con un número distinto de 0 y 1.
; La lista se ingresa del bit más significativo al menos significativ
inicio:         xor ebx, ebx    ;registro base

lee:            MOV EAX, 1
                MOV EDX, DS
                LDL ECX, 1
                LDH ECX, 4
                SYS 1

verifica:       cmp [edx], 1
                jz sigue
                cmp [edx], 0
                jnz fin

sigue:          shl ebx, 1
                add ebx, [edx]
                jmp lee

fin:            mov [edx], ebx
                mov eax, 1
                ldl ecx, 1
                ldh ecx, 4
                sys 2
                stop
