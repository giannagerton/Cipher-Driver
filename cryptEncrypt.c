#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <asm/uaccess.h>

static int TEXT_LENGTH = 50;

static char text[TEXT_LENGTH];

int device_ioctl(struct inode* inode, struct file* file, unsigned int ioctl_num, unsigned long ioctl_param) {
	int i;
	char* temp;
	char ch;
	
	switch (ioctl_num) {
	case IOCTL_CREATE_FILES: 
		while (*temp != '\0') {
			temp++;
			i++;
		}
		cryptEncrypt_write(file, (char*)ioctl_param, i, 0);
		break;
	}
	return 0;
}

static ssize_t device_write(struct file* filep, const char* buffer, size_t count, loff_t* offset) {
	int i;

	for (i = 0; i < count; i++) {
		get_user(text[i], buffer + i);
	}
	printk(KERN_INFO "write\n");
	return i;
}

static ssize_t device_read(struct file* filep, const char* buffer, size_t count, loff_t* offset) {
	int i;
	
	for (i = 0; i < count; i++) {
		put_user(text[i], buf+i);
	}
	printk(KERN_INFO "read\n");
	return i;
}

struct file_operations Fops = {
	.read = device_read,
	.write = device_write,
	.ioctl = device_ioctl,
	.open = device_open,
	.close = device_close
};

