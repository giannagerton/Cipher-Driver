
#ifndef CRYPTCTL_H
#define CRYPTCTL_H

#include <linux/ioctl.h>

#define CRYPT_IO_MAGIC 100

#define IOCTL_CREATE _IOWR(CRYPT_IO_MAGIC, 1, int) // not sure this is right
#define IOCTL_DESTROY _IOR(CRYPT_IO_MAGIC, 2, int)

#endif


