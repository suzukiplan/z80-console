# Console Computer for Z80 - Emulator

Console Computer は、標準では標準入出力機能だけもつ極めてシンプルなコンピュータでありながら、プラグインと Memory Mapped I/O により柔軟な拡張性を併せ持つ **シンプル＆自由な汎用コンピュータ** です。

Console Computer for Z80 は、Console Computer のプログラムを Z80 で記述できます。Z80 ベースのハードウェア（エミュレータを含む）を新規に設計したり、既存の Z80 ベースのハードウェア（エミュレータを含む）上で動作するプログラムを記述するのに適したベースシステムとなることを目指しています。

本リポジトリは、Console Computer for Z80 の [エミュレータ](src/z80console.hpp)、[UNIX+互換機用CLI実装](src/cli_unix.cpp)、[Example](example) を提供します。

## System Configuration

- CPU: Z80 互換
  - Console Computer としてのクロックレート規定はなく、通常はベストエフォートでの動作を想定
  - Z80A 相当での同期（3,579,545 Hzでの動作）をエミュレート可能
  - 実際に存在しない速度（例: 1,000 Hz）などの同期もエミュレート可能
- MMU _(Memory Mangement Unit)_
  - 8KB 区切り (8ページ) で最大 256 バンクに切り替え可能な MMU を搭載
    - プログラム（ROM）サイズ: 最小 8KB 〜 最大 8KB x 256 (2MB)
    - メインメモリ（RAM）サイズ: 8KB x 256 (2MB)
- I/O:
  - 標準では最小限のシステム I/O のみ提供:
    - 0x00 ~ 0x07: バンク切り替え
    - 0x0F: 標準入出力
  - IN/OUT 命令（入出力）による拡張入出力機構: **Plugin**
  - LD 命令（メモリアクセス）による拡張入出力機構: **Memory Mapped I/O**
- No BIOS _(I DON'T LIKE IT BECAUSE UNFREE)_

## Components

- [z80.hpp](src/z80.hpp) : Central Processing Unit (Emulator)
- [z80console.hpp](src/z80console.hpp) : Console Computer (Emulator)
- [cli_unix.cpp](src/cli_unix.cpp) : Command Line Interface for UNIX

C++11 以降の Clang C++ でコンパイルできます。

## Command Line Interface

現時点では、BSD 系 UNIX 互換システム（Linux と MacOS を含む）向けの CLI のみ提供しています。

### z80con

```bash
z80con [-p {i|o} {00|01|02...FF} my-plugin-so:function]
       [-m {r|w} {00|01|02...FF} my-mmap-so:function]
       [-r {0|1|2...7}[:{0|1|2...7}]]
       [-c [clocks-per-second]]
       [-v [{stdout|stderr}]]
       my-program.bin
```

- `[-p {i|o} ポート番号 共有ライブラリ:関数名]` _optional_
  - Plugin の割り当て
  - 共有ライブラリは プリフィクス lib と 拡張子 .so を省略して指定
    - 例: `libhoge.so` なら `hoge` と指定する
  - Plugin は 0 個以上の複数を割り当て可能
  - 同一ポートの Plugin を複数指定した場合、右側に指定したものが有効
- `[-m {r|w} {00|01|02...FF} my-mmap-so:function]` _optional_
  - Memory Mapped I/O の割り当て
  - アドレスページ番号は、メモリマップ対象とするアドレスの上位 8bit を指定する  
  - Memory Mapped I/O は 0 個以上の複数を割り当て可能
  - 同一アドレスページ番号の Memory Mapped I/O を複数指定した場合、右側に指定したものが有効
- `[-r {0|1|2...7}[:{0|1|2...7}]]` _optional_
  - RAM の割り当て範囲（バンク番号）
  - デフォルト（省略時）は `-r 4:7` (`-r 4` or `-r 7:4` と等価) を仮定
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

## Examples

| Path | Description |
|:-:|:-|
| [example/hello](example/hello) | `Hello, World!` と 改行 を 標準出力 |
| [example/plugin](example/plugin) | Plugin の簡単な実行例 |
| [example/mmap](example/mmap) | Memory Mapped I/O の簡単な実行例 |

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
  - Console Computer のシステムポート
  - Plugins を優先させることも可能
- Plugins: 0x00 ~ 0xFF
  - Plugin を割り当てできるポート
  - Plugin の方が System Ports よりも優先が高い
    - System Ports に対して Plugin 上書き割り当てすると、対応するシステム I/O が使えなくなる

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

## Plugin

Console Computer は、**プログラム実行** と **標準入出力** のみ実現できる最小限の I/O のみ提供していますが、このままではせいぜいハノイの塔ぐらいしか作れません。

例えば、FM音源 や PSG音源を搭載して音楽を再生、計測器との入出力、インターネット経由でサーバと通信などは、Plugin を定義することで実現できます。

Plugin の実体は、対応するポート番号への IN/OUT 命令が実行された時にコールバックされる関数です。

詳しくは以下の Example を参照してください。

[example/plugin](example/plugin)

## Memory Mapped I/O

Memory Mapped I/O とは、アドレスを 256 バイト区切りの 256 ページとして、各アドレスページへのアクセスをトラップして外部入出力を行うことができます。

例えば、16KB の VRAM（ビデオメモリ）を持つデバイス（TMS9918Aなど）にアクセスする際、CPU アドレスの 0x8000 ~ 0xBFFF の範囲（ページ 0x80 ~ 0xBF）の範囲を Memory Mapped I/O とすることで、Plugin よりもシンプルなプログラムで VRAM アクセスが実現できます。

詳しくは以下の Example を参照してください。

[example/mmap](example/mmap)

## Licenses

### Console Computer - Emulator (MIT)

See the [LICENSE.txt](LICENSE.txt)

### SUZUKI PLAN - Z80 Emulator (MIT)

See the [https://github.com/suzukiplan/z80/blob/master/LICENSE.txt](https://github.com/suzukiplan/z80/blob/master/LICENSE.txt)
