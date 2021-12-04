# [WIP] Simple Plugin Example

Plugin の簡単な実行例です。

## OS support status

- [x] macOS
- [ ] Linux

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
../../z80con -v plugin.bin -p i C0 plugin:in -p o C1 plugin:out 
Loading in from libplugin.so ... succeed
Loading out from libplugin.so ... succeed
Start the ConsoleComputer
libplugin.so: Invoked start()
[0000] NOP
libplugin.so: Invoked in(C0)
[0001] IN A<$00>, ($C0) = $00
[0003] IN A<$00>, ($C1) = $FF
[0005] OUT ($C0), A<$FF>
[0007] OUT ($C1), A<$FF>
libplugin.so: Invoked out(C1) = FF
[0009] LD A<$FF>, $00
[000B] NOP
libplugin.so: Invoked end()
[000C] RET to $FFFF (SP<$0000>)
ConsoleComputer has been ended (code: 0)
```
