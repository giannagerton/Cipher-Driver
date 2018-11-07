#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "cryptctl.h"

static int major_number;

static int file_num = 0;

static int __init device_init(void) {
	
}

static int __exit device_exit(void) {

}

int device_ioctl(struct inode* inode, struct file* file, unsigned int ioctl_num, unsigned long ioctl_param) {
	int i, retval = 0;
	char* temp;
	switch (ioctl_num) {
		case IOCTL_CREATE:
			temp = (char*)ioctl_param; // key
			// read the key
			// create the files with the key
			if (request_module("cryptEncrypt") != 0) {
			}
			
			// done creating files
			retval = file_num;
			file_num++;
			break;
		case IOCTL_DESTROY:
			int id;
			id  = (int)ioctl_param; // encrypt descrypt id
			// delete the files
			ret_val = 0;
			break; 
	}
	return retval;
}

int device_open(struct inode* inode, struct file* filp)) {
	return 0;
}

int device_release(struct inode* inode, struct file* filp) {
	return 0;
}

struct file_operations Fops = {
	.ioctl = device_ioctl,
	.open = device_open,
	.release = device_release
};

module_init(device_init);
module_exit(device_exit);
			
