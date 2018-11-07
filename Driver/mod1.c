#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <linux/moduleparam.h>
#include <linux/uaccess.h>

struct fake_device {
	char data[100];
	struct semaphore sem;
} virtual_device;

struct cdev *mcdev;
int major_number;
int ret;

dev_t dev_num;

#define DEVICE_NAME "TestingDriver"

//here we write the functions

int device_open(struct inode *inode, struct file *filp){

	if(down_interruptible(&virtual_device.sem) != 0){
		printk(KERN_ALERT "device was unable to lock");
		return -1;
	}
	printk(KERN_INFO "device was opened yerr");
	return 0;
}

ssize_t device_read(struct file* filp, char* bufStoreData,size_t bufCount,loff_t* curOffset){
	//when user wants to use the device. file ,where, how much, file offset.
	printk(KERN_INFO "This driver's reading from the device now.");
	
	//(destination, source, sizetoTransfer
	ret = copy_to_user(bufStoreData,virtual_device.data,bufCount);
	return ret;
}

	//when sending data to the device
ssize_t device_write(struct file* filp, const char* bufSourceData, size_t bufCount, loff_t* curOffset){
	printk(KERN_INFO "driver is writing to device");
	//(destination, source, count)
	ret = copy_from_user(virtual_device.data, bufSourceData,bufCount);
	return ret;

}

int device_close(struct inode *inode, struct file *filp){
	up(&virtual_device.sem);
	printk(KERN_INFO " Donion rings.");
	return 0;
}

struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_close,
	.write = device_write,
	.read = device_read
};



static int driver_entry(void){
	ret = alloc_chrdev_region(&dev_num,0,1,DEVICE_NAME);
	if(ret<0){
		printk(KERN_ALERT "yerrr failed to get a major number! \n");
		return ret;
	}
	major_number = MAJOR(dev_num);
	printk(KERN_INFO " yerrr we got a major number: %d ", major_number);

	printk(KERN_ALERT "\t use \"mknod /dev/%s c %d 0\" for device file", DEVICE_NAME,major_number);
	// why is this not working????	
	mcdev = cdev_alloc();
	mcdev->ops = &fops;
	mcdev->owner = THIS_MODULE;
	
	sema_init(&virtual_device.sem,1);
	ret = cdev_add(mcdev,dev_num,1);
	if(ret<0){
		printk(KERN_ALERT "PD unable to add cdev to kernel");
		return ret;
	}
	
	return 0;
}

static void driver_exit(void){
	cdev_del(mcdev);

	unregister_chrdev_region(dev_num,1);
	printk(KERN_ALERT "yerrr pd just unloaded");
}


module_init(driver_entry);
module_exit(driver_exit);

