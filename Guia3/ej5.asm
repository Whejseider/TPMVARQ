inicio:    mov eax, 0b01       ; modo lectura decimal
    mov edx, DS
    ldl ecx, 1
    ldh ecx, 4
    sys 1               ; lee n en [edx] (celda 0)

    mov efx, [edx]      ; efx = contador (arranca en n)
    mov eex, 1          ; eex = resultado parcial (arranca en 1)

bucle_fact:    cmp efx, 1
    jnp fin_fact        ; si efx <= 1, ya terminé (cubre n=0 y n=1)

    mov ebx, eex        ; parámetro 1 = resultado acumulado
    mov ecx, efx        ; parámetro 2 = contador actual
    mov edx, retorno
    jmp multiplicar     ; "call"

retorno:    mov eex, eax        ; guardo el nuevo resultado
    sub efx, 1          ; decremento el contador
    jmp bucle_fact

fin_fact:    mov [4], eex        ; guardo el resultado final en la celda 4
    mov eax, 0b01       ; modo escritura decimal
    mov edx, DS
    add edx, 4
    ldl ecx, 1
    ldh ecx, 4
    sys 2
    stop

; ---------- subrutina ----------
multiplicar:    mov eax, 0
bucle_mul:    mov ac, ebx
    and ac, 1
    jz no_suma
    add eax, ecx
no_suma:    shl ecx, 1
    shr ebx, 1
    jnz bucle_mul
    jmp edx             ; "return"
