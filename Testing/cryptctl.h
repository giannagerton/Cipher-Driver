#ifndef CRYPTCTL_H
#define CRYPTCTL_H

#include <linux/ioctl.h>

#define MAGIC_NUM 'a'
#define MESSAGE_SIZE 1000
#define KEY_SIZE 100
#define ENCRYPT "cryptEncrypt"
#define DECRYPT "cryptDecrypt"
#define DEVICE_NAME "cryptctl"

struct idKey {
	int id;
	char key[KEY_SIZE];
};

struct idBuff {
	int id;
	char buff[MESSAGE_SIZE];
};

#define IOCTL_CREATE _IOR(MAGIC_NUM, 0, int)
#define IOCTL_DESTROY _IOW(MAGIC_NUM, 1, int)  

#endif
