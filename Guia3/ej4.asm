; Suponiendo que no existe la instrucción MUL, crear un algoritmo que reciba en EBX y ECX los
; dos valores y retorne en EAX su producto.

inicio:     xor eax, eax

bucle:      mov edx, ebx
            and edx, 1
            jz no_suma
            add eax, ECX

no_suma:    shl ecx, 1
            shr ebx, 1
            jnz bucle

fin:        stop
