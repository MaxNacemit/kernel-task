KDIR ?= /lib/modules/$$(uname -r)/build

default:
	make -C $(KDIR) M=$(CURDIR) modules

clean:
	make -C $(KDIR) M=$(CURDIR) clean
