obj-m += mod1.o

KDIR = /usr/src/linux-headers-$(shell uname -r)

all:
	make -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf *.o *.ko *.mod.* *.symvers *.order
