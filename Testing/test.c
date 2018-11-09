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

#define ENCRYPT "cryptEncrypt"
#define DECRYPT "cryptDecrypt"

MODULE_LICENSE("GPL");

#define DEVICE_NAME "cryptctl"
struct cryptctl_dev {
	unsigned char *data;
	char key[100];
	unsigned long buffer_size; 
	unsigned long block_size;  
	struct mutex cryptctl_mutex; 
	struct cdev cdev;
};


/* parameters */
static int cryptctl_ndevices = 101;

static unsigned int cryptctl_major = 0;
static struct cryptctl_dev *cryptctl_devices = NULL;
static struct class *cryptctl_class = NULL;

static int id;

static int device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param);

struct file_operations cryptctl_fops = {
	.owner =    THIS_MODULE,
	.ioctl =    device_ioctl
};

static int create_crypt_device(int cryptbool, int pair_minor);
static void cryptctl_cleanup_model(int devices_to_destroy);

static int
create_device_pair(void)
{
	int pair_minor, retval, cryptbool;
	retval = 0;
	pair_minor = (id * 2) + 1;
	cryptbool = 0;
	if ((retval = create_crypt_device(cryptbool, pair_minor)) != 0) {
		goto err;
	}
	
	pair_minor++;
	cryptbool = 1;
	if ((retval = create_crypt_device(cryptbool, pair_minor)) != 0) {
		goto err;
	}
	return retval;
err:
	cryptctl_cleanup_model(pair_minor);
	return retval;

}

static int 
create_crypt_device(int cryptbool, int pair_minor) {
	int err = 0;
	struct cryptctl_dev* dev;
	dev_t devno;
	struct device *device;
	dev = &cryptctl_devices[pair_minor];
	devno = MKDEV(cryptctl_major, pair_minor); 
	device = NULL;
	
	BUG_ON(dev == NULL || cryptctl_class == NULL);

	/* Memory is to be allocated when the device is opened the first time */
	dev->data = NULL;     
	dev->buffer_size = CRYPTCTL_BUFFER_SIZE;
	dev->block_size = CRYPTCTL_BLOCK_SIZE;
	
	cdev_init(&dev->cdev, &cryptctl_fops);
	dev->cdev.owner = THIS_MODULE;
	
	err = cdev_add(&dev->cdev, devno, 1);
	if (err)
	{
		printk(KERN_WARNING "[target] Error %d while trying to add %s%d",
			err, "crypt", pair_minor);
		return err;
	}
	
	if (cryptbool == 0) {
		device = device_create(cryptctl_class, NULL, /* no parent device */ 
			devno, NULL, /* no additional data */
			ENCRYPT "%d", pair_minor);
	}
	else {
		device = device_create(cryptctl_class, NULL,
			devno, NULL,
			DECRYPT "%d", pair_minor);
	}

	if (IS_ERR(device)) {
		err = PTR_ERR(device);
		printk(KERN_WARNING "[target] Error %d while trying to create %s%d",
			err, "crypt", pair_minor);
		cdev_del(&dev->cdev);
		return err;
	}
	return 0;
}


int device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param) {

	int retval;
	retval = 0;
	switch (ioctl_num) {
		case IOCTL_CREATE:
			if (create_device_pair() != 0) {
				printk(KERN_ALERT "Error creating device pair");
				return retval;
			}
			retval = id;
			//TODO: set key for the device (gotten from ioctl_param)
			
			id++;
			break;

		case IOCTL_DESTROY:
			
			break;
	}
	return retval;
}

static void
cryptctl_destroy_device(struct cryptctl_dev *dev, int minor,
	struct class *class)
{
	BUG_ON(dev == NULL || cryptctl_class == NULL);
	device_destroy(class, MKDEV(cryptctl_major, minor));
	cdev_del(&dev->cdev);
	kfree(dev->data);
	mutex_destroy(&dev->cryptctl_mutex);
	return;
}

static void
cryptctl_cleanup_module(int devices_to_destroy)
{
	int i;
	
	/* Get rid of character devices (if any exist) */
	if (cryptctl_devices) {
		for (i = 0; i < devices_to_destroy; ++i) {
			cryptctl_destroy_device(&cryptctl_devices[i], i, cryptctl_class);
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
	struct device *device = NULL;
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
	
	/* Construct devices */
	
	BUG_ON(cryptdev == NULL || cryptctl_class == NULL);

	cryptdev = &cryptctl_devices[0];
	/* Memory is to be allocated when the device is opened the first time */
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

	device = device_create(cryptctl_class, NULL, /* no parent device */ 
		dev, NULL, /* no additional data */
		DEVICE_NAME "%d", 0);

	if (IS_ERR(device)) {
		err = PTR_ERR(device);
		printk(KERN_WARNING "[target] Error %d while trying to create %s%d",
			err, DEVICE_NAME, 0);
		cdev_del(&cryptdev->cdev);
		goto fail;
	}
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





