//Headers
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/io.h>

//This function is triggered at module loading
int Custom_peripheral_driver_init(void)
{
    //Write your code here.
    return 0;
}

//This function is triggered at module unloading
void Custom_peripheral_driver_exit(void)
{
    //Clean the ressources you used before exiting (free allocated memory, switch off peripherals clocks...) 
}

MODULE_LICENSE("GPL");
module_init(Custom_peripheral_driver_init);
module_exit(Custom_peripheral_driver_exit);
