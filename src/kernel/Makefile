TARGET = kernel.elf

WORKDIR=../..
SOURCE_DIR=${WORKDIR}/src
BUILD_DIR=$(WORKDIR)/build
LIBC_DIR=$(HOME)/osbook/devenv/x86_64-elf
EDK2_DIR=$(HOME)/edk2
MKFONT_CMD=$(WORKDIR)/tools/mkfont/makefont.py

CPPFLAGS += -I$(LIBC_DIR)/include/c++/v1 -I$(LIBC_DIR)/include -I$(LIBC_DIR)/include/freetype2 \
            -I$(EDK2_DIR)/MdePkg/Include -I$(EDK2_DIR)/MdePkg/Include/X64 \
			-I$(SOURCE_DIR)/bootloader/Include \
			-I$(SOURCE_DIR)/kernel/include \
			-nostdlibinc  -D__ELF__ -D_LDBL_EQ_DBL -D_GNU_SOURCE -D_POSIX_TIMERS\
			 -DEFIAPI='__attribute__((ms_abi))'
CFLAGS   += -O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone
CXXFLAGS += -O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone \
	        -fno-exceptions -fno-rtti -std=c++17
LDFLAGS += -L${LIBC_DIR}/lib --entry KernelMain -z norelro -z separate-code --image-base 0x100000  --static\
	-z separate-code

.PHONY: all
all: $(TARGET)

kernel.elf:  Makefile main.o
	ld.lld $(LDFLAGS)  -o $(BUILD_DIR)/kernel.elf main.o -lc -lc++

%.o: %.cpp Makefile
	clang++ $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@