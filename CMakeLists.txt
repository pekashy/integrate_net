cmake_minimum_required(VERSION 3.9)
project(integrate_net C)

set(CMAKE_C_STANDARD 99)

add_executable(integrate_net main.c client.c server.c)