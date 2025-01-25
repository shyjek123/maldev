cmake_minimum_required(VERSION 3.29)
project(shellcode C)

set(CMAKE_C_STANDARD 11)

include_directories(.)
include_directories(example)

add_executable(shellcode
    example/injection.c
    example/injection.h
    example/main.c
    headers.h
    main.c
    procInjector.c)
