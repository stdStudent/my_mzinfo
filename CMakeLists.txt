cmake_minimum_required(VERSION 3.22)
project(mzinfo C)

set(CMAKE_C_STANDARD 11)

add_executable(mzinfo main.c mz.c mz.h system.c system.h disas.c disas.h)
