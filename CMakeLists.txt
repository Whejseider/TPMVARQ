cmake_minimum_required(VERSION 4.0)
project(TPMVARQ C)

set(CMAKE_C_STANDARD 11)

add_executable(TPMVARQ
        main.c
        vmx/vmx.c
        vmx/vmx.h
        memory/memory.c
        memory/memory.h
        instructions/instructions.c
        instructions/instructions.h
        loader/loader.c
        loader/loader.h
        disassembler/disassembler.c
        disassembler/disassembler.h
        syscalls/syscalls.c
        syscalls/syscalls.h
        utils/utils.c
        utils/utils.h
        image/image.c
        image/image.h
)
