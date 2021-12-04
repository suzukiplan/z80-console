# Simple Memory Mapped I/O Example

Memory Mapped I/O の簡単な実行例です。

## Pre-requests

- GNU Make
- Clang C++
- [z88dk](https://github.com/z88dk/z88dk) (z80asm command)

## How to build and execute

```bash
make
```

## Result

```bash
% make
../../z80con -v mmap.bin -m r C1 mmap:readAddr -m w C2 mmap:writeAddr 
Loading readAddr from libmmap.so ... succeed
Loading writeAddr from libmmap.so ... succeed
Start the ConsoleComputer
libmmap.so: Invoked start()
[0000] LD A, ($C0FF) = $00
libmmap.so: Invoked readAddr(C100)
[0003] LD A, ($C100) = $11
libmmap.so: Invoked readAddr(C180)
[0006] LD A, ($C180) = $11
libmmap.so: Invoked readAddr(C1FF)
[0009] LD A, ($C1FF) = $11
[000C] LD A, ($C200) = $00
[000F] LD ($C1FF), A<$00>
[0012] LD ($C200), A<$00>
libmmap.so: Invoked wrieAddr(C200) = 00
[0015] LD ($C280), A<$00>
libmmap.so: Invoked wrieAddr(C280) = 00
[0018] LD ($C2FF), A<$00>
libmmap.so: Invoked wrieAddr(C2FF) = 00
[001B] LD ($C300), A<$00>
libmmap.so: Invoked end()
[001E] RET to $FFFF (SP<$0000>)
ConsoleComputer has been ended (code: 0)
```
