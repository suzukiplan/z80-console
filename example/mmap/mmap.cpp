#include <stdio.h>

/**
 * @brief 共有ライブラリの初期化処理を記述（※関数実装自体を省略可能）
 * @param (ctx) 呼び出し元 Z80Console のインスタンス（Z80Console* にキャストして利用可能）
 */
//
extern "C" void start(void* ctx)
{
    puts("libmmap.so: Invoked start()");
}

/**
 * @brief 共有ライブラリの終了処理を記述（※関数実装自体を省略可能）
 * @param (ctx) 呼び出し元 Z80Console のインスタンス（Z80Console* にキャストして利用可能）
 */
extern "C" void end(void* ctx)
{
    puts("libmmap.so: Invoked end()");
}

/**
 * @brief メモリ読み込み処理
 * @param (ctx) 呼び出し元 Z80Console のインスタンス（Z80Console* にキャストして利用可能）
 * @param (addr) 読み込みアドレス
 * @return 入力結果
 */
extern "C" unsigned char readAddr(void* ctx, unsigned short addr)
{
    printf("libmmap.so: Invoked readAddr(%04X)\n", addr);
    return 0x11;
}

/**
 * @brief メモリ書き込み処理
 * @param (ctx) 呼び出し元 Z80Console のインスタンス（Z80Console* にキャストして利用可能）
 * @param (addr) 書き込みアドレス
 * @param (value) 書き込み値
 */
extern "C" void writeAddr(void* ctx, unsigned short addr, unsigned char value)
{
    printf("libmmap.so: Invoked wrieAddr(%04X) = %02X\n", addr, value);
}
