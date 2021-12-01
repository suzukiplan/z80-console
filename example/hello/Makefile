CONSOLE=../../z80con
PROJECT=hello

all: $(CONSOLE) $(PROJECT).bin
	$(CONSOLE) -v $(PROJECT).bin

clean:
	rm -f $(PROJECT).bin
	rm -f $(PROJECT).o
	rm -f $(CONSOLE) 

$(CONSOLE):
	cd ../.. && make

$(PROJECT).bin: $(PROJECT).asm
	z80asm -b $(PROJECT).asm

