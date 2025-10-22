inicio:    mov eax, 0       ; f0 = 0  
    mov ebx, 1       ; f1 = 1  
    mov ecx, 100     ; límite = 100  
    mov edx, DS      ; base en memoria (segmento de datos)

guardar0:    mov [edx], eax   ; guardo f0 en memoria  
    add edx, 4       ; avanzo 4 bytes  

guardar1:    mov [edx], ebx   ; guardo f1 en memoria  
    add edx, 4       ; avanzo 4 bytes  

loop:      add eax, ebx     ; eax = f0 + f1 (nuevo número)  
    mov esi, eax     ; guardo en esi el valor nuevo  
    cmp esi, ecx     ; comparo con 100  
    jge fin          ; si es >=100, termino  

    mov [edx], esi   ; guardo en memoria el número  
    add edx, 4       ; avanzo memoria  

    mov eax, ebx     ; f0 = f1  
    mov ebx, esi     ; f1 = nuevo  
    jmp loop         ; repetir  

fin:      mov eax, 0b01    ; salida decimal  
    ldh ecx, 0x04    ; 4 bytes cada número  
    ldl ecx, 0x09    ; cantidad (0,1,1,2,3,5,8,13,21,34,55,89 → 11 números)  
    sys 0x2          ; imprimir todo  
    stop             ; detener
