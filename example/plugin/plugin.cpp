#include "../../src/z80console.hpp"

extern "C" void start(void* ctx)
{
    puts("libplugin.so: Invoked start()");
}

extern "C" void end(void* ctx)
{
    puts("libplugin.so: Invoked end()");
}

extern "C" unsigned char in(void* ctx, unsigned char port)
{
    printf("libplugin.so: Invoked in(%02X)\n", port);
    return 0;
}

extern "C" void out(void* ctx, unsigned char port, unsigned char value)
{
    printf("libplugin.so: Invoked out(%02X) = %02X\n", port, value);
}

 
