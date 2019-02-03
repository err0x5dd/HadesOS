all:
	cd kernel/ && make
	cd module/test/ && make
	cd module/test2/ && make

run: run-test

run-test:
	qemu-system-x86_64 -serial file:output.txt -kernel bin/kernel -initrd bin/module/test.ko,bin/module/test2.ko -m 1024

run-plain:
	qemu-system-x86_64 -serial file:output.txt -kernel bin/kernel -m 1024
