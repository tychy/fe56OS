TARGET = kernel.elf

WORKDIR=../..
SOURCE_DIR=${WORKDIR}/src/kernel
BUILD_DIR=$(WORKDIR)/build
LIBC_DIR=$(HOME)/osbook/devenv/x86_64-elf
EDK2_DIR=$(HOME)/edk2
MKFONT_CMD=$(WORKDIR)/tools/makefont.py

SOURCES=$(wildcard $(SOURCE_DIR)/*.cpp)
OBJS=$(SOURCES:.cpp=.o) hankaku.o newlib_support.o asmfunc.o \
        usb/memory.o usb/device.o usb/xhci/ring.o usb/xhci/trb.o usb/xhci/xhci.o \
        usb/xhci/port.o usb/xhci/device.o usb/xhci/devmgr.o usb/xhci/registers.o \
        usb/classdriver/base.o usb/classdriver/hid.o usb/classdriver/keyboard.o \
        usb/classdriver/mouse.o


CPPFLAGS += -I$(LIBC_DIR)/include/c++/v1 -I$(LIBC_DIR)/include -I$(LIBC_DIR)/include/freetype2 \
            -I$(EDK2_DIR)/MdePkg/Include -I$(EDK2_DIR)/MdePkg/Include/X64 \
			-I$(SOURCE_DIR)/.\
			-nostdlibinc  -D__ELF__ -D_LDBL_EQ_DBL -D_GNU_SOURCE -D_POSIX_TIMERS\
			 -DEFIAPI='__attribute__((ms_abi))'
CFLAGS   += -O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone
CXXFLAGS += -O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone \
	        -fno-exceptions -fno-rtti -std=c++17
LDFLAGS += -L${LIBC_DIR}/lib --entry KernelMain -z norelro -z separate-code --image-base 0x100000  --static\
	-z separate-code


.PHONY: all
all: $(TARGET)

kernel.elf:  Makefile $(OBJS) 
	ld.lld $(LDFLAGS)  -o $(BUILD_DIR)/kernel.elf $(OBJS) -lc -lc++

%.o: %.cpp Makefile
	clang++ $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

%.o: %.c Makefile
	clang $(CPPFLAGS) $(CFLAGS) -c $<

%.o: %.asm Makefile
	nasm -f elf64 -o $@ $<

hankaku.o:  hankaku.txt
	$(MKFONT_CMD) -o hankaku.bin hankaku.txt
	objcopy -I binary -O elf64-x86-64 -B i386:x86-64 hankaku.bin hankaku.o