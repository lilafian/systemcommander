# Building the kernel
This guide will walk you through building System Commander.
## Dependencies
* gcc
* nasm
* parted (if building full test image)
* sudo (if building full test image)
* dosfstools (mkfs.fat, if building full test image)
* qemu (if running in a vm)
* ovmf (if using efi in a vm (which is recommended))
## Building
### Just the kernel
```bash
make
```
### Full test image
```bash
git submodule update --init
make
make testimg-efi # for efi/gpt
make testimg-bios # for bios/mbr (unsupported as of now)
```
### To run with QEMU
```bash
# first, build the test image
make vmprepare # if using efi
make qemu # graphics, no debugging
make qemu-nographic # serial only, no debugging
make qemu-gdb # graphics, with debugging
make qemu-gdb-nographic # no graphics, with debugging
```
