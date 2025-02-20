## Description:
- This driver exposes an IOCTL API to user space so you can setup memory mapped device registers from your high-level application.
Generally, device drivers are set with standardized way such as devicetree for arm architecture which relies on "of_device_id" and platform_driver" structures. So this driver is made mainly for fun or in case you need to setup your hardware from application layer.
- The file "api.h" defines different data structures to be exchanged between kernel and user space as well as the different properties to be defined for a memory mapped device registers:
    - Write operations:
        - IOCTL_SET_BASE_ADDRESS: Set device base address.
        - IOCTL_SET_REGISTER_NAME: For each register you can save it in the driver session using a specific name so you can access it latelly using its name so you avoid providing its coordinates each time. You can achieve this by defining its proporties in "register_metadata" structure and provide it to driver.
        - IOCTL_SET_REGISTER_DATA: This querry is useful for writing data to a previously saved register. You can make use of "register_data" structure to send data to driver.
        - IOCTL_SET_BIT_DATA: This is useful for bitwise operations so you don't need to reset the whole register value. Use "bit_data" structure for this.
    - Read operations:
        - IOCTL_GET_BASE_ADDRESS: Get device base address.
        - IOCTL_GET_REGISTERS_MAP: Read the saved registers name mapping table.
        - IOCTL_GET_REGISTERS_DATA: Read registers data.

