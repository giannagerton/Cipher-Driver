#ifndef CRYPTCTL_H
#define CRYPTCTL_H

#include <linux/ioctl.h>

#define MAGIC_NUM 'a'

#define IOCTL_CREATE _IOR(MAGIC_NUM, 0, int)
#define IOCTL_DESTROY _IOW(MAGIC_NUM, 1, int)  

#endif
