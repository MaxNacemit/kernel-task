KDIR ?= /lib/modules/$$(uname -r)/build
bj-m += task2.o


default:
	make -C $(KDIR) M=$(CURDIR) modules

clean:
	make -C $(KDIR) M=$(CURDIR) clean
