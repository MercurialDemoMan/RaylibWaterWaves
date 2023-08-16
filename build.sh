#!/bin/bash

set -xe

CFLAGS="-O3 -Wall -Wextra -ggdb `pkg-config --cflags raylib`"
LIBS="-lm `pkg-config --libs raylib` -lglfw -ldl -lpthread"

clang main.c -O0 $CFLAGS $LIBS -o waves