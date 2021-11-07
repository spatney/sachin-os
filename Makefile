GCCPARAMS = -m32 -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -Wno-write-strings
ASPARAMS = --32
LDPARAMS = -melf_i386

objects = loader.o lib/port.o lib/gdt.o lib/stdlib.o lib/interruptstubs.o lib/interrupts.o lib/driver.o lib/keyboard.o lib/mouse.o kernel.o

%.o: %.cpp
	g++ $(GCCPARAMS) -c -o $@ $<

%.o: %.s
	as $(ASPARAMS) -o $@ $<

kernel.bin: linker.ld $(objects)
	ld $(LDPARAMS) -T $< -o $@ $(objects)
	rm $(objects)

install: kernel.bin
	sudo cp $< /boot/kernel.bin

kernel.iso: kernel.bin
	mkdir iso
	mkdir iso/boot
	mkdir iso/boot/grub
	cp $< iso/boot/
	echo 'set timeout=0' > iso/boot/grub/grub.cfg
	echo 'set default=0' >> iso/boot/grub/grub.cfg
	echo 'menuentry "PixelMite" {' >> iso/boot/grub/grub.cfg
	echo '	multiboot /boot/kernel.bin' >> iso/boot/grub/grub.cfg
	echo '	boot' >> iso/boot/grub/grub.cfg
	echo '}' >> iso/boot/grub/grub.cfg
	grub-mkrescue --output=$@ iso
	rm -rf iso
	rm kernel.bin

.phony: clean
clean:
	rm -f $(objects) kernel.bin kernel.iso

run: kernel.iso
	(pkill VirtualBoxVM) || true
	VirtualBoxVM --startvm "MoOS" &

wsl-run: kernel.iso
	(powershell.exe "taskkill /IM "VirtualBoxVM.exe" /F") || true
	/mnt/c/Program\ Files/Oracle/VirtualBox/VirtualBoxVM.exe --startvm "MoOS" &