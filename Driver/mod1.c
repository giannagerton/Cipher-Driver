#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <linux/moduleparam.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/string.h>

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

char* encrypt(const char* message, char* key){
	int i, x, count = 0;
	int len = strlen(message);
	char* cipher_text = (char*) kmalloc(sizeof(char)*len, GFP_KERNEL);
	char* newKey = (char*) kmalloc(sizeof(char)*len, GFP_KERNEL);
	for(i = 0; ; i++){
		if(strlen(key) == i)
			i = 0;
		if(count == len)
			break;
		newKey[count] = key[i];
		count++;
	}
	for(i = 0; i < len; i++){
		x = (message[i] + newKey[i]) % 26;
		x += 'A';
		cipher_text[i] = x;
	}
	return cipher_text;
}

char* decrypt(char* cipher_text, char* key){
	int i, x, len = strlen(cipher_text), count = 0;
	char* str = (char*) kmalloc(sizeof(char)*strlen(cipher_text), GFP_KERNEL);
	char* newKey = (char*) kmalloc(sizeof(char)*len, GFP_KERNEL);
	for(i = 0; ; i++){
		if(strlen(key) == i)
			i = 0;
		if(count == len)
			break;
		newKey[count] = key[i];
		count++;
	} 
	printk("cipher_text: %s\n", cipher_text);
	for(i = 0; i < len; i++){
		x = (cipher_text[i] - newKey[i] + 26) % 26;
		x += 'A';
		str[i] = x;
	}
	return str;
}

int device_open(struct inode *inode, struct file *filp){

	if(down_interruptible(&virtual_device.sem) != 0){
		printk(KERN_ALERT "device was unable to lock");
		return -1;
	}
	printk(KERN_INFO "device was opened yerr");
	return 0;
}

ssize_t device_read(struct file* filp, char* bufStoreData,size_t bufCount,loff_t* curOffset){
	char orig_message[1000];
	printk("My debugger is Printk\n");
	printk(KERN_INFO "This driver's reading from the device now.");
	strcpy(orig_message, decrypt(virtual_device.data, "LEMON"));
	//orig_message = decrypt(virtual_device.data, "LEMONLEMONLE");
//	printk("orig message %s\n", orig);	
	//(destination, source, sizetoTransfer
	ret = copy_to_user(bufStoreData, orig_message, bufCount);
	return ret;
}

	//when sending data to the device
ssize_t device_write(struct file* filp, const char* bufSourceData, size_t bufCount, loff_t* curOffset){
//	char cipher_text[1000];
//	const char* cipher_text = (char*)kmalloc(sizeof(char)*strlen(bufSourceData), GFP_KERNEL);
//	strcpy(cipher_text, encrypt(bufSourceData, "LEMON"));
	//const void* cipher_text = (const char*)encrypt(bufSourceData, "LEMON");
	printk("bufsourcedata: %s\n", bufSourceData);
	ret = copy_from_user(virtual_device.data, bufSourceData, bufCount);
	printk("WRITE %s\n", virtual_device.data);
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

