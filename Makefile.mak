KDIR ?= /lib/modules/$$(uname -r)/build
BUILD_DIR ?= $(PWD)/build
BUILD_DIR_MAKEFILE ?= $(PWD)/build/Makefile

default: $(BUILD_DIR_MAKEFILE)
    make -C $(KDIR) M=$(BUILD_DIR) src=$(PWD) modules_install

$(BUILD_DIR):
    mkdir -p "$@"

$(BUILD_DIR_MAKEFILE): $(BUILD_DIR)
    touch "$@"

clean:
    make -C $(KDIR) M=$(BUILD_DIR) src=$(PWD) clean

obj-m := module.ko