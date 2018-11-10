#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <linux/moduleparam.h>
#include <linux/uaccess.h>
#include <linux/ctype.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/cred.h>

#include "cryptctl.h"

#define MAX_FILES 100
#define PAIR_SIZE 2

/*=====================================================================================================*/
	//PROTOTYPES

static int device_open(struct inode *, struct file*);
static int device_release(struct inode *, struct file*);
int encrypt_msg(char *msg);
int decrypt_msg(char *msg);
ssize_t cryptctl_ioctl(struct file *, unsigned int, unsigned long);
static ssize_t crypt_read(struct file *, char*, size_t, loff_t *);
static ssize_t crypt_write(struct file *, const char *, size_t, loff_t*);
int create_device_pair(void);


/*=====================================================================================================*/

static int major_number;
static struct cdev *mcdev;
static struct device *cryptctl_device = NULL;
static struct class *cryptctl_class = NULL;


int devCount = 0;	
int ret;
int pairsCreated = 0;
struct cdev* mcdev_arr[MAX_FILES];
dev_t dev_num;

struct fake_device{
	char data[1000];
	char key[100];
	struct semaphore sem;
} virtual_device;

#define DEVICE_NAME "cryptctl"
#define CLASS_NAME "cryptctl"

struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_release,
	.ioctl = cryptctl_ioctl,
	.write = crypt_write,
	.read = crypt_read
};


static int __init cryptctl_init(void){

	printk(KERN_INFO "Welcome!");
	if (alloc_chrdev_region(&dev_num, 0,MAX_FILES , DEVICE_NAME "_proc") < 0){  //$cat /proc/devices
		return -1;
	}
	devCount++;
	major_number = MAJOR(dev_num);
	printk(KERN_INFO "we got a major number: %d", major_number);
	
	if ((cryptctl_class = class_create(THIS_MODULE, CLASS_NAME "_sys")) == NULL){    //$ls /sys/class
		unregister_chrdev_region(dev_num, 1);
		return -1;
	}
	
	if (device_create(cryptctl_class, NULL, dev_num, DEVICE_NAME ) == NULL){ //$ls /dev/
		class_destroy(cryptctl_class);
		unregister_chrdev_region(dev_num, 1);
        	return -1;
	}
	
	cdev_init(&mcdev, &fops);
	
	if (cdev_add(&mcdev, dev_num, 1) == -1){
		device_destroy(cryptctl_class, dev_num);
		class_destroy(cryptctl_class);
		unregister_chrdev_region(dev_num, 1);
		return -1;
	}	
	return 0;

}

static void __exit cryptctl_exit(void){
	device_destory(cryptctl_class, dev_num);
	cdev_del(&mcdev);
	class_destory(cryptctl_class);
	unregister_chardev_region(dev_num, MAXFILES);
}

int encrypt_msg(char *msg){
	return -1;	
}

int decrypt_msg(char *msg){
	return -1;
}

static int device_open(struct inode *, struct file*){
	return -1;
}

static int device_release(struct inode *, struct file*){
	return -1;
}

ssize_t cryptctl_ioctl(struct file *, unsigned int, unsigned long){
	return -1;
}

static ssize_t crypt_read(struct file *, char*, size_t, loff_t *){
	return -1;
}

static ssize_t crypt_write(struct file *, const char *, size_t, loff_t*){
	return -1;
}

int create_device_pair(void){
	return -1;
}

module_init(cryptctl_init);
module_exit(cryptctl_exit);
