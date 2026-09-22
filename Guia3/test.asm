inicio:    mov ebx, 0          ; ebx = posición del bit actual
    mov efx, 0          ; efx = resultado acumulado
    mov edx, DS         ; edx apunta a la celda 0 del data segment
                        ; (siempre se lee ahí, celda [0])

leer:    mov eax, 0b01       ; modo lectura: decimal
    ldl ecx, 0x01       ; leer 1 solo valor
    ldh ecx, 0x04       ; de 4 bytes
    sys 0x1             ; guarda el valor leído en [edx] = [0]

    mov eex, [edx]      ; copio el valor leído a un registro

    cmp eex, 0          ; ¿vino un 0?
    jz es_bit
    cmp eex, 1          ; ¿vino un 1?
    jz es_bit
    jmp fin             ; ni 0 ni 1 -> fin de la lista

es_bit:    shl eex, ebx        ; corro el bit a su posición correspondiente
    or efx, eex         ; lo acumulo en el resultado
    add ebx, 1          ; siguiente posición
    jmp leer            ; leo el próximo bit

fin:    mov [4], efx        ; guardo el resultado en la celda 4
    mov eax, 0b01       ; modo escritura: decimal
    mov edx, DS
    add edx, 4          ; apunto a donde guardé el resultado
    ldl ecx, 0x01       ; escribir 1 valor
    ldh ecx, 0x04       ; de 4 bytes
    sys 0x2             ; lo imprime
    stop
