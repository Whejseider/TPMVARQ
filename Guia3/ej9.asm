;9. Modificar el ejercicio anterior de modo que antes de ingresar la lista se lea un valor que
;represente una máscara con la cual se debe realizar un AND a cada número de la lista antes de
;calcular la cantidad de bits en 1.

inicio:     mov eax, 1        ; modo lectura decimal
            mov edx, DS
            ldh ecx, 4
            ldl ecx, 1
            sys 1             ; lee la máscara en [edx] (celda 0)

            mov eex, [edx]    ; eex = máscara (se mantiene durante todo el programa)

lee:        mov eax, 1        ; modo lectura decimal
            mov edx, DS       ; siempre leo en la celda 0
            ldh ecx, 4
            ldl ecx, 1
            sys 1

            mov ebx, [edx]    ; ebx = número leído
            cmp ebx, 0
            jn fin            ; corta solo con negativo

            and ebx, eex

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
