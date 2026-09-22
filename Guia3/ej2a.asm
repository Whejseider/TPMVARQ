; . Dada una lista de ceros y unos que se ingresan por teclado, imprimir el valor decimal
; equivalente. El fin de la lista se indica con un número distinto de 0 y 1.
; a. La lista se ingresa del bit menos significativo al más significativo.

inicio:         xor ebx, ebx    ;registro base
                xor eex, eex    ;contador i = 0
set_mascara:    xor efx, efx    ;registro mascara = 0

lee:            MOV EAX, 1
                MOV EDX, DS
                LDL ECX, 1
                LDH ECX, 4
                SYS 1

verifica:       cmp [edx], 1
                jz sigue
                cmp [edx], 0
                jnz fin

sigue:          mov efx, [edx]
                shl efx, eex    ;efx << eex
                add ebx, efx
                add eex, 1      ;i++
                jmp set_mascara

fin:            mov [edx], ebx
                mov eax, 1
                ldl ecx, 1
                ldh ecx, 4
                sys 2
                stop
