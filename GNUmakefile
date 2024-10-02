# Nuke built-in rules and variables.
override MAKEFLAGS += -rR

export KCC=x86_64-elf-gcc
export KLD=x86_64-elf-ld

override QEMUFLAGS := -m 2G
override IMAGE_NAME := m4xdevOS-x86_64

.PHONY: all
all: $(IMAGE_NAME).iso

.PHONY: all-hdd
all-hdd: $(IMAGE_NAME).hdd

.PHONY: run
run: run-x86_64

.PHONY: run-debug
run-debug: run-x86_64-debug

.PHONY: run-hdd
run-hdd: run-hdd-x86_64

.PHONY: run-x86_64
run-x86_64: ovmf/ovmf-code-x86_64.fd ovmf/ovmf-vars-x86_64.fd $(IMAGE_NAME).iso
	qemu-system-x86_64 \
		-M q35 \
		-no-reboot \
		-no-shutdown \
		-serial file:serial.log \
		-d int \
		-drive if=pflash,unit=0,format=raw,file=ovmf/ovmf-code-x86_64.fd,readonly=on \
		-drive if=pflash,unit=1,format=raw,file=ovmf/ovmf-vars-x86_64.fd \
		-cdrom $(IMAGE_NAME).iso \
		$(QEMUFLAGS)

.PHONY: run-hdd-x86_64
run-hdd-x86_64: ovmf/ovmf-code-x86_64.fd ovmf/ovmf-vars-x86_64.fd $(IMAGE_NAME).hdd
	qemu-system-x86_64 \
		-M q35 \
		-d int \
		-serial file:serial.log \
		-no-reboot \
		-no-shutdown \
		-drive if=pflash,unit=0,format=raw,file=ovmf/ovmf-code-x86_64.fd,readonly=on \
		-drive if=pflash,unit=1,format=raw,file=ovmf/ovmf-vars-x86_64.fd \
		-hda $(IMAGE_NAME).hdd \
		$(QEMUFLAGS)

.PHONY: run-x86_64-debug
run-x86_64-debug: ovmf/ovmf-code-x86_64.fd ovmf/ovmf-vars-x86_64.fd $(IMAGE_NAME).iso
	qemu-system-x86_64 \
		-M q35 \
		-s -S \
		-no-reboot \
		-no-shutdown \
		-serial file:serial.log \
		-monitor stdio\
		-drive if=pflash,unit=0,format=raw,file=ovmf/ovmf-code-x86_64.fd,readonly=on \
		-drive if=pflash,unit=1,format=raw,file=ovmf/ovmf-vars-x86_64.fd \
		-cdrom $(IMAGE_NAME).iso \
		$(QEMUFLAGS)

.PHONY: run-bios
run-bios: $(IMAGE_NAME).iso
	qemu-system-x86_64 \
		-M q35 \
		-cdrom $(IMAGE_NAME).iso \
		-boot d \
		$(QEMUFLAGS)

.PHONY: run-hdd-bios
run-hdd-bios: $(IMAGE_NAME).hdd
	qemu-system-x86_64 \
		-M q35 \
		-hda $(IMAGE_NAME).hdd \
		$(QEMUFLAGS)

ovmf/ovmf-code-x86_64.fd:
	mkdir -p ovmf
	curl -Lo $@ https://github.com/limine-bootloader/edk2-ovmf-nightly/releases/latest/download/ovmf-code-x86_64.fd

ovmf/ovmf-vars-x86_64.fd:
	mkdir -p ovmf
	curl -Lo $@ https://github.com/limine-bootloader/edk2-ovmf-nightly/releases/latest/download/ovmf-vars-x86_64.fd

limine/limine:
	rm -rf limine
	git clone https://github.com/limine-bootloader/limine.git --branch=v8.x-binary --depth=1
	$(MAKE) -C limine

kernel-deps:
	./kernel/get-deps
	touch kernel-deps

.PHONY: kernel
kernel: kernel-deps
	$(MAKE) -C kernel

$(IMAGE_NAME).iso: limine/limine kernel
	rm -rf iso_root
	mkdir -p iso_root/boot
	cp -v kernel/bin-x86_64/kernel iso_root/boot/
	mkdir -p iso_root/boot/limine
	cp -v limine.conf iso_root/boot/limine/
	mkdir -p iso_root/EFI/BOOT
	cp -v limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin iso_root/boot/limine/
	cp -v limine/BOOTX64.EFI iso_root/EFI/BOOT/
	cp -v limine/BOOTIA32.EFI iso_root/EFI/BOOT/
	xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o $(IMAGE_NAME).iso
	./limine/limine bios-install $(IMAGE_NAME).iso
	rm -rf iso_root

$(IMAGE_NAME).hdd: limine/limine kernel
	rm -f $(IMAGE_NAME).hdd
	dd if=/dev/zero bs=1M count=0 seek=64 of=$(IMAGE_NAME).hdd
	sgdisk $(IMAGE_NAME).hdd -n 1:2048 -t 1:ef00
	./limine/limine bios-install $(IMAGE_NAME).hdd
	mformat -i $(IMAGE_NAME).hdd@@1M
	mmd -i $(IMAGE_NAME).hdd@@1M ::/EFI ::/EFI/BOOT ::/boot ::/boot/limine
	mcopy -i $(IMAGE_NAME).hdd@@1M kernel/bin-x86_64/kernel ::/boot
	mcopy -i $(IMAGE_NAME).hdd@@1M limine.conf ::/boot/limine
	mcopy -i $(IMAGE_NAME).hdd@@1M limine/limine-bios.sys ::/boot/limine
	mcopy -i $(IMAGE_NAME).hdd@@1M limine/BOOTX64.EFI ::/EFI/BOOT
	mcopy -i $(IMAGE_NAME).hdd@@1M limine/BOOTIA32.EFI ::/EFI/BOOT

.PHONY: clean
clean:
	if test -f kernel-deps; then $(MAKE) -C kernel clean; fi
	rm -rf iso_root $(IMAGE_NAME).iso $(IMAGE_NAME).hdd

.PHONY: distclean
distclean:
	if test -f kernel-deps; then $(MAKE) -C kernel distclean; fi
	rm -rf iso_root *.iso *.hdd kernel-deps limine ovmf
