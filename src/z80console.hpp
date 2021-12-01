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

class Z80Console
{
  private:
    unsigned char ramBankIndexStart;
    unsigned char ramBankIndexEnd;
    inline bool isRamIndex(int n) { return ramBankIndexStart <= n && n <= ramBankIndexEnd; }

    struct ExternalDevices {
        void (*out[256])(void*, unsigned char, unsigned char);
        unsigned char (*in[256])(void*, unsigned char);
        void (*write[256])(void*, unsigned short, unsigned char);
        unsigned char (*read[256])(void*, unsigned short);
    } devices;

    int romCount;
    int ramCount;
    unsigned char rom[256][8192];
    unsigned char ram[256][8192];
    unsigned char banks[8];
    bool startFlag;
    bool endFlag;

  public:
    Z80* cpu;

    Z80Console()
    {
        cpu = new Z80(readMemory, writeMemory, inPort, outPort, this);
        cpu->addReturnHandler([](void* arg) {
            auto _this = (Z80Console*)arg;
            // Shutdown the ConsoleComputer when call the RET instruction when SP equals 0
            if (0 == _this->cpu->reg.SP) {
                _this->endFlag = true;
                _this->cpu->requestBreak();
            }
        });
        hardwareReset();
    }

    ~Z80Console()
    {
        if (cpu) delete cpu;
    }

    void hardwareReset()
    {
        romCount = 0;
        memset(rom, 0, sizeof(rom));
        ramCount = 256;
        ramBankIndexStart = 4;
        ramBankIndexEnd = 7;
        reset();
    }

    void reset()
    {
        memset(&devices, 0, sizeof(devices));
        memset(ram, 0, sizeof(ram));
        memset(&cpu->reg, 0, sizeof(cpu->reg));
        this->resetBanks(ramBankIndexStart, ramBankIndexEnd);
        this->startFlag = false;
        this->endFlag = false;
    }

    bool isEnded() { return this->endFlag; }
    int getRomCount() { return this->romCount; }
    int getRamCount() { return this->ramCount; }
    int getReturnCode() { return this->cpu->reg.pair.A; }

    bool addOutputDevice(unsigned char portNumber, void (*out)(void*, unsigned char, unsigned char))
    {
        if (startFlag) return false;
        devices.out[portNumber] = out;
        return true;
    }

    bool addInputDevice(unsigned char portNumber, unsigned char (*in)(void*, unsigned char))
    {
        if (startFlag) return false;
        devices.in[portNumber] = in;
        return true;
    }

    bool addWriteMemoryMap(unsigned short address, void (*write)(void*, unsigned short, unsigned char))
    {
        if (startFlag) return false;
        devices.write[(address & 0xFF00) >> 8] = write;
        return true;
    }

    bool addReadMemoryMap(unsigned short address, unsigned char (*read)(void*, unsigned short))
    {
        if (startFlag) return false;
        devices.read[(address & 0xFF00) >> 8] = read;
        return true;
    }

    bool setRamCount(int ramCount)
    {
        if (startFlag) return false;
        if (ramCount < 1) {
            this->ramCount = 1;
        } else if (256 < ramCount) {
            this->ramCount = 256;
        } else {
            this->ramCount = ramCount;
        }
        return true;
    }

    bool addRomData(const void* data, int dataSize)
    {
        if (startFlag) return false;
        const char* ptr = (const char*)data;
        while (0 < dataSize && romCount < 256) {
            if (0x2000 <= dataSize) {
                memcpy(rom[romCount], ptr, 0x2000);
                ptr += 0x2000;
                dataSize -= 0x2000;
            } else {
                memset(rom[romCount], 0, 0x2000);
                memcpy(rom[romCount], ptr, dataSize);
                dataSize -= dataSize;
            }
            romCount++;
        }
        return true;
    }

    bool resetBanks(int ramStart, int ramEnd = 7)
    {
        if (startFlag) return false;
        ramStart &= 0b111;
        ramEnd &= 0b111;
        if (ramStart < ramEnd) {
            this->ramBankIndexStart = ramStart;
            this->ramBankIndexEnd = ramEnd;
        } else {
            this->ramBankIndexStart = ramEnd;
            this->ramBankIndexEnd = ramStart;
        }
        int romBankIndex = 0;
        for (int i = 0; i < ramBankIndexStart; i++) {
            this->banks[i] = romBankIndex++;
        }
        for (int i = ramBankIndexStart, n = 0; i <= ramBankIndexEnd; i++, n++) {
            this->banks[i] = n;
        }
        if (ramBankIndexEnd < 7) {
            for (int i = ramBankIndexEnd; i < 8; i++) {
                this->banks[i] = romBankIndex++;
            }
        }
        return true;
    }

    int execute(int clocks)
    {
        if (romCount < 1 || endFlag) return 0;
        startFlag = true;
        return cpu->execute(clocks);
    }

    inline static unsigned char readMemory(void* ctx, unsigned short addr)
    {
        auto _this = (Z80Console*)ctx;
        if (!_this->startFlag || _this->endFlag) return 0xFF;
        unsigned char page = (addr & 0xFF00) >> 8;
        if (_this->devices.read[page]) {
            return _this->devices.read[page](ctx, addr);
        }
        int n = (addr & 0xE000) >> 13;
        if (_this->isRamIndex(n)) {
            return _this->ram[n % _this->ramCount][addr & 0x1FFF];
        } else {
            return _this->rom[n % _this->romCount][addr & 0x1FFF];
        }
    }

    inline static void writeMemory(void* ctx, unsigned short addr, unsigned char value)
    {
        auto _this = (Z80Console*)ctx;
        if (!_this->startFlag || _this->endFlag) return;
        unsigned char page = (addr & 0xFF00) >> 8;
        if (_this->devices.write[page]) {
            _this->devices.write[page](ctx, addr, value);
            return;
        }
        int n = (addr & 0xE000) >> 13;
        if (_this->isRamIndex(n)) _this->ram[n % _this->ramCount][addr & 0x1FFF] = value;
    }

    inline static unsigned char inPort(void* ctx, unsigned char portNumber)
    {
        auto _this = (Z80Console*)ctx;
        if (!_this->startFlag || _this->endFlag) return 0xFF;
        if (_this->devices.in[portNumber]) {
            return _this->devices.in[portNumber](_this->cpu, portNumber);
        } else {
            if (portNumber < 8) return _this->banks[portNumber];
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
        if (!_this->startFlag || _this->endFlag) return;
        if (_this->devices.out[portNumber]) {
            _this->devices.out[portNumber](_this->cpu, portNumber, value);
        } else {
            if (portNumber < 8) {
                _this->banks[portNumber] = value;
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
