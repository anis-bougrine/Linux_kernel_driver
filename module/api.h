#include <linux/ioctl.h>

#define MY_IOCTL_MAGIC 'k'
#define MAX_REGISTERS 32

// Data structures
typedef struct
{
    char name[4];
    int address;
} register_metadata;

typedef struct
{
    char name[4];
    int data;
} register_data;

typedef struct
{
    char register_name[4];
    char bit_offset;
    char bit_value;
} bit_data;

// IOCTL commands
#define IOCTL_SET_BASE_ADDRESS _IOW(MY_IOCTL_MAGIC, 1, int)
#define IOCTL_SET_REGISTER_NAME _IOW(MY_IOCTL_MAGIC, 2, register_metadata)
#define IOCTL_SET_REGISTER_DATA _IOW(MY_IOCTL_MAGIC, 3, register_data)
#define IOCTL_SET_BIT_DATA _IOW(MY_IOCTL_MAGIC, 4, bit_data)

#define IOCTL_GET_BASE_ADDRESS _IOR(MY_IOCTL_MAGIC, 5, int)
#define IOCTL_GET_REGISTERS_MAP _IOR(MY_IOCTL_MAGIC, 6, register_metadata[MAX_REGISTERS])
#define IOCTL_GET_REGISTERS_DATA _IOR(MY_IOCTL_MAGIC, 7, register_data[MAX_REGISTERS])
