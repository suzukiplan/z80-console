all: hello.bin ../../z80con
	../../z80con hello.bin

../../z80con:
	cd ../.. && make

hello.bin: hello.asm
	z80asm -b hello.asm

