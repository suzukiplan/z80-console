all: z80con

z80con: src/z80.hpp src/z80console.hpp src/cli.cpp
	clang++ -std=c++14 -Wall -o z80con -I ./src src/cli.cpp
