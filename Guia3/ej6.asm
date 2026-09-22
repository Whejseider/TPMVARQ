; Suponiendo que no existe la instrucción DIV, crear un algoritmo que reciba en EBX y ECX los
; dos valores y retorne en EAX el resultado de la división, dejando en AC el resto.

inicio:     xor eax, eax
            xor ac, ac
            cmp ecx, 0
            jz error

bucle:      cmp ebx, ecx    ;ebx < ecx ?
            jn fin
            sub ebx, ecx    ;ebx - ecx ej 5 - 2 = 3
            add eax, 1      ;i++
            jmp bucle

error:      mov ac, 0
            stop

fin:        mov ac, ebx
            stop
