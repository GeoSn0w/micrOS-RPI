CFILES = $(shell find . -name "*.c")
OFILES = $(CFILES:.c=.o)

all: kernel8.img

LLVMPATH = /usr/local/opt/llvm/bin
CLANGFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -mcpu=cortex-a53+nosimd

boot.o: **/boot.S
	$(LLVMPATH)/clang --target=aarch64-elf $(CLANGFLAGS) -c **/boot.S -o boot.o

%.o: %.c
	$(LLVMPATH)/clang --target=aarch64-elf $(CLANGFLAGS) -c $< -o $@

kernel8.img: boot.o $(OFILES)
	$(LLVMPATH)/ld.lld -m aarch64elf -nostdlib boot.o $(OFILES) -T link.ld -o kernel8.elf
	$(LLVMPATH)/llvm-objcopy -O binary kernel8.elf kernel8.img

all: finalize

clean:
	/bin/rm kernel8.elf **/*.o **/*.img

finalize:
	/bin/rm kernel8.elf **/*.o boot.o
