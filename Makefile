all: main md5-nopad

main: main.cpp common.hpp Makefile
	g++ -std=c++17 -O3 -o main main.cpp

md5-nopad: md5-nopad.cpp common.hpp Makefile
	g++ -std=c++17 -O3 -o md5-nopad md5-nopad.cpp
