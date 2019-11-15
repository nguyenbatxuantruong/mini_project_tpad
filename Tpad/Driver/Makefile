KDIR = /lib/modules/$(shell uname -r)/build
obj-m = tgamepad.o
all:
	make -C $(KDIR) M=$(shell pwd)
clean:
	make -C $(KDIR) M=$(shell pwd) clean
