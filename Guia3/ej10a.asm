;10. Hacer un programa que permita ingresar una lista de números que finaliza con un número
;negativo. Luego, el programa pedirá ingresar n-1 números de la misma lista y, al finalizar la
;carga, deberá mostrar el número que falta. Los números de las listas pueden ser ingresados en
;cualquier orden. Ejemplo: se ingresan 4,5,3,7,-1 luego 3,7,4 y muestra 5.
;a. Resolver el problema utilizando un vector. <<<<<<<<<<<<<<<<<<<
;b. Proponer una solución sin utilizar arreglos.

inicio:     xor efx, efx        ;i = 0
            mov edx, DS         ; edx: puntero fijo para leer (celda 0)
            mov ebx, DS
            add ebx, 4          ; ebx: puntero de escritura del array (desde offset 4)

lee:        mov eax, 1
            ldh ecx, 4
            ldl ecx, 1
            sys 1

            mov ac, [edx]
            cmp ac, 0
            jn fin_lee

            mov [ebx], ac
            add efx, 1      ;i++
            add ebx, 4      ;avanzo puntero 1 pos
            jmp lee

fin_lee:    mov eex, efx    ;j=i
            sub eex, 1      ;i--

lee_otra:   cmp eex, 0
            jmp buscar

            mov eax, 1
            ldh ecx, 4
            ldl ecx, 1
            sys 1

            mov ac, [edx]

            mov [ebx], ac
            sub eex, 1
            add ebx, 4
            jmp lee_otra
