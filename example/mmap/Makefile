CONSOLE=../../z80con
PROJECT=mmap

all: $(CONSOLE) $(PROJECT).bin libmmap.so
	make exec LD_LIBRARY_PATH=`pwd`

exec:
	$(CONSOLE) -v $(PROJECT).bin -m r C1 mmap:readAddr -m w C2 mmap:writeAddr 

clean:
	rm -f $(PROJECT).bin
	rm -f $(PROJECT).o
	rm -f $(CONSOLE) 
	rm -f libmmap.so

$(CONSOLE):
	cd ../.. && make

$(PROJECT).bin: $(PROJECT).asm
	z80asm -b $(PROJECT).asm

libmmap.so: mmap.cpp
	clang++ --std=c++11 -shared -fPIC -o libmmap.so mmap.cpp

