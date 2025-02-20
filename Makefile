obj-m += kernel_space_peripheral_setup.o

all:
    make -C $(PWD)/kernel_source/ M=$(PWD)/module/ modules

clean:
    make -C $(PWD)/kernel_source/ M=$(PWD)/module/ clean
