all:
	cd kernel/ && make
	cd module/test/ && make

clean:
	cd kernel/ && make clean
	cd module/test/ && make clean


run: run-test

run-test:
	qemu-system-x86_64 -serial file:output.txt -kernel bin/kernel -initrd bin/module/test.ko -m 1024

run-plain:
	qemu-system-x86_64 -serial file:output.txt -kernel bin/kernel -m 1024
