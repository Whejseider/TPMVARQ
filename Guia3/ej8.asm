inicio:
lee:        mov eax, 1        ; modo lectura decimal
            mov edx, DS       ; siempre leo en la celda 0
            ldh ecx, 4
            ldl ecx, 1
            sys 1

            mov ebx, [edx]    ; ebx = número leído
            cmp ebx, 0
            jn fin            ; corta solo con negativo

            mov ecx, 0        ; i = bits ya probados
            mov efx, 0        ; j = bits en 1

cuenta:     cmp ecx, 32
            jz mostrar
            mov ac, ebx       ; copia para testear sin destruir ebx
            and ac, 1
            jz no_suma
            add efx, 1
no_suma:    shr ebx, 1        ; ahora sí, en los dos caminos
            add ecx, 1
            jmp cuenta

mostrar:    mov [4], efx      ; guardo el resultado
            mov eax, 1        ; modo escritura decimal
            mov edx, DS
            add edx, 4
            ldh ecx, 4
            ldl ecx, 1
            sys 2
            jmp lee           ; vuelvo a leer el siguiente

fin:        stop
