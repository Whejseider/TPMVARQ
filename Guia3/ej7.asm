; Dado un número natural, imprimir un 1 si es primo y un 0 si no lo es.

inicio:     mov eax, 1
            mov edx, ds
            ldh ecx, 4
            ldl ecx, 1
            sys 1

set:        mov ebx, [edx]  ;n

            cmp ebx, 2
            jn no_primo

            xor eax, 2      ;div = 2 hasta div < n

bucle:      cmp eax, ebx    ;div < ebx ?
            jnn es_primo    ; 29 < 29 ? no 0 salta

            mov ecx, ebx
            div ecx, eax
            cmp ac, 0
            jz no_primo

            add eax, 1      ;sig divisor
            jmp bucle

es_primo:   mov edx, 1
            jmp fin

no_primo:   mov edx, 0

fin:        mov [4], edx
            mov eax, 1
            mov edx, ds
            add edx, 4
            ldh ecx, 4
            ldl ecx, 1
            sys 2
            stop


