/**
 * Cosnole Computer for Z80 - Emulator
 * -----------------------------------------------------------------------------
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Yoji Suzuki.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * -----------------------------------------------------------------------------
 */
#include "z80.hpp"
#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

class Z80Console
{
  private:
    inline bool isRamIndex(int n) { return ctx.ramBankIndexStart <= n && n <= ctx.ramBankIndexEnd; }

    class Handler
    {
      public:
        void (*callback)(void*);
        Handler(void (*callback)(void*)) { this->callback = callback; }
    };

    struct ExternalDevices {
        void (*out[256])(void*, unsigned char, unsigned char);
        unsigned char (*in[256])(void*, unsigned char);
        void (*write[256])(void*, unsigned short, unsigned char);
        unsigned char (*read[256])(void*, unsigned short);
        std::vector<Handler*> startHandlers;
        std::vector<Handler*> endHandlers;
    } devices;

    struct Context {
        unsigned char banks[8];
        unsigned char ramBankIndexStart;
        unsigned char ramBankIndexEnd;
        unsigned char startFlag;
        unsigned char endFlag;
        unsigned char reserved[4];
    } ctx;

  public:
    struct Memory {
        int count;
        unsigned char data[256][8192];
    };
    struct Memory rom;
    struct Memory ram;
    Z80* cpu;

    Z80Console()
    {
        cpu = new Z80(readMemory, writeMemory, inPort, outPort, this);
        cpu->addReturnHandler([](void* arg) {
            auto _this = (Z80Console*)arg;
            // Shutdown the ConsoleComputer when call the RET instruction when SP equals 0
            if (0 == _this->cpu->reg.SP) {
                for (auto handler : _this->devices.endHandlers) handler->callback(arg);
                _this->ctx.endFlag = true;
                _this->cpu->requestBreak();
            }
        });
        rom.count = 0;
        memset(rom.data, 0, sizeof(rom.data));
        ram.count = 256;
        ctx.ramBankIndexStart = 4;
        ctx.ramBankIndexEnd = 7;
        ctx.endFlag = false;
        reset();
    }

    ~Z80Console()
    {
        for (auto handler : devices.startHandlers) delete handler;
        devices.startHandlers.clear();
        for (auto handler : devices.endHandlers) delete handler;
        devices.endHandlers.clear();
        if (cpu) delete cpu;
    }

    void reset()
    {
        memset(&devices, 0, sizeof(devices));
        memset(ram.data, 0, sizeof(ram.data));
        memset(&cpu->reg, 0, sizeof(cpu->reg));
        resetBanks(ctx.ramBankIndexStart, ctx.ramBankIndexEnd);
        ctx.startFlag = false;
        if (ctx.endFlag) {
            for (auto handler : devices.endHandlers) handler->callback(this);
            ctx.endFlag = false;
        }
    }

    bool isEnded() { return this->ctx.endFlag; }
    int getRomCount() { return this->rom.count; }
    int getRamCount() { return this->ram.count; }
    int getReturnCode() { return this->cpu->reg.pair.A; }

    bool addOutputDevice(unsigned char portNumber, void (*out)(void*, unsigned char, unsigned char))
    {
        if (ctx.startFlag) return false;
        devices.out[portNumber] = out;
        return true;
    }

    bool addInputDevice(unsigned char portNumber, unsigned char (*in)(void*, unsigned char))
    {
        if (ctx.startFlag) return false;
        devices.in[portNumber] = in;
        return true;
    }

    bool addWriteMemoryMap(unsigned short address, void (*write)(void*, unsigned short, unsigned char))
    {
        if (ctx.startFlag) return false;
        devices.write[(address & 0xFF00) >> 8] = write;
        return true;
    }

    bool addReadMemoryMap(unsigned short address, unsigned char (*read)(void*, unsigned short))
    {
        if (ctx.startFlag) return false;
        devices.read[(address & 0xFF00) >> 8] = read;
        return true;
    }

    bool addStartHandler(void (*handler)(void*))
    {
        if (ctx.startFlag) return false;
        devices.startHandlers.push_back(new Handler(handler));
        return true;
    }

    bool addEndHandler(void (*handler)(void*))
    {
        if (ctx.startFlag) return false;
        devices.endHandlers.push_back(new Handler(handler));
        return true;
    }

    bool setRamCount(int ramCount)
    {
        if (ctx.startFlag) return false;
        if (ramCount < 1) {
            ram.count = 1;
        } else if (256 < ramCount) {
            ram.count = 256;
        } else {
            ram.count = ramCount;
        }
        return true;
    }

    bool addRomData(const void* data, int dataSize)
    {
        if (ctx.startFlag) return false;
        const char* ptr = (const char*)data;
        while (0 < dataSize && rom.count < 256) {
            if (0x2000 <= dataSize) {
                memcpy(rom.data[rom.count], ptr, 0x2000);
                ptr += 0x2000;
                dataSize -= 0x2000;
            } else {
                memset(rom.data[rom.count], 0, 0x2000);
                memcpy(rom.data[rom.count], ptr, dataSize);
                dataSize -= dataSize;
            }
            rom.count++;
        }
        return true;
    }

    bool resetBanks(int ramStart, int ramEnd = 7)
    {
        if (ctx.startFlag) return false;
        ramStart &= 0b111;
        ramEnd &= 0b111;
        if (ramStart < ramEnd) {
            this->ctx.ramBankIndexStart = ramStart;
            this->ctx.ramBankIndexEnd = ramEnd;
        } else {
            this->ctx.ramBankIndexStart = ramEnd;
            this->ctx.ramBankIndexEnd = ramStart;
        }
        int romBankIndex = 0;
        for (int i = 0; i < ctx.ramBankIndexStart; i++) {
            ctx.banks[i] = romBankIndex++;
        }
        for (int i = ctx.ramBankIndexStart, n = 0; i <= ctx.ramBankIndexEnd; i++, n++) {
            ctx.banks[i] = n;
        }
        if (ctx.ramBankIndexEnd < 7) {
            for (int i = ctx.ramBankIndexEnd; i < 8; i++) {
                ctx.banks[i] = romBankIndex++;
            }
        }
        return true;
    }

    int execute(int clocks)
    {
        if (rom.count < 1 || ctx.endFlag) return 0;
        if (!ctx.startFlag) {
            for (auto handler : devices.startHandlers) handler->callback(this);
            ctx.startFlag = true;
        }
        return cpu->execute(clocks);
    }

    inline static unsigned char readMemory(void* ctx, unsigned short addr)
    {
        auto _this = (Z80Console*)ctx;
        if (!_this->ctx.startFlag || _this->ctx.endFlag) return 0xFF;
        unsigned char page = (addr & 0xFF00) >> 8;
        if (_this->devices.read[page]) {
            return _this->devices.read[page](ctx, addr);
        }
        int n = (addr & 0xE000) >> 13;
        if (_this->isRamIndex(n)) {
            return _this->ram.data[n % _this->ram.count][addr & 0x1FFF];
        } else {
            return _this->rom.data[n % _this->rom.count][addr & 0x1FFF];
        }
    }

    inline static void writeMemory(void* ctx, unsigned short addr, unsigned char value)
    {
        auto _this = (Z80Console*)ctx;
        if (!_this->ctx.startFlag || _this->ctx.endFlag) return;
        unsigned char page = (addr & 0xFF00) >> 8;
        if (_this->devices.write[page]) {
            _this->devices.write[page](ctx, addr, value);
            return;
        }
        int n = (addr & 0xE000) >> 13;
        if (_this->isRamIndex(n)) _this->ram.data[n % _this->ram.count][addr & 0x1FFF] = value;
    }

    inline static unsigned char inPort(void* ctx, unsigned char portNumber)
    {
        auto _this = (Z80Console*)ctx;
        if (!_this->ctx.startFlag || _this->ctx.endFlag) return 0xFF;
        if (_this->devices.in[portNumber]) {
            return _this->devices.in[portNumber](_this->cpu, portNumber);
        } else {
            if (portNumber < 8) return _this->ctx.banks[portNumber];
            if (0x0F == portNumber) {
                char buf[0x10000];
                printf("> ");
                memset(buf, 0, sizeof(buf));
                fgets(buf, sizeof(buf) - 1, stdin);
                unsigned short addr = _this->cpu->reg.pair.H;
                addr <<= 8;
                addr |= _this->cpu->reg.pair.L;
                unsigned short maxLength = _this->cpu->reg.pair.B;
                maxLength <<= 8;
                maxLength |= _this->cpu->reg.pair.C;
                unsigned short inputLength = strlen(buf);
                for (int i = 0; i < inputLength && i < maxLength; i++) {
                    _this->cpu->writeByte(addr++, buf[i]);
                }
                return 0;
            }
        }
        return 0xFF;
    }

    inline static void outPort(void* ctx, unsigned char portNumber, unsigned char value)
    {
        auto _this = (Z80Console*)ctx;
        if (!_this->ctx.startFlag || _this->ctx.endFlag) return;
        if (_this->devices.out[portNumber]) {
            _this->devices.out[portNumber](_this->cpu, portNumber, value);
        } else {
            if (portNumber < 8) {
                _this->ctx.banks[portNumber] = value;
            } else if (0x0F == portNumber) {
                char buf[257];
                unsigned short addr = _this->cpu->reg.pair.H;
                addr <<= 8;
                addr |= _this->cpu->reg.pair.L;
                for (int i = 0; i < value; i++) {
                    buf[i] = _this->cpu->readByte(addr++);
                }
                buf[value] = 0;
                printf("%s", buf);
            }
        }
    }
};
