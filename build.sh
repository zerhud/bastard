#!/usr/bin/env bash


g++ -std=c++23 \
	-fdiagnostics-color=always \
	-fwhole-program -march=native test.cpp \
	-o test 2>&1 | less

./test 2>&1 |less

