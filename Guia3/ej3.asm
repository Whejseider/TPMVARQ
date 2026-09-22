inicio:     mov eax, 1
            mov edx, ds
            ldh ecx, 4
            ldl ecx, 1
            sys 1

            xor ecx, ecx ;contador i=0
            mov ebx, [edx] ;numero leido

bucle:      cmp efx, 32
            jz fin
            cmp ebx, 0
            jn es_uno
            mov [edx], 0
            jmp guardar

es_uno:     mov [edx], 1

guardar:    shl ebx, 1
            add edx, 4  ;avanzo pos edx
            add efx, 1  ;i++
            jmp bucle

fin:        mov eax, 1
            mov edx, ds
            ldh ecx, 4
            ldl ecx, 32
            sys 2
            stop


