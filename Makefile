all:
	make -C boot
	make -C kernel
	ld -static boot/boot.o -Lkernel -lkernel -o vmunix

clean:
	make -C boot clean
	make -C kernel clean
	rm -f vmunix iso/boot/vmunix
	rm -f os.iso

test: vmunix
	kvm -kernel vmunix -smp cores=4,sockets=4 -m 5G

debug: vmunix
	kvm -kernel vmunix -s -smp cores=4,sockets=4 -m 5G

bochs-debug: vmunix
	cp vmunix iso/boot/
	grub-mkrescue -o os.iso iso/
	bochs
