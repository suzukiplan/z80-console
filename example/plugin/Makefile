CONSOLE=../../z80con
PROJECT=plugin
LD_LIBRARY_PATH=`pwd`

all: $(CONSOLE) $(PROJECT).bin libplugin.so
	make exec LD_LIBRARY_PATH=`pwd`

exec:
	$(CONSOLE) -v $(PROJECT).bin -p i C0 plugin:in -p o C1 plugin:out 

clean:
	rm -f $(PROJECT).bin
	rm -f $(PROJECT).o
	rm -f $(CONSOLE) 
	rm -f libplugin.so

$(CONSOLE):
	cd ../.. && make

$(PROJECT).bin: $(PROJECT).asm
	z80asm -b $(PROJECT).asm

libplugin.so: plugin.cpp
	clang++ --std=c++11 -shared -fPIC -o libplugin.so plugin.cpp

