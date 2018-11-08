#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <linux/moduleparam.h>
#include <linux/uaccess.h>

#include "cryptctl.h"

#define MAX_FILES 100
#define PAIR_SIZE 2

struct fake_device {
	char data[100];
	char* device_key;
	struct semaphore sem;
} virtual_device;

struct file_operations fops;

struct cdev* mcdev_arr[MAX_FILES];
int major_number;
int ret;
static struct class* c1;

int id;
dev_t dev_num;

int create_device_pair(void);
int create_crypt_device(char* text, int filenum);
int destroy_device_pair(int id);

#define DEVICE_NAME "cryptdriver"

int device_ioctl(struct inode* inode, struct file* file, unsigned int ioctl_num, unsigned long ioctl_param) {
	int id;
	char* key;
	switch (ioctl_num) {
		case IOCTL_CREATE:
			key = (char*)ioctl_param;
			// probably need something else to pass param
			if (create_device_pair() != 0) {
				return -1;
			}

			// write the key somewhere
			
			break;
		case IOCTL_DESTROY:
			id = (int)ioctl_param;
			destroy_device_pair(id);
			break;
	}
	return 0;
}

int create_device_pair(void) {
	int filenum;
	char* text;
	text = "";
	if ((MAX_FILES / PAIR_SIZE ) < id) {
		printk(KERN_ALERT "Too many files");
		return -1;
	}
	filenum = (PAIR_SIZE * id); 
	sprintf(text, "cryptEncrypt%d", id);
	create_crypt_device(text, filenum);

	filenum++;
	sprintf(text, "cryptDecrypt%d", id);
	create_crypt_device(text, filenum);
	return 0;
}

int create_crypt_device(char* text, int filenum) {
	dev_t curr_dev;
	cdev_init(mcdev_arr[filenum], &fops);
	curr_dev = MKDEV(MAJOR(dev_num), MINOR(dev_num) + filenum);
	cdev_add(mcdev_arr[filenum], curr_dev, 1);
	if (device_create(c1, NULL, curr_dev, NULL,  text) == NULL) {
		printk(KERN_ALERT "could not create device file");
		return -1;
	}
	return 0;
}

int destroy_device_pair(int id) {
	int filenum;
	filenum = id * PAIR_SIZE;
	cdev_del(mcdev_arr[filenum]);
	device_destroy(c1, MKDEV(MAJOR(dev_num), MINOR(dev_num) + filenum));
	
	filenum++;
	cdev_del(mcdev_arr[filenum]);
	device_destroy(c1, MKDEV(MAJOR(dev_num), MINOR(dev_num) + filenum));

	return 0;
}

int device_open(struct inode* inode, struct file* filp) {
	if (down_interruptible(&virtual_device.sem) != 0){ 
		printk(KERN_ALERT "device was unable to lock");
		return -1;
	}
	printk(KERN_INFO "device was opened yerr");
	return 0;
}

ssize_t device_read(struct file* filp, char* bufStoreData, size_t bufCount, loff_t* curOffset) {
	printk(KERN_INFO "This driver'sreading from the device now.");
	
	ret = copy_to_user(bufStoreData, virtual_device.data, bufCount);
	return ret;
}

ssize_t device_write(struct file* filp, const char* bufSourceData, size_t bufCount, loff_t* curOffset) {
	printk(KERN_INFO "driver is writing to device");
	ret = copy_from_user(virtual_device.data, bufSourceData, bufCount);
	return ret;
}

int device_close(struct inode* inode, struct file* filp) {
	up(&virtual_device.sem);
	printk(KERN_INFO "Donion rings.");
	return 0;
}

struct file_operations fops = {
	.owner = THIS_MODULE, 
	.open = device_open, 
	.release = device_close,
	.write = device_write, 
	.read = device_read
};

static int driver_entry(void) {
	ret = alloc_chrdev_region(&dev_num, 0, MAX_FILES, DEVICE_NAME);
	id = 0;
	if (ret<0) {
		printk(KERN_ALERT "failed to get major num\n");
		return ret;
	}
	major_number = MAJOR(dev_num);
	printk(KERN_INFO "we got major number: %d ", major_number);
	
	printk(KERN_ALERT "\t use \"mknod /dev/%s c %d 0\" for device file", DEVICE_NAME, major_number);
	
	mcdev_arr[0] = cdev_alloc();
	mcdev_arr[0]-> ops = &fops;
	mcdev_arr[0]->owner = THIS_MODULE;

	sema_init(&virtual_device.sem, 1);
	ret = cdev_add(mcdev_arr[0], dev_num, 1);
	
	c1 = class_create(THIS_MODULE, "cryptctl");
	
	if (ret < 0) {
		printk(KERN_ALERT "PD unable to add cdev to kernel");
		return ret;
	}
	return 0;
}

static void driver_exit(void) {

	cdev_del(mcdev_arr[0]);

	unregister_chrdev_region(dev_num, MAX_FILES);
	printk(KERN_ALERT "yerrr pd just unloaded");
}

module_init(driver_entry);
module_exit(driver_exit);

