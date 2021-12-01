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
../../z80con -v hello.bin
Start the ConsoleComputer
[0000] LD HL<$0000>, $000A
[0003] LD A<$00>, $0E
[0005] OUT ($0F), A<$0E>
Hello, World!
[0007] LD A<$0E>, $00
[0009] RET to $FFFF (SP<$0000>)
ConsoleComputer has been ended (code: 0)
```

