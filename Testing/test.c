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

#define CFAKE_BLOCK_SIZE 512

#define CFAKE_BUFFER_SIZE 4000

MODULE_AUTHOR("Eugene A. Shatokhin");
MODULE_LICENSE("GPL");

#define CFAKE_DEVICE_NAME "cfake"
struct cfake_dev {
	unsigned char *data;
	char key[100];
	unsigned long buffer_size; 
	unsigned long block_size;  
	struct mutex cfake_mutex; 
	struct cdev cdev;
};


/* parameters */
static int cfake_ndevices = 101;

static unsigned int cfake_major = 0;
static struct cfake_dev *cfake_devices = NULL;
static struct class *cfake_class = NULL;


struct file_operations cfake_fops = {
	.owner =    THIS_MODULE,
};

int device_ioctl(struct inode *inode, struct file *file, unsigned int ioctl_num, unsigned long ioctl_param) {
	
}

static int
cfake_construct_device(struct cfake_dev *dev, int minor, 
	struct class *class)
{
	int err = 0;
	dev_t devno = MKDEV(cfake_major, minor);
	struct device *device = NULL;
	
	BUG_ON(dev == NULL || class == NULL);

	/* Memory is to be allocated when the device is opened the first time */
	dev->data = NULL;     
	dev->buffer_size = 4000;
	dev->block_size = 512;
	mutex_init(&dev->cfake_mutex);
	
	cdev_init(&dev->cdev, &cfake_fops);
	dev->cdev.owner = THIS_MODULE;
	
	err = cdev_add(&dev->cdev, devno, 1);
	if (err)
	{
		printk(KERN_WARNING "[target] Error %d while trying to add %s%d",
			err, CFAKE_DEVICE_NAME, minor);
		return err;
	}

	device = device_create(class, NULL, /* no parent device */ 
		devno, NULL, /* no additional data */
		CFAKE_DEVICE_NAME "%d", minor);

	if (IS_ERR(device)) {
		err = PTR_ERR(device);
		printk(KERN_WARNING "[target] Error %d while trying to create %s%d",
			err, CFAKE_DEVICE_NAME, minor);
		cdev_del(&dev->cdev);
		return err;
	}
	return 0;
}


static void
cfake_destroy_device(struct cfake_dev *dev, int minor,
	struct class *class)
{
	BUG_ON(dev == NULL || class == NULL);
	device_destroy(class, MKDEV(cfake_major, minor));
	cdev_del(&dev->cdev);
	kfree(dev->data);
	mutex_destroy(&dev->cfake_mutex);
	return;
}

static void
cfake_cleanup_module(int devices_to_destroy)
{
	int i;
	
	/* Get rid of character devices (if any exist) */
	if (cfake_devices) {
		for (i = 0; i < devices_to_destroy; ++i) {
			cfake_destroy_device(&cfake_devices[i], i, cfake_class);
		}
		kfree(cfake_devices);
	}
	
	if (cfake_class)
		class_destroy(cfake_class);

	/* [NB] cfake_cleanup_module is never called if alloc_chrdev_region()
	 * has failed. */
	unregister_chrdev_region(MKDEV(cfake_major, 0), cfake_ndevices);
	return;
}


static int __init
cfake_init_module(void)
{
	int err = 0;
	int i = 0;
	int devices_to_destroy = 0;
	dev_t dev = 0;
	
	if (cfake_ndevices <= 0)
	{
		printk(KERN_WARNING "[target] Invalid value of cfake_ndevices: %d\n", 
			cfake_ndevices);
		err = -EINVAL;
		return err;
	}
	
	/* Get a range of minor numbers (starting with 0) to work with */
	err = alloc_chrdev_region(&dev, 0, cfake_ndevices, CFAKE_DEVICE_NAME);
	if (err < 0) {
		printk(KERN_WARNING "[target] alloc_chrdev_region() failed\n");
		return err;
	}
	cfake_major = MAJOR(dev);

	/* Create device class (before allocation of the array of devices) */
	cfake_class = class_create(THIS_MODULE, CFAKE_DEVICE_NAME);
	if (IS_ERR(cfake_class)) {
		err = PTR_ERR(cfake_class);
		goto fail;
	}
	
	/* Allocate the array of devices */
	cfake_devices = (struct cfake_dev *)kzalloc(
		cfake_ndevices * sizeof(struct cfake_dev), 
		GFP_KERNEL);
	if (cfake_devices == NULL) {
		err = -ENOMEM;
		goto fail;
	}
	
	/* Construct devices */
	for (i = 0; i < cfake_ndevices; ++i) {
		err = cfake_construct_device(&cfake_devices[i], i, cfake_class);
		if (err) {
			devices_to_destroy = i;
			goto fail;
		}
	}
	return 0; /* success */

fail:
	cfake_cleanup_module(devices_to_destroy);
	return err;
}

static void __exit
cfake_exit_module(void)
{
	cfake_cleanup_module(cfake_ndevices);
	return;
}

module_init(cfake_init_module);
module_exit(cfake_exit_module);





