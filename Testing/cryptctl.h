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
#define IOCTL_KEY _IOW(MAGIC_NUM, 2, struct idKey)
#define IOCTL_ENCRYPT _IOWR(MAGIC_NUM, 3, struct idBuff)
#define IOCTL_DECRYPT _IOWR(MAGIC_NUM, 4, struct idBuff)

#endif
