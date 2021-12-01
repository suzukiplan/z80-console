# Hello, World!

コンソールに `Hello, World!` を出力するシンプルな実装例です。

## Pre-requests

- GNU Make
- [z88dk](https://github.com/z88dk/z88dk) (z80asm command)

## How to use

```bash
make
```

## Result

```bash
% make
z80asm -b hello.asm
cd ../.. && make
clang++ -std=c++14 -Wall -o z80con -I ./src src/cli.cpp
../../z80con hello.bin
Start the ConsoleComputer
ROM: 8192 bytes
RAM: 2097152 bytes
Hello, World!
ConsoleComputer has been ended (code: 0)
```

