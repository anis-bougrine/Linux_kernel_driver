#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include "api.h"

// Global variables
static int base_address = 0;
static register_metadata registers_map[MAX_REGISTERS];
static register_data registers_data[MAX_REGISTERS];

// Function prototypes
static long peripheral_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static int __init peripheral_init(void);
static void __exit peripheral_exit(void);

// File operations structure
static struct file_operations fops = {
    .unlocked_ioctl = peripheral_ioctl,
};

// IOCTL handler
static long peripheral_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int i;
    register_metadata *rmeta;
    register_data *rdata;
    bit_data *bdata;
    int *addr;

    switch (cmd)
    {
    case IOCTL_SET_BASE_ADDRESS:
        if (copy_from_user(&base_address, (int *)arg, sizeof(int)))
        {
            return -EFAULT;
        }
        printk(KERN_INFO "Base address set to 0x%x\n", base_address);
        break;

    case IOCTL_SET_REGISTER_NAME:
        rmeta = kmalloc(sizeof(register_metadata), GFP_KERNEL);
        if (!rmeta)
        {
            return -ENOMEM;
        }
        if (copy_from_user(rmeta, (register_metadata *)arg, sizeof(register_metadata)))
        {
            kfree(rmeta);
            return -EFAULT;
        }
        for (i = 0; i < MAX_REGISTERS; i++)
        {
            if (registers_map[i].name[0] == '\0')
            {
                memcpy(registers_map[i].name, rmeta->name, 4);
                registers_map[i].address = rmeta->address;
                break;
            }
        }
        kfree(rmeta);
        break;

    case IOCTL_SET_REGISTER_DATA:
        rdata = kmalloc(sizeof(register_data), GFP_KERNEL);
        if (!rdata)
        {
            return -ENOMEM;
        }
        if (copy_from_user(rdata, (register_data *)arg, sizeof(register_data)))
        {
            kfree(rdata);
            return -EFAULT;
        }
        for (i = 0; i < MAX_REGISTERS; i++)
        {
            if (memcmp(registers_map[i].name, rdata->name, 4) == 0)
            {
                registers_data[i].data = rdata->data;
                break;
            }
        }
        kfree(rdata);
        break;

    case IOCTL_SET_BIT_DATA:
        bdata = kmalloc(sizeof(bit_data), GFP_KERNEL);
        if (!bdata)
        {
            return -ENOMEM;
        }
        if (copy_from_user(bdata, (bit_data *)arg, sizeof(bit_data)))
        {
            kfree(bdata);
            return -EFAULT;
        }
        for (i = 0; i < MAX_REGISTERS; i++)
        {
            if (memcmp(registers_map[i].name, bdata->register_name, 4) == 0)
            {
                if (bdata->bit_value)
                {
                    registers_data[i].data |= (1 << bdata->bit_offset);
                }
                else
                {
                    registers_data[i].data &= ~(1 << bdata->bit_offset);
                }
                break;
            }
        }
        kfree(bdata);
        break;

    case IOCTL_GET_BASE_ADDRESS:
        addr = kmalloc(sizeof(int), GFP_KERNEL);
        if (!addr)
        {
            return -ENOMEM;
        }
        *addr = base_address;
        if (copy_to_user((int *)arg, addr, sizeof(int)))
        {
            kfree(addr);
            return -EFAULT;
        }
        kfree(addr);
        break;

    case IOCTL_GET_REGISTERS_MAP:
        if (copy_to_user((register_metadata *)arg, registers_map, sizeof(register_metadata) * MAX_REGISTERS))
        {
            return -EFAULT;
        }
        break;

    case IOCTL_GET_REGISTERS_DATA:
        if (copy_to_user((register_data *)arg, registers_data, sizeof(register_data) * MAX_REGISTERS))
        {
            return -EFAULT;
        }
        break;

    default:
        return -EINVAL;
    }
    return 0;
}

// Module initialization
static int __init peripheral_init(void)
{
    printk(KERN_INFO "Peripheral setup driver unloaded\n");
    register_chrdev(240, "peripheral_setup", &fops); // Register character device with major number 240
    return 0;
}

// Module cleanup
static void __exit peripheral_exit(void)
{
    printk(KERN_INFO "Peripheral setup driver unloaded\n");
    unregister_chrdev(240, "peripheral_setup");
}

module_init(peripheral_init);
module_exit(peripheral_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anis Bougrine");
MODULE_DESCRIPTION("General pripheral registers manipulation");
