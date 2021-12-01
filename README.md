# Console Computer for Z80 - Emulator

## About

Console Computer for Z80 は、Z80 で記述されたプログラムを実行でき、標準入出力機能とプラグイン機能のみ持つシンプルで自由度の高い コンピュータシステム です。

本リポジトリは Console Computer for Z80 のエミュレータ、CLI実装、examples を提供します。

## Warp and Woof

私は自由度な Z80 プログラミング環境を求めています。

世界初（？）のホームコンピュータ標準規格の MSX あたりが良いのではないかと思って触ってみたりしたのですが、実際に触ってみると「自由度があまり高く無い」という結論に至りました。

正確には、初代 MSX はシンプルな反面性能限界が低いため自由度が低く、その点は MSX2 以降で色々と底上げされたものの MSX との互換性維持のため Memory Management Unit (スロット) 周りの仕様がシンプルではない事で自由度が低く感じられました。

そこで、MSX よりもシンプルでありながら、Z80 の限界を最大限引き出せるコンピュータとして、この Console Computer の規格を考え、その実体を Emulator として実装しました。

## System Configuration

- CPU: Z80 互換
  - Console Computer for Z80 としてのクロックレート規定はしない
  - Z80A 相当の同期（3,579,545 Hzでの動作）は可能
- MMU _(Memory Mangement Unit)_
  - 8KB 区切り (8ページ) でバンク切り替え可能な MMU を搭載
  - プログラム（ROM）サイズ: 最小 8KB 〜 最大 8KB x 256 (2MB)
  - メインメモリ（RAM）サイズ: 最小 8KB 〜 最大 8KB x 256 (2MB)
- I/O:
  - 最小限のシステム I/O を提供:
    - 0x00 ~ 0x07: バンク切り替え
    - 0x0F: 標準入出力
  - 外部入出力を User Port Plugin による外部入出力機構
- No BIOS _(I DON'T LIKE IT BECAUSE UNFREE)_

## Components

- [z80.hpp](src/z80.hpp) : CPU (Emulator)
- [z80con.hpp](src/z80con.hpp) : Console Computer for Z80
- [cli.cpp](src/cli.cpp) : Command Line Interface

## Command Line Interface

### z80con

```bash
z80con [-p {i|o} {00|01|02...FF} my-plugin-so:function]
       [-r {0|1|2...7}[:{0|1|2...7}]]
       [-m {1|2|3...256}]
       [-c [clocks-per-second]]
       [-v [{stdout|stderr}]]
       my-program.bin
```

- `[-p I/O ポート番号 共有ライブラリ:関数名]` _optional_
  - User Plugin Port の割り当て
  - 共有ライブラリは プリフィクス lib と 拡張子 .so を省略して指定
    - 例: `libhoge.so` なら `hoge` と指定する
  - User Plugin Port は 0 個以上の複数を割り当て可能
  - 同一ポートの User Plugin Port を複数指定した場合、右側に指定したものが有効
- `[-r {0|1|2...7}[:{0|1|2...7}]]` _optional_
  - RAM の割り当て範囲（バンク番号）
  - デフォルト（省略時）は `-r 4:7` (`-r 4` or `-r 7:4` と等価) を仮定
- `[-m {1|2|3...256}]` _optional_
  - RAM の搭載ユニット数 (1unit = 8KB)
  - 省略時は 256 (2MB) を仮定
- `[-c [clocks-per-second]]` _optional_
  - CPU クロック周波数を指定して同期
  - 指定省略時は実行端末のベストエフォート (= 同期無し) で動作
  - `clocks-per-second` を省略した場合 `3579545` (Z80A 相当) を仮定
  - 実行端末の処理性能を超える数値は指定不可（※エラーにはならない）
- `[-v [{stdout|stderr}]]` _optional_
  - 動的ディスアセンブルを表示
  - `stdout` 標準出力（省略時のデフォルト）
  - `stderr` 標準エラー出力
- `my-program.bin` _required_
  - 実行するプログラム
  - 複数個指定できる
  - 1ファイル = 8KB パディング（8KB 未満の場合、末尾が 0x00 で埋められる）

## Programming Manual (Z80)

Console Computer for Z80 で動作する Z80 のプログラム

## Default Memory Map

- Bank 0~3: Program Banks (32KB)
  - 0x0000 ~ 0x1FFF : Program Bank 0
  - 0x2000 ~ 0x3FFF : Program Bank 1
  - 0x4000 ~ 0x5FFF : Program Bank 2
  - 0x6000 ~ 0x7FFF : Program Bank 3
- Bank 4~7: RAM Banks (32KB)
  - 0x8000 ~ 0x9FFF : RAM Bank 0
  - 0xA000 ~ 0xBFFF : RAM Bank 1
  - 0xC000 ~ 0xDFFF : RAM Bank 2
  - 0xE000 ~ 0xFFFF : RAM Bank 3

## I/O Map

- System Ports: 0x00 ~ 0x0F
  - Console Computer for Z80 のシステムポート
  - User Plugin Ports を優先させることも可能
- User Plugin Ports: 0x10 ~ 0xFF
  - 外部プラグイン用ポート

| Port | I | O | Description |
|:-:|:-:|:-:|:-|
| 0x00 | o | o | Bank 0 Switch |
| 0x01 | o | o | Bank 1 Switch |
| 0x02 | o | o | Bank 2 Switch |
| 0x03 | o | o | Bank 3 Switch |
| 0x04 | o | o | Bank 4 Switch |
| 0x05 | o | o | Bank 5 Switch |
| 0x06 | o | o | Bank 6 Switch |
| 0x07 | o | o | Bank 7 Switch |
| 0x0F | o | o | Console Read, Console Write |

### 0x00 ~ 0x07 [I/O] Bank Switch

- 解説
  - Bank 0 ~ 7 のインデックス読み取り (I) と 切り替え (O) を行う
- 入力レジスタ
  - n/a
- 出力レジスタ
  - n/a

### 0x0F [I] Console Read

- 解説
  - コンソールからの入力文字列を読み込む
  - 入力完了（改行入力）まで、プログラムの処理は中断される
  - 復帰・改行コードは入力文字列に含まれない
- 入力レジスタ
  - HL : 入力文字列の格納アドレス
  - BC : 入力文字列の最大バイト数
- 出力レジスタ
  - F/carry : 入力文字列数が BC を超える場合セット、BC 以下の場合リセット

(Example)

```z80
    ld hl, $C000
    ld bc, 12
    in a, ($0F)
```

`Hello, World!` を 入力すると、次のように動作する

- 0xC000 ~ 0xC00B に `Hello, World` が格納される
- F/carry が セットされる
- レジスタ A に 12 が設定される

### 0x0F [O] Console Write

- 解説
  - コンソールへ文字列を書き込む
  - 出力完了まで、プログラムの処理は中断される
- 入力レジスタ
  - HL : 出力文字列の格納アドレス
- 入力値
  - 出力文字列バイト数 (0 ~ 255)
- 出力レジスタ
  - n/a

(Example: [example/hello/hello.asm]([example/hello/hello.asm))

```z80
org $0000

.Start
   ld hl, SAMPLE_TEXT
   ld a, 14
   out ($0F), a
   ld a, 0
   ret

SAMPLE_TEXT:
   db "Hello, World!", $0A
```

## User Plugin Port

Console Computer for Z80 は、**プログラム実行** と **コマンドライン入出力** のみ実行できる最小限の I/O のみ提供していますが、このままではせいぜいハノイの塔ぐらいしか作れません。

例えば、FM音源 や PSG音源を搭載して音楽を再生、計測器との入出力、インターネット経由でサーバと通信などは、User Plugin Port を定義することで実現できます。

User Plugin Port は、コマンドラインで指定した共有ライブラリ（将来的に Windows に対応時は DLL）に定義されている指定関数がコールバックされます。

### Minimum Example

例えば、

```bash
z80con -p i F0 hoge:in -p o F1 hoge:out program.bin
```

というコマンドライン指定で z80con を実行した場合、共有ライブラリ `libhoge.so` の最小実装は次のようになります。

```cpp
// hoge.cpp (libhoge.soの最小実装)

extern "C" unsigned char in(void* z80console, unsigned char port) {
    return 0xFF;
}

extern "C" void out(void* z80console, unsigned char port, unsigned char value) {
}
```

そして、 `program.bin` で `IN A, ($F0)` が実行されると上記の `in` 関数がコールバックされ、`OUT ($F1), A` が実行されると上記の `out` 関数がコールバックされます。

### Input Function Prototype

```c++
extern "C" unsigned char functionName(void* z80console, unsigned char port);
```

- 引数:
  - `z80console`:
    - 呼び出し元 Console Computer for Z80 のインスタンス
    - [z80console.hpp](src/z80console.hpp) を `include` して　`Z80Console*` へキャスト可能
  - `port`: 入力ポート番号
- 戻り値: ポート入力の結果を `0` ~ `255` の範囲で返す

### Output Function Prototype

```c++
extern "C" void functionName(void* z80console, unsigned char port, unsigned char value);
```

- 引数:
  - `z80console`:
    - 呼び出し元 Console Computer for Z80 のインスタンス
    - [z80console.hpp](src/z80console.hpp) を `include` して　`Z80Console*` へキャスト可能
  - `port`: 出力ポート番号
  - `value`: 出力値
- 戻り値: n/a

## Licenses

### Console Computer for Z80 - Emulator (MIT)

```text
Copyright (c) 2021 Yoji Suzuki.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
```

### SUZUKI PLAN - Z80 Emulator (MIT)

[https://github.com/suzukiplan/z80](https://github.com/suzukiplan/z80)

```text
Copyright (c) 2019 Yoji Suzuki.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
```