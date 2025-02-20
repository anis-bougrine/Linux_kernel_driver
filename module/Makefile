LDD_NAME = kernel_space_peripheral_setup
obj-m += $(LDD_NAME).o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	strip --strip-debug $(LDD_NAME).ko

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
