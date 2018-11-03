#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "cryptctl.h"

static int file_num = 0;

int device_ioctl(struct inode* inode, struct file* file, unsigned int ioctl_num, unsigned long ioctl_param) {
	int i, retval = 0;
	char* temp;
	switch (ioctl_num) {
		case IOCTL_CREATE:
			temp = (char*)ioctl_param; // key
			// read the key
			// create the files with the key
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
			


