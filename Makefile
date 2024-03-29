all: z80con

clean:
	rm -f z80con

hello:
	cd example/hello && make

z80con: src/z80.hpp src/z80console.hpp src/cli_unix.cpp
	clang++ -std=c++14 -Wall -Werror -fPIC -o z80con -I ./src src/cli_unix.cpp -ldl
