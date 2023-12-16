ASM=nasm
ASMFLAGS=-felf64
LD = ld

main.o: main.asm colon.inc lib.inc
	$(ASM) $(ASMFLAGS) -o $@ $<

%.o: %.asm
	$(ASM) $(ASMFLAGS) -o $@ $<

main: main.o dict.o lib.o
	$(LD) -o $@ $^

.PHONY: clean

clean:
	rm *.o main