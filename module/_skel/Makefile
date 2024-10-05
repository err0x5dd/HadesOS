SRCS = $(shell find . -type f -name '*.[cS]')
OBJS = $(addsuffix .o, $(basename $(SRCS)))
OUTP = $(basename $(shell pwd))

CC = i686-elf-gcc
LD = i686-elf-ld

ASFLAGS = -m32
CFLAGS = -m32 -Wall -g -fno-stack-protector -I _include
LDFLAGS = -melf_i386 -Tmodule.ld

module: $(OBJS)
	$(LD) $(LDFLAGS) -o ../../bin/module/$(OUTP).ko $^
	objdump -S ../../bin/module/$(OUTP).ko >debug.txt

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^
	git add $^

%.o: %.S
	$(CC) $(ASFLAGS) -c -o $@ $^
	git add $^

clean:
	rm $(OBJS)
	clear

.PHONY: clean all
