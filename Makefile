CFILES = $(shell find . -name "*.c")
OFILES = $(CFILES:.c=.o)

all: kernel8.img

LLVMPATH = /usr/local/opt/llvm/bin
CLANGFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -mcpu=cortex-a53+nosimd

boot.o: **/boot.S
	$(LLVMPATH)/clang --target=aarch64-elf $(CLANGFLAGS) -c **/boot.S -o boot.o

%.o: %.c
	$(LLVMPATH)/clang --target=aarch64-elf $(CLANGFLAGS) -c $< -o $@
	
font_psf.o: font.psf
	$(LLVMPATH)/ld.lld -m aarch64elf -r -b binary -o font_psf.o font.psf

font_sfn.o: font.sfn
	$(LLVMPATH)/ld.lld -m aarch64elf -r -b binary -o font_sfn.o font.sfn
	
kernel8.img: boot.o font_psf.o font_sfn.o $(OFILES)
	$(LLVMPATH)/ld.lld -m aarch64elf -nostdlib boot.o font_psf.o font_sfn.o $(OFILES) -T link.ld -o kernel8.elf
	$(LLVMPATH)/llvm-objcopy -O binary kernel8.elf kernel8.img

all: finalize

clean:
	/bin/rm kernel8.elf **/*.o **/*.img

finalize:
	/bin/rm kernel8.elf $(shell find . -name "*.o")
