#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "api.h"

int main()
{
    int fd = open("/dev/XXX_peripheral_setup", O_RDWR);
    if (fd < 0)
    {
        perror("Failed to open device");
        return 1;
    }

    // Example: Set base address
    int base_addr = 0x1000;
    if (ioctl(fd, IOCTL_SET_BASE_ADDRESS, &base_addr) < 0)
    {
        perror("IOCTL_SET_BASE_ADDRESS failed");
        close(fd);
        return 2;
    }

    // Example: Set register name
    register_metadata rmeta = {.name = "REG1", .address = 0x1004};
    if (ioctl(fd, IOCTL_SET_REGISTER_NAME, &rmeta) < 0)
    {
        perror("IOCTL_SET_REGISTER_NAME failed");
        close(fd);
        return 3;
    }

    // Example: Set register data
    register_data rdata = {.name = "REG1", .data = 0x1234};
    if (ioctl(fd, IOCTL_SET_REGISTER_DATA, &rdata) < 0)
    {
        perror("IOCTL_SET_REGISTER_DATA failed");
        close(fd);
        return 4;
    }

    // Example: Get base address
    int retrieved_base_addr;
    if (ioctl(fd, IOCTL_GET_BASE_ADDRESS, &retrieved_base_addr) < 0)
    {
        perror("IOCTL_GET_BASE_ADDRESS failed");
        close(fd);
        return 5;
    }
    printf("Retrieved base address: 0x%x\n", retrieved_base_addr);

    close(fd);
    return 0;
}
