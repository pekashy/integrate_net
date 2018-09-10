cmake_minimum_required(VERSION 3.9)
project(integrate_net C)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES
        client.c
        server.c
        headers.h
        )

add_executable(integrate_net client.c server.c headers.h)