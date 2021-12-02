all: z80con

clean:
	rm -f z80con

z80con: src/z80.hpp src/z80console.hpp src/cli_unix.cpp
	clang++ -std=c++14 -Wall -Werror -o z80con -I ./src src/cli_unix.cpp
