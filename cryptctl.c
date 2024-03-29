#include <linux/version.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mutex.h>

#include <linux/uaccess.h>

#include "cryptctl.h"

#define CRYPTCTL_BLOCK_SIZE 512
#define CRYPTCTL_BUFFER_SIZE 4000

MODULE_LICENSE("GPL");

struct cryptctl_dev {
	char message[MESSAGE_SIZE];
	char key[KEY_SIZE];
	unsigned long buffer_size; 
	unsigned long block_size;  
	struct cdev cdev;
};


/* parameters */
static int cryptctl_ndevices = 101;
static unsigned int cryptctl_major = 0;
static struct cryptctl_dev *cryptctl_devices = NULL;
static struct class *cryptctl_class = NULL;
static int id;

int device_open(struct inode* inode, struct file* filp);
int device_close(struct inode* inode, struct file* filp);
long device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param);
static int create_crypt_device(int cryptbool, int pair_minor);
static void cryptctl_destroy_device(struct cryptctl_dev *dev, int filenum);
static void cryptctl_cleanup_module(int filenum);


ssize_t device_read(struct file*, char* bufStoreData,size_t,loff_t*);

ssize_t device_write(struct file*, const char* , size_t, loff_t*);


struct file_operations cryptctl_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = device_ioctl,
	.open = device_open, 
	.release = device_close,
	.read = device_read,
	.write = device_write

};

void encrypt(const char* message, char* key, int idd){
        int i, x, count = 0;
        int len = strlen(message);
        //char* cipher_text = (char*) kmalloc(sizeof(char)*len, GFP_KERNEL);
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
               (&cryptctl_devices[idd])->message[i] = x;
        }
	printk(KERN_ALERT "%s", (&cryptctl_devices[idd])->message);
	kfree(newKey);
}

void decrypt(char* cipher_text, char* key, int idd){
        int i, x, len = strlen(cipher_text), count = 0;
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
               (&cryptctl_devices[idd])->message[i] = x;
        }
	kfree(newKey);
    	printk(KERN_ALERT "%s", (&cryptctl_devices[idd])->message);
}



int device_open(struct inode* inode, struct file* filp) {
	printk(KERN_INFO "device was opened");
	return 0;
}

int device_close(struct inode* inode, struct file* filp) {
	printk(KERN_INFO "closed device");
	return 0;
}

ssize_t device_read(struct file* filp, char* bufStoreData,size_t bufCount,loff_t* curOffset){
        int ret;
	//when user wants to use the device. file ,where, how much, file offset.
        printk(KERN_INFO "This driver's reading from the device now.");
	
        //(destination, source, sizetoTransfer
	ret = copy_to_user(bufStoreData, (&cryptctl_devices[1])->key,bufCount);
	printk(KERN_ALERT "The value is %s", (&cryptctl_devices[1])->key);
        return ret;
}

        //when sending data to the device
ssize_t device_write(struct file* filp, const char* bufSourceData, size_t bufCount, loff_t* curOffset){
        int ret;
	 printk(KERN_INFO "driver is writing to device");
        //(destination, source, count)
        ret = copy_from_user((&cryptctl_devices[1])->message, bufSourceData,bufCount);
        return ret;

}


int
create_device_pair(void)
{
	int filenum, retval, cryptbool;
	retval = 0;
	filenum = (id * 2) + 1;
	cryptbool = 0;
	if ((retval = create_crypt_device(cryptbool, filenum)) != 0) {
		goto err;
	}
	
	filenum++;
	cryptbool = 1;
	if ((retval = create_crypt_device(cryptbool, filenum)) != 0) {
		goto err;
	}
	return retval;
err:
	cryptctl_cleanup_module(filenum);
	return retval;

}

int 
create_crypt_device(int cryptbool, int filenum) {
	int err = 0;
	struct cryptctl_dev* dev;
	dev_t devno;
	struct device *device;
	dev = &cryptctl_devices[filenum];
	devno = MKDEV(cryptctl_major, filenum); 
	device = NULL;
	
	BUG_ON(dev == NULL || cryptctl_class == NULL);

	/* Memory is to be allocated when the device is opened the first time */
	dev->buffer_size = CRYPTCTL_BUFFER_SIZE;
	dev->block_size = CRYPTCTL_BLOCK_SIZE;
	
	cdev_init(&dev->cdev, &cryptctl_fops);
	dev->cdev.owner = THIS_MODULE;
	
	err = cdev_add(&dev->cdev, devno, 1);
	if (err)
	{
		printk(KERN_WARNING "[target] Error %d while trying to add %s%d",
			err, "crypt", filenum);
		return err;
	}
	
	// create one of the three device types
	if (cryptbool == 0) {
		device = device_create(cryptctl_class, NULL, devno, NULL, ENCRYPT "%d", id);
	}

	else if (cryptbool == 1) {
		device = device_create(cryptctl_class, NULL, devno, NULL, DECRYPT "%d", id);
	}

	else {
		device = device_create(cryptctl_class, NULL, devno, NULL, "cryptctl");
	}

	if (IS_ERR(device)) {
		err = PTR_ERR(device);
		printk(KERN_WARNING "[target] Error %d while trying to create %s%d",
			err, "crypt", filenum);
		cdev_del(&dev->cdev);
		return err;
	}
	return 0;
}


long device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param) {

	struct cryptctl_dev *dev, *dev2;
	long retval;
	struct idBuff *buffStruct;
	struct idKey *keyStruct;
	int num, i;
	retval = 0;
	switch (ioctl_num) {
		case IOCTL_CREATE:
			if (((2 * id) + 1) > cryptctl_ndevices) {
				printk(KERN_ALERT "Too many id pairs existing");
				return -1;
			} 
			if ((retval = create_device_pair()) != 0) {
				printk(KERN_ALERT "Error creating device pair");
				return retval;
			}
			retval = id;
			//TODO: set key for the device (gotten from ioctl_param)
						
			id++;
			break;

		case IOCTL_DESTROY:
			// TODO: check if value is acceptable (pair id exists) better
			
			if (ioctl_param > id) {
				printk(KERN_ALERT "Attempted to delete a pair which hasn't been created");
				return -1;
			}

			num = (2 * (int)ioctl_param) + 1;
			dev = &cryptctl_devices[num];
			cryptctl_destroy_device(dev, num);
			
			num++;
			dev = &cryptctl_devices[num];
			cryptctl_destroy_device(dev, num);
			break;

		case IOCTL_KEY:
			
			keyStruct = (struct idKey*)ioctl_param;
			num = keyStruct->id;
			printk(KERN_ALERT "%d",	num);
			num = (2 * num) + 1;
			printk(KERN_ALERT "the key is %s", keyStruct->key);
			dev = &cryptctl_devices[num];
			num++;
			dev2 = &cryptctl_devices[num];
			for (i = 0; i < KEY_SIZE; i++) {
				dev->key[i] = keyStruct->key[i];
				dev2->key[i] = keyStruct->key[i];
			}			
			retval = 0;
			printk(KERN_ALERT "Got here");
			printk(KERN_ALERT "Value is %s", (&cryptctl_devices[1])->key);
			break;
	case IOCTL_ENCRYPT:
			
			buffStruct = (struct idBuff*)ioctl_param;
			num = buffStruct->id;
			printk(KERN_ALERT "%d",	num);
			num = (2 * num) + 1;
			dev =  &cryptctl_devices[num];
			printk(KERN_ALERT "the key is %s", (&cryptctl_devices[num])->key);
			encrypt(buffStruct->buff,dev->key,num);
			for (i = 0; i < strlen(dev->message); i++) {
				buffStruct->buff[i] = dev->message[i];
			}
			retval = 0;
			break;
	case IOCTL_DECRYPT:
			
			buffStruct = (struct idBuff*)ioctl_param;
			num = buffStruct->id;
			printk(KERN_ALERT "%d",	num);
			num = (2 * num) + 1;
			dev =  &cryptctl_devices[num];
			num++;
			printk(KERN_ALERT "the key is %s", dev->key);
			decrypt(dev->message,dev->key,num);
			dev2 = &cryptctl_devices[num];
			for (i = 0; i < strlen(dev->message); i++) {
				buffStruct->buff[i] = dev2->message[i];
			}
			retval = 0;
			break;
	


			}
	return retval;
}

void cryptctl_destroy_device(struct cryptctl_dev *dev, int filenum)
{
	BUG_ON(dev == NULL || cryptctl_class == NULL);
	device_destroy(cryptctl_class, MKDEV(cryptctl_major, filenum));
	cdev_del(&dev->cdev);
	return;
}

static void
cryptctl_cleanup_module(int devices_to_destroy)
{
	int i;
	
	/* Get rid of character devices (if any exist) */
	if (cryptctl_devices) {
		for (i = 0; i < devices_to_destroy; ++i) {
			cryptctl_destroy_device(&cryptctl_devices[i], i);
		}
		kfree(cryptctl_devices);
	}
	
	if (cryptctl_class)
		class_destroy(cryptctl_class);

	/* [NB] cryptctl_cleanup_module is never called if alloc_chrdev_region()
	 * has failed. */
	unregister_chrdev_region(MKDEV(cryptctl_major, 0), cryptctl_ndevices);
	return;
}


static int __init
cryptctl_init_module(void)
{
	struct cryptctl_dev *cryptdev;
	int err = 0;
	//struct device *device = NULL;
	dev_t dev = 0;
	id = 0;
	cryptdev = NULL;
	
	if (cryptctl_ndevices <= 0)
	{
		printk(KERN_WARNING "[target] Invalid value of cryptctl_ndevices: %d\n", 
			cryptctl_ndevices);
		err = -EINVAL;
		return err;
	}
	
	/* Get a range of minor numbers (starting with 0) to work with */
	err = alloc_chrdev_region(&dev, 0, cryptctl_ndevices, DEVICE_NAME);
	if (err < 0) {
		printk(KERN_WARNING "[target] alloc_chrdev_region() failed\n");
		return err;
	}
	cryptctl_major = MAJOR(dev);

	/* Create device class (before allocation of the array of devices) */
	cryptctl_class = class_create(THIS_MODULE, DEVICE_NAME);
	if (IS_ERR(cryptctl_class)) {
		err = PTR_ERR(cryptctl_class);
		goto fail;
	}
	
	/* Allocate the array of devices */
	cryptctl_devices = (struct cryptctl_dev *)kzalloc(
		cryptctl_ndevices * sizeof(struct cryptctl_dev), 
		GFP_KERNEL);
	if (cryptctl_devices == NULL) {
		err = -ENOMEM;
		goto fail;
	}
	
	// create cryptctl file (cryptctl creation == 0, index 0)
	if ((err = create_crypt_device(2, 0)) != 0) {
		goto fail;
	}
	/* Construct devices */
	
	//BUG_ON(cryptdev == NULL || cryptctl_class == NULL);


	//cryptdev = &cryptctl_devices[0];


	/* Memory is to be allocated when the device is opened the first time */

	/*
	cryptdev->data = NULL;     
	cryptdev->buffer_size = CRYPTCTL_BUFFER_SIZE;
	cryptdev->block_size = CRYPTCTL_BLOCK_SIZE;
	
	cdev_init(&cryptdev->cdev, &cryptctl_fops);
	cryptdev->cdev.owner = THIS_MODULE;
	
	err = cdev_add(&cryptdev->cdev, dev, 1);
	if (err)
	{
		printk(KERN_WARNING "[target] Error %d while trying to add %s",
			err, DEVICE_NAME);
		goto fail;
	}

	device = device_create(cryptctl_class, NULL, dev, NULL, DEVICE_NAME "%d", 0);

	if (IS_ERR(device)) {
		err = PTR_ERR(device);
		printk(KERN_WARNING "[target] Error %d while trying to create %s%d",
			err, DEVICE_NAME, 0);
		cdev_del(&cryptdev->cdev);
		goto fail;
	}
	*/
	return 0; /* success */

fail:
	cryptctl_cleanup_module(1);
	return err;
}

static void __exit
cryptctl_exit_module(void)
{
	cryptctl_cleanup_module(cryptctl_ndevices);
	return;
}

module_init(cryptctl_init_module);
module_exit(cryptctl_exit_module);





