#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

MODULE_LICENSE("GPL");

#define MYDEV_NAME "my_driver"
static dev_t dev;
static struct cdev my_cdev;
static struct class *my_class;

static int my_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "my_driver: open()\n");
    return 0;
}

static int my_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "my_driver: release()\n");
    return 0;
}

static ssize_t my_read(struct file *file, char __user *buf, size_t lbuf, loff_t *ppos)
{
    printk(KERN_INFO "my_driver: read()\n");
    return 0;
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t lbuf, loff_t *ppos)
{
    printk(KERN_INFO "my_driver: write()\n");
    return lbuf;
}

static struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .read = my_read,
    .write = my_write,
};

static int __init my_driver_init(void)
{
    int err;

    /* Allocate a major number dynamically */
    err = alloc_chrdev_region(&dev, 0, 1, MYDEV_NAME);
    if (err < 0) {
        printk(KERN_ERR "Failed to allocate chrdev region\n");
        return err;
    }

    /* Initialize the cdev structure and add it to the kernel */
    cdev_init(&my_cdev, &my_fops);
    my_cdev.owner = THIS_MODULE;

    err = cdev_add(&my_cdev, dev, 1);
    if (err < 0) {
        printk(KERN_ERR "Failed to add cdev\n");
        unregister_chrdev_region(dev, 1);
        return err;
    }

    /* Create a class for the device */
    my_class = class_create(THIS_MODULE, MYDEV_NAME);
    if (IS_ERR(my_class)) {
        printk(KERN_ERR "Failed to create class\n");
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev, 1);
        return PTR_ERR(my_class);
    }

    /* Create the device and register it with sysfs */
    device_create(my_class, NULL, dev, NULL, MYDEV_NAME);

    printk(KERN_INFO "my_driver: module loaded\n");

    return 0;
}

static void __exit my_driver_exit(void)
{
    /* Destroy the device and unregister it from sysfs */
    device_destroy(my_class, dev);

    /* Destroy the class */
    class_destroy(my_class);

    /* Remove the cdev from the kernel */
    cdev_del(&my_cdev);

    /* Free the major number */
    unregister_chrdev_region(dev, 1);

    printk(KERN_INFO "my_driver: module unloaded\n");
}

module_init(my_driver_init);
module_exit(my_driver_exit);
