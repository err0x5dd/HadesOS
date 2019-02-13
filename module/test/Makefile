SRCS = $(shell find . -type f -name '*.[cS]')
OBJS = $(addsuffix .o, $(basename $(SRCS)))

CC = gcc
LD = ld

ASFLAGS = -m32
CFLAGS = -m32 -Wall -g -fno-stack-protector -I include
LDFLAGS = -melf_i386 -Ttest.ld

test: $(OBJS)
	$(LD) $(LDFLAGS) -o ../../bin/module/test.ko $^
	objdump -S ../../bin/module/test.ko >debug.txt

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
