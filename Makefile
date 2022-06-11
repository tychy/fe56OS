SOURCEDIR:=src
BUILDDIR:=build
KERNEL:=$(BUILDDIR)/kernel.elf

kernel: $(SOURCEDIR)
	clang++ -O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone -fno-exceptions -fno-rtti -std=c++17 \
		-c $(SOURCEDIR)/kernel/main.cpp -o $(BUILDDIR)/main.o
	ld.lld --entry KernelMain -z separate-code --image-base 0x100000 --static -o $(KERNEL) $(BUILDDIR)/main.o

.PHONY: clean
clean:
	@rm -f $(BUILDDIR)/*

