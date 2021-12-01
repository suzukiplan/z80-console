#include "z80console.hpp"
#include <dlfcn.h>
#include <map>
#include <string>
#include <unistd.h>

static void printUsage()
{
    fprintf(stderr, "usage: z80con [-p {i|o} {00|01|02...FF} my-plugin-so:function]\n");
    fprintf(stderr, "              [-r {0|1|2...7}[:{0|1|2...7}]]\n");
    fprintf(stderr, "              [-c [clocks-per-second]]\n");
    fprintf(stderr, "              [-v [{stdout|stderr}]]\n");
    fprintf(stderr, "              my-program.bin\n");
}

#define DEFAULT_CLOCK_RATE 3579545L
static long clockRate = 0L;
static long clockRateM = 0L;
static long consumedClocks = 0L;

static void consumeClock(void* arg, int clocks)
{
    consumedClocks += clocks;
    if (clockRateM < consumedClocks) {
        long sleepMilliSec = consumedClocks / clockRateM;
        usleep(sleepMilliSec * 1000);
        consumedClocks -= sleepMilliSec * clockRateM;
    }
}

static void* searchSymbol(std::map<std::string, void*>& dlHandles, const char* lib, const char* symbol)
{
    auto itr = dlHandles.find(lib);
    if (itr == dlHandles.end()) {
        std::string path = "lib";
        path += lib;
        path += ".so";
        auto handle = dlopen(path.c_str(), RTLD_NOW);
        if (NULL == handle) return NULL;
        dlHandles[lib] = handle;
    }
    auto fp = dlsym(dlHandles[lib], symbol);
    if (NULL == fp) {
        std::string underScoreSybol = "_";
        underScoreSybol += symbol;
        fp = dlsym(dlHandles[lib], underScoreSybol.c_str());
    }
    return fp;
}

static bool loadRom(Z80Console& console, const char* fileName)
{
    FILE* fp = fopen(fileName, "rb");
    if (!fp) {
        fprintf(stderr, "error: ROM file not found (%s)\n", fileName);
        return false;
    }
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    if (0 < size) {
        fseek(fp, 0, SEEK_SET);
        void* buf = malloc(size);
        if (!buf) {
            fclose(fp);
            fprintf(stderr, "error: No memory\n");
            return false;
        }
        fread(buf, 1, size, fp);
        console.addRomData(buf, size);
        free(buf);
    }
    fclose(fp);
    return true;
}

static int hex2int(const char* str)
{
    int result = 0;
    while (*str) {
        result <<= 4;
        if (isdigit(*str)) {
            result += (*str) - '0';
        } else {
            switch (toupper(*str)) {
                case 'A': result += 10; break;
                case 'B': result += 11; break;
                case 'C': result += 12; break;
                case 'D': result += 13; break;
                case 'E': result += 14; break;
                case 'F': result += 15; break;
            }
        }
        str++;
    }
    return result;
}

static bool isdigitString(const char* str)
{
    int len = 0;
    while (str[len]) {
        if (!isdigit(str[len])) return false;
        len++;
    }
    return 0 < len;
}

static bool addPlugin(Z80Console& console, std::map<std::string, void*>& dlHandles, char* arg1, char* arg2, char* arg3)
{
    bool isInput;
    switch (arg1[0]) {
        case 'i': isInput = true; break;
        case 'o': isInput = false; break;
        default:
            fprintf(stderr, "error: Unknown plugin type (%s)\n", arg1);
            printUsage();
            return false;
    }
    unsigned char port = (unsigned char)hex2int(arg2);
    char* lib = arg3;
    char* symbol = strchr(lib, ':');
    if (!symbol) {
        fprintf(stderr, "error: Plugin symbol not specified (%s)\n", arg3);
        printUsage();
        return false;
    }
    *symbol++ = '\0';
    fprintf(stderr, "Loading lib%s.so ... ", lib);
    void* ptr = searchSymbol(dlHandles, lib, symbol);
    if (!ptr) {
        fprintf(stderr, "error while loading symbol (%s:%s)\n", lib, symbol);
        perror("Reason");
        return false;
    } else {
        fprintf(stderr, "succeed\n");
    }
    if (isInput) {
        console.addInputDevice(port, (unsigned char (*)(void*, unsigned char))ptr);
    } else {
        console.addOutputDevice(port, (void (*)(void*, unsigned char, unsigned char))ptr);
    }
    return true;
}

int main(int argc, char* argv[])
{
    Z80Console console;
    std::map<std::string, void*> dlHandles;

    for (int i = 1; i < argc; i++) {
        if ('-' == argv[i][0]) {
            switch (argv[i][1]) {
                case 'p': {
                    if (argc <= i + 3) {
                        fprintf(stderr, "error: Missing argument for -p option\n");
                        printUsage();
                        return -1;
                    }
                    if (!addPlugin(console, dlHandles, argv[i + 1], argv[i + 2], argv[i + 3])) {
                        return -1;
                    }
                    i += 3;
                    break;
                }
                case 'r': {
                    if (argc <= i + 1) {
                        fprintf(stderr, "error: Missing argument for -r option\n");
                        printUsage();
                        return -1;
                    }
                    i++;
                    int ramStart = atoi(argv[i]);
                    char* ramEndPtr = strchr(argv[i], ':');
                    if (ramEndPtr) {
                        int ramEnd = atoi(ramEndPtr + 1);
                        console.resetBanks(ramStart, ramEnd);
                    } else {
                        console.resetBanks(ramStart);
                    }
                    break;
                }
                case 'm': {
                    if (argc <= i + 1) {
                        fprintf(stderr, "error: Missing argument for -m option\n");
                        printUsage();
                        return -1;
                    }
                    i++;
                    int ramCount = atoi(argv[i]);
                    if (ramCount < 0 || 256 < ramCount) {
                        fprintf(stderr, "error: Invalid value of -m option (%d)\n", ramCount);
                        printUsage();
                        return -1;
                    }
                    console.setRamCount(ramCount);
                    break;
                }
                case 'v': {
                    bool isStdout = true;
                    if (i + 1 < argc) {
                        if (0 == strcmp(argv[i + 1], "stdout")) {
                            i++;
                            isStdout = true;
                        } else if (0 == strcmp(argv[i + 1], "stderr")) {
                            i++;
                            isStdout = false;
                        }
                    }
                    if (isStdout) {
                        console.cpu->setDebugMessage([](void* arg, const char* msg) {
                            puts(msg);
                        });
                    } else {
                        console.cpu->setDebugMessage([](void* arg, const char* msg) {
                            fprintf(stderr, "%s\n", msg);
                        });
                    }
                    break;
                }
                case 'c': {
                    if (i + 1 < argc && isdigitString(argv[i + 1])) {
                        i++;
                        clockRate = atol(argv[i]);
                    } else {
                        clockRate = DEFAULT_CLOCK_RATE;
                    }
                    clockRateM = clockRate / 1000;
                    if (0 < clockRate) {
                        if (clockRateM < 1) {
                            fprintf(stderr, "error: The clock rate must be at least 1000 Hz.\n");
                            return -1;
                        }
                        console.cpu->setConsumeClockCallback(consumeClock);
                    } else {
                        console.cpu->setConsumeClockCallback(NULL);
                    }
                    break;
                }
                default:
                    fprintf(stderr, "error: Unknown argument (%s)\n", argv[i]);
                    printUsage();
                    return -1;
            }
        } else {
            if (!loadRom(console, argv[i])) return -1;
        }
    }
    if (0 == console.getRomCount()) {
        fprintf(stderr, "error: ROM file was not specified\n");
        printUsage();
        return -1;
    }
    fprintf(stderr, "Start the ConsoleComputer\n");
    while (!console.isEnded()) console.execute(DEFAULT_CLOCK_RATE);
    int returnCode = console.getReturnCode();
    fprintf(stderr, "ConsoleComputer has been ended (code: %d)\n", returnCode);
    for (auto itr = dlHandles.begin(); dlHandles.end() != itr; itr++) dlclose(itr->second);
    return returnCode;
}
