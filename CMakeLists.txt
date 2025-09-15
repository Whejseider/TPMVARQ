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
        dissasembler/dissasembler.c
        dissasembler/dissasembler.h)
