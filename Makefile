WORKDIR=$(CURDIR)
SOURCE_DIR=$(WORKDIR)/src
BUILD_DIR=$(WORKDIR)/build
EDK2_DIR=$(HOME)/edk2
KERNEL_ELF=$(BUILD_DIR)/kernel.elf

CXXFLAGS=-O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone \
	-fno-exceptions -fno-rtti -std=c++17
CFLAGS=-O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone

BASEDIR := $(HOME)/osbook/devenv/x86_64-elf
CPPFLAGS := -I$(BASEDIR)/include/c++/v1 -I$(BASEDIR)/include -I$(BASEDIR)/include/freetype2 \
    -I$(EDK2DIR)/MdePkg/Include -I$(EDK2DIR)/MdePkg/Include/X64 \
    -nostdlibinc -D__ELF__ -D_LDBL_EQ_DBL -D_GNU_SOURCE -D_POSIX_TIMERS \
    -DEFIAPI='__attribute__((ms_abi))'
LDFLAGS := -L$(BASEDIR)/lib --entry=KernelMain -z norelro --image-base 0x100000 --static \
	-z separate-code

.PHONY: all
all: loader kernel.elf

# If you are the first time to build,
# you may have to execute `ln -s $HOME/fe56OS/src/FePkg $HOME/edk`.
# and edit $(HOME)/edk2/Conf/target.txt
.PHONY: loader
loader: $(EDK2_DIR)/edksetup.sh
	WORKSPACE=$(EDK2_DIR) source $(EDK2_DIR)/edksetup.sh;\
	WORKSPACE=$(EDK2_DIR) build

.PHONY:kernel
kernel: $(SOURCE_DIR)/kernel/main.cpp
	clang++ $(CXXFLAGS) $(CPPFLAGS)\
		-c $(SOURCE_DIR)/kernel/main.cpp -o $(BUILD_DIR)/main.o
	ld.lld $(LDFLAGS) -o $(KERNEL_ELF) $(BUILD_DIR)/main.o

LOADER_EFI=$(EDK2_DIR)/Build/FeX64/DEBUG_CLANG38/X64/Loader.efi
MOUNT_POINT=$(WORKDIR)/mnt
disk.img: loader kernel
	rm -rf $@
	qemu-img create -f raw $@ 200M
	mkfs.fat -n 'FeOS' -s 2 -f 2 -R 32 -F 32 $@
	mkdir -p ${MOUNT_POINT} 
	sudo mount -o loop $@ ${MOUNT_POINT}
	sleep 0.5
	sudo mkdir -p ${MOUNT_POINT}/EFI/BOOT
	sudo cp $(LOADER_EFI) ${MOUNT_POINT}/EFI/BOOT/BOOTX64.EFI
	sudo cp $(KERNEL_ELF) ${MOUNT_POINT}/
	sleep 0.5
	sudo umount mnt
	sleep 0.5

run: disk.img
	qemu-system-x86_64 \
		-m 1G \
		-drive if=pflash,format=raw,readonly,file=$(WORKDIR)/tools/OVMF_CODE.fd \
		-drive if=pflash,format=raw,file=$(WORKDIR)/tools/OVMF_VARS.fd \
		-drive if=ide,index=0,media=disk,format=raw,file=$< \
		-device nec-usb-xhci,id=xhci \
		-device usb-mouse -device usb-kbd \
		-monitor stdio \
		-s

.PHONY: clean
clean:
	@rm -f $(BUILD_DIR)/*

