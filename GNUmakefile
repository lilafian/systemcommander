# SPDX-License-Identifier: GPL-3.0-or-later
# Defines rules for building and testing parts of the kernel.
# Copyright (C) 2026 lilaf */

.SUFFIXES:

override KBINOUTPUT := systemcommander
override IMGOUTPUT := build/systemcommander.img
override DEFFONTNAME := ter-116n
override OVMF_SYS_PATH := /usr/share/OVMF/x64
CC := gcc
NASM := nasm
LD := ld
CFLAGS := -g -O0 -pipe
QEMUFLAGS := -machine q35 -m 1g -no-reboot
NASMFLAGS := -g
LDFLAGS := 

override CFLAGS += \
	-Wall \
	-Wextra \
	-std=gnu11 \
	-ffreestanding \
	-fno-stack-protector \
	-fno-stack-check \
	-fno-lto \
	-fno-PIC \
	-ffunction-sections \
	-fdata-sections \
	-m64 \
	-march=x86-64 \
	-mabi=sysv \
	-mno-80387 \
	-mno-mmx \
	-mno-sse \
	-mno-sse2 \
	-mno-red-zone \
	-mcmodel=kernel \
	-Isrc/include \

override QEMUFLAGS += \
	-drive format=raw,media=disk,file=$(IMGOUTPUT) \
	-drive if=pflash,format=raw,readonly=on,file=$(OVMF_SYS_PATH)/OVMF_CODE.4m.fd \
	-drive if=pflash,format=raw,file=ovmf/OVMF_VARS.4m.fd \

override NASMFLAGS += \
	-f elf64 \
	$(patsubst -g,-g -F dwarf,$(NASMFLAGS)) \
	-Wall

override LDFLAGS += \
	-m elf_x86_64 \
	-nostdlib \
	-static \
	-z max-page-size=0x1000 \
	--gc-sections \
	-T linker.lds

override ALL_SRC := $(shell find -L src -type f 2>/dev/null | LC_ALL=C sort)
override C_FILES := $(filter %.c,$(ALL_SRC))
override ASM_FILES := $(filter %.asm,$(ALL_SRC))
override OBJ := $(addprefix obj/,$(C_FILES:.c=.c.o) $(ASM_FILES:.asm=.asm.o))
override HEADER_DEPS := $(addprefix obj/,$(CFILES:.c=.c.d) $(ASFILES:.S=.S.d))

.PHONY: all
all: build/$(KBINOUTPUT)

-include $(HEADER_DEPS)

build/$(KBINOUTPUT): GNUmakefile linker.lds $(OBJ)
	mkdir -p "$(dir $@)"
	$(LD) $(LDFLAGS) $(OBJ) -o $@

obj/%.c.o: %.c GNUmakefile
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) -c $< -o $@

obj/%.asm.o: %.asm GNUmakefile
	mkdir -p "$(dir $@)"
	$(NASM) $(NASMFLAGS) $< -o $@

.PHONY: bootimg-efi
bootimg-efi:
	make -C limine
	mkdir -p build/img/boot
	cp -v build/$(KBINOUTPUT) build/img/boot
	cp -v assets/fonts/$(DEFFONTNAME).psf build/img/boot/deffont.psf
	mkdir -p build/img/boot/limine
	cp -v limine.conf limine/limine-uefi-cd.bin build/img/boot/limine
	mkdir -p build/img/EFI/BOOT
	cp -v limine/BOOTX64.EFI build/img/EFI/BOOT
	cp -v limine/BOOTIA32.EFI build/img/EFI/BOOT
	dd if=/dev/zero of=$(IMGOUTPUT) bs=4M count=256 status=progress conv=fsync # 1g image
	parted $(IMGOUTPUT) mklabel gpt
	parted -a optimal $(IMGOUTPUT) mkpart primary 1MiB 100%
	LOOP=$$(sudo losetup --partscan --show --find $(IMGOUTPUT)); \
	sudo mkfs.fat -F 32 $${LOOP}p1; \
	mkdir -p build/imgmp; \
	sudo mount $${LOOP}p1 build/imgmp; \
	sudo cp -rv build/img/* build/imgmp; \
	sudo umount build/imgmp; \
	sudo losetup -d $${LOOP}

.PHONY: bootimg-bios
bootimg-bios:
	make -C limine
	mkdir -p build/img/boot
	cp -v build/$(KBINOUTPUT) build/img/boot
	cp -v assets/fonts/$(DEFFONTNAME).psf build/img/boot/deffont.psf
	mkdir -p build/img/boot/limine
	cp -v limine.conf limine/limine-bios.sys limine/limine-bios-cd.bin build/img/boot/limine
	dd if=/dev/zero of=$(IMGOUTPUT) bs=4M count=256 status=progress conv=fsync # 1g image
	parted $(IMGOUTPUT) mklabel msdos
	parted -a optimal $(IMGOUTPUT) mkpart primary 1MiB 100%
	LOOP=$$(sudo losetup --partscan --show --find $(IMGOUTPUT)); \
	sudo mkfs.fat -F 32 $${LOOP}p1; \
	mkdir -p build/imgmp; \
	sudo mount $${LOOP}p1 build/imgmp; \
	sudo cp -rv build/img/* build/imgmp; \
	sudo umount build/imgmp; \
	sudo losetup -d $${LOOP}
	limine/limine bios-install $(IMGOUTPUT)

.PHONY: clean
clean:
	rm -rf build obj

.PHONY: vmprepare
vmprepare:
	mkdir -p ovmf
	sudo cp $(OVMF_SYS_PATH)/OVMF_VARS.4m.fd ovmf
	sudo chmod a+rwx ovmf/OVMF_VARS.4m.fd

.PHONY: qemu
qemu:
	qemu-system-x86_64 \
		-serial stdio \
		$(QEMUFLAGS)

.PHONY: qemu-nographic
qemu-nographic:
	qemu-system-x86_64 \
		-vga none \
		-nographic \
		$(QEMUFLAGS)

.PHONY: qemu-gdb
qemu-gdb:
	qemu-system-x86_64 \
		-s -S \
		-serial stdio \
		$(QEMUFLAGS)

.PHONY: qemu-gdb-nographic
qemu-gdb-nographic:
	qemu-system-x86_64 \
		-vga none \
		-nographic \
		-s -S \
		$(QEMUFLAGS)
