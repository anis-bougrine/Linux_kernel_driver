#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/mutex.h>
#include "api.h"

// Global variables
#define DEVICE_NAME "XXX_peripheral_setup"
#define CLASS_NAME "XXX_peripheral_class"

static int major_number;
static struct class *peripheral_class = NULL;
static struct device *peripheral_device = NULL;

static int base_address = 0;
static void __iomem *mapped_base = NULL; // Pointer to mapped physical memory
static register_metadata registers_map[MAX_REGISTERS];

// Mutex to protect global variables
static DEFINE_MUTEX(peripheral_mutex);

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
        // Map the physical memory to kernel virtual address space
        mutex_lock(&peripheral_mutex);
        mapped_base = ioremap(base_address, MAX_REGISTERS * sizeof(int));
        if (!mapped_base)
        {
            printk(KERN_ERR "Failed to map physical memory\n");
            mutex_unlock(&peripheral_mutex);
            return -ENOMEM;
        }
        printk(KERN_INFO "Base address set to 0x%x and mapped\n", base_address);
        mutex_unlock(&peripheral_mutex);
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
        mutex_lock(&peripheral_mutex);
        for (i = 0; i < MAX_REGISTERS; i++)
        {
            if (registers_map[i].name[0] == '\0')
            {
                memcpy(registers_map[i].name, rmeta->name, 4);
                registers_map[i].address = rmeta->address;
                break;
            }
        }
        mutex_unlock(&peripheral_mutex);
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
        mutex_lock(&peripheral_mutex);
        for (i = 0; i < MAX_REGISTERS; i++)
        {
            if (memcmp(registers_map[i].name, rdata->name, 4) == 0)
            {
                // Write data directly to physical memory
                iowrite32(rdata->data, mapped_base + registers_map[i].address);
                break;
            }
        }
        mutex_unlock(&peripheral_mutex);
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
        mutex_lock(&peripheral_mutex);
        for (i = 0; i < MAX_REGISTERS; i++)
        {
            if (memcmp(registers_map[i].name, bdata->register_name, 4) == 0)
            {
                // Read-modify-write operation for bit manipulation
                u32 reg_value = ioread32(mapped_base + registers_map[i].address);
                if (bdata->bit_value)
                {
                    reg_value |= (1 << bdata->bit_offset);
                }
                else
                {
                    reg_value &= ~(1 << bdata->bit_offset);
                }
                iowrite32(reg_value, mapped_base + registers_map[i].address);
                break;
            }
        }
        mutex_unlock(&peripheral_mutex);
        kfree(bdata);
        break;

    case IOCTL_GET_BASE_ADDRESS:
        addr = kmalloc(sizeof(int), GFP_KERNEL);
        if (!addr)
        {
            return -ENOMEM;
        }
        mutex_lock(&peripheral_mutex);
        *addr = base_address;
        mutex_unlock(&peripheral_mutex);
        if (copy_to_user((int *)arg, addr, sizeof(int)))
        {
            kfree(addr);
            return -EFAULT;
        }
        kfree(addr);
        break;

    case IOCTL_GET_REGISTERS_MAP:
        mutex_lock(&peripheral_mutex);
        if (copy_to_user((register_metadata *)arg, registers_map, sizeof(register_metadata) * MAX_REGISTERS))
        {
            mutex_unlock(&peripheral_mutex);
            return -EFAULT;
        }
        mutex_unlock(&peripheral_mutex);
        break;

    case IOCTL_GET_REGISTERS_DATA:
    {
        register_data reg_data[MAX_REGISTERS];

        mutex_lock(&peripheral_mutex);
        // Read data directly from physical memory
        for (i = 0; i < MAX_REGISTERS; i++)
        {
            if (registers_map[i].name[0] != '\0')
            {                                                                        // Check if the register is valid
                memcpy(reg_data[i].name, registers_map[i].name, 4);                  // Copy the name
                reg_data[i].data = ioread32(mapped_base + registers_map[i].address); // Read the data
            }
        }
        mutex_unlock(&peripheral_mutex);

        // Copy the data back to user space
        if (copy_to_user((register_data *)arg, reg_data, sizeof(register_data) * MAX_REGISTERS))
        {
            return -EFAULT;
        }
        break;
    }

    default:
        return -EINVAL;
    }
    return 0;
}

// Module initialization
static int __init peripheral_init(void)
{
    printk(KERN_INFO "Peripheral setup driver loaded\n");

    // Initialize the registers_map array
    memset(registers_map, 0, sizeof(registers_map));

    // Register character device
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0)
    {
        printk(KERN_ERR "Failed to register character device\n");
        return major_number;
    }

    // Create device class
    peripheral_class = class_create(CLASS_NAME);
    if (IS_ERR(peripheral_class))
    {
        printk(KERN_ERR "Failed to create device class\n");
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(peripheral_class);
    }

    // Create device file
    peripheral_device = device_create(peripheral_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(peripheral_device))
    {
        printk(KERN_ERR "Failed to create device file\n");
        class_destroy(peripheral_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(peripheral_device);
    }

    printk(KERN_INFO "Device created: /dev/%s\n", DEVICE_NAME);
    return 0;
}

// Module cleanup
static void __exit peripheral_exit(void)
{
    if (mapped_base)
    {
        iounmap(mapped_base); // Unmap the physical memory
    }
    device_destroy(peripheral_class, MKDEV(major_number, 0));
    class_destroy(peripheral_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "Peripheral setup driver unloaded\n");
}

module_init(peripheral_init);
module_exit(peripheral_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anis Bougrine");
MODULE_DESCRIPTION("General peripheral registers manipulation");
