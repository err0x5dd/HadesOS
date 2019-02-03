SRCS = $(shell find . -type f -name '*.[cS]')
OBJS = $(addsuffix .o, $(basename $(SRCS)))

CC = gcc
LD = ld

ASFLAGS = -m32
CFLAGS = -m32 -Wall -g -fno-stack-protector -I include
LDFLAGS = -melf_i386 -Tkernel.ld

kernel: $(OBJS)
	$(LD) $(LDFLAGS) -o ../bin/$@ $^
	objdump -S ../bin/kernel >debug.txt

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^
	git add $^

%.o: %.S
	$(CC) $(ASFLAGS) -c -o $@ $^
	git add $^

clean:
	rm $(OBJS)
	clear

run:
	qemu-system-x86_64 -serial file:output.txt -kernel ../bin/kernel -m 1024

.PHONY: clean all
