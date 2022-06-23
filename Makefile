KDIR ?= /lib/modules/$$(uname -r)/build

default:
    make -C $(KDIR) M=$(PWD) modules

clean:
    make -C $(KDIR) M=$(BUILD_DIR) src=$(PWD) clean
