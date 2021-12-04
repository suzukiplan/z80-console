#include <stdio.h>

/**
 * @brief 共有ライブラリの初期化処理を記述（※関数実装自体を省略可能）
 * @param (ctx) 呼び出し元 Z80Console のインスタンス（Z80Console* にキャストして利用可能）
 */
//
extern "C" void start(void* ctx)
{
    puts("libplugin.so: Invoked start()");
}

/**
 * @brief 共有ライブラリの終了処理を記述（※関数実装自体を省略可能）
 * @param (ctx) 呼び出し元 Z80Console のインスタンス（Z80Console* にキャストして利用可能）
 */
extern "C" void end(void* ctx)
{
    puts("libplugin.so: Invoked end()");
}

/**
 * @brief 入力（IN）処理
 * @param (ctx) 呼び出し元 Z80Console のインスタンス（Z80Console* にキャストして利用可能）
 * @param (port) 入力ポート番号
 * @return 入力結果
 */
extern "C" unsigned char in(void* ctx, unsigned char port)
{
    printf("libplugin.so: Invoked in(%02X)\n", port);
    return 0;
}

/**
 * @brief 出力（OUT）処理
 * @param (ctx) 呼び出し元 Z80Console のインスタンス（Z80Console* にキャストして利用可能）
 * @param (port) 出力ポート番号
 * @param (value) 出力値
 */
extern "C" void out(void* ctx, unsigned char port, unsigned char value)
{
    printf("libplugin.so: Invoked out(%02X) = %02X\n", port, value);
}
