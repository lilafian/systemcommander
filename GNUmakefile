.SUFFIXES:

override KBINOUTPUT := systemcommander
override ISOOUTPUT := build/systemcommander.iso
override DEFFONTNAME := ter-116n
override OVMF_SYS_PATH := /usr/share/OVMF/x64
CC := gcc
LD := ld
CFLAGS := -g -O0 -pipe
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

override LDFLAGS += \
	-m elf_x86_64 \
	-nostdlib \
	-static \
	-z max-page-size=0x1000 \
	--gc-sections \
	-T linker.lds

override ALL_SRC := $(shell find -L src -type f 2>/dev/null | LC_ALL=C sort)
override C_FILES := $(filter %.c,$(ALL_SRC))
override OBJ := $(addprefix obj/,$(C_FILES:.c=.c.o))
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

.PHONY: iso
iso:
	make -C limine
	mkdir -p build/iso
	mkdir -p build/iso/boot
	cp -v build/$(KBINOUTPUT) build/iso/boot
	cp -v assets/fonts/$(DEFFONTNAME).psf build/iso/boot/deffont.psf
	mkdir -p build/iso/boot/limine
	cp -v limine.conf limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin build/iso/boot/limine
	mkdir -p build/iso/EFI/BOOT
	cp -v limine/BOOTX64.EFI build/iso/EFI/BOOT
	cp -v limine/BOOTIA32.EFI build/iso/EFI/BOOT
	xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
		-apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		build/iso -o $(ISOOUTPUT)
	limine/limine bios-install $(ISOOUTPUT)

.PHONY: clean
clean:
	rm -rf build obj

.PHONY: vmprepare
vmprepare:
	mkdir -p ovmf
	sudo cp $(OVMF_SYS_PATH)/OVMF_VARS.4m.fd ovmf
	sudo chmod a+rwx ovmf/OVMF_VARS.4m.fd

.PHONY: qemutest
qemutest:
	qemu-system-x86_64 \
		-machine pc \
		-m 1g \
		-drive if=pflash,format=raw,readonly=on,file=$(OVMF_SYS_PATH)/OVMF_CODE.4m.fd \
		-drive if=pflash,format=raw,file=ovmf/OVMF_VARS.4m.fd \
		-drive format=raw,media=cdrom,file=$(ISOOUTPUT) \
		-serial stdio \
		-no-reboot

.PHONY: qemutest-gdb
qemutest-gdb:
	qemu-system-x86_64 \
		-s -S \
		-machine pc \
		-m 1g \
		-drive if=pflash,format=raw,readonly=on,file=$(OVMF_SYS_PATH)/OVMF_CODE.4m.fd \
		-drive if=pflash,format=raw,file=ovmf/OVMF_VARS.4m.fd \
		-drive format=raw,media=cdrom,file=$(ISOOUTPUT) \
		-serial stdio \
		-no-reboot
