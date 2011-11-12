#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <asm/unistd.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include "dummy_dev.h"

#define IN_BUF_LEN	20
#define OUT_BUF_LEN	50

static lang_t langtype;
static char *inbuffer = NULL;
static char *outbuffer = NULL;

/*
 * the open routine of 'dummy_dev'
 */
static int dummy_open(struct inode *inode, struct file *file)
{
//	dump_stack();
	return 0;
}

/*
 * the release routine of 'dummy_dev'
 */
static int dummy_release(struct inode *inode, struct file *file)
{
	return 0;
}

/**
 * the write routine of 'dummy_dev'
 */
static ssize_t dummy_write(struct file *filp, const char *user_buf, size_t len, loff_t *off)
{
	if (down_interruptible(&my_devp->sem))
		return -ERESTARTSYS;

	if (len > IN_BUF_LEN)
		len = IN_BUF_LEN;

	if(copy_from_user(inbuffer, user_buf, len)) {
		up(&my_devp->sem);
		return -EFAULT;
	}
	
	up(&my_devp->sem);

	printk(KERN_INFO"%s:inbuffer=%s\n", __func__, inbuffer);

	return 0;
}

/*
 * the read routine of 'dummy_dev'
 */
static ssize_t dummy_read(struct file *filp, char *user_buf, size_t len, loff_t *off)
{
	int length = 0;

	if (down_interruptible(&my_devp->sem))
		return -ERESTARTSYS;
	
	switch (langtype) {
	case english:
		length = sprintf(outbuffer, "english: %s.", inbuffer);
		break;
	case chinese:
		length = sprintf(outbuffer, "chinese: %s.", inbuffer);
		break;
	case pinyin:
		length = sprintf(outbuffer, "pinyin: %s.", inbuffer);
		break;
	default:
		break;
	}

	if(copy_to_user(user_buf, outbuffer, length)) {
		up(&my_devp->sem);
		return -EFAULT;
	}

	up(&my_devp->sem);

	printk(KERN_INFO"%s:outbuffer=%s\n", __func__, outbuffer);
	
	return 0;
}

/*
 * the ioctl routine of 'dummy_dev'
 */
static int dummy_ioctl(struct inode *inode, struct file *filep,
            unsigned int cmd, unsigned long arg)
{
	int err = 0;

	switch (cmd) {
	case DUMMY_IOCTL_RESETLANG:
		printk(KERN_INFO"DUMMY_IOCTL_RESETLANG\n");
		langtype = english;
	case DUMMY_IOCTL_GETLANG:
		printk(KERN_INFO"DUMMY_IOCTL_GETLANG\n");
		err = copy_to_user((int *)arg, &langtype, sizeof(int));
		break;
	case DUMMY_IOCTL_SETLANG:
		printk(KERN_INFO"DUMMY_IOCTL_SETLANG\n");
		err = copy_from_user(&langtype, (int *)arg, sizeof(int));
		break;
	default:
		printk(KERN_INFO"No such cmd.\n");
		err = ENOTSUPP;
		break;
	}
	printk(KERN_INFO"dummy_ioctl\n");

	return err;
}

/*
 * file_operations of 'dummy_dev'
 */
static struct file_operations dummy_dev_ops = {
	.owner = THIS_MODULE,
	.open = dummy_open,
	.read = dummy_read,
	.write = dummy_write,
	.ioctl = dummy_ioctl,
	.release = dummy_release,
};

static int __init my_init(void)
{
	int ret;
	dev_t devno;

	printk("my_init\n");
	/* register the 'dummy_dev' char device */
	if (mydev_major) {
		devno = MKDEV(mydev_major, mydev_minor);
		ret = register_chrdev_region(devno, 1, "mydev");
	} else {
		ret = alloc_chrdev_region(&devno, 0, 1, "mydev");
		mydev_major = MAJOR(devno);
	}
	if (ret < 0)
		goto fail_register_chrdev;

	my_devp = kmalloc(sizeof(struct my_dev), GFP_KERNEL);
	if (!my_devp) {
		ret = -ENOMEM;
		goto fail_malloc;
	}

	my_devp->cdev = cdev_alloc();
	cdev_init(my_devp->cdev, &dummy_dev_ops);
	my_devp->cdev->owner = THIS_MODULE;
	ret = cdev_add(my_devp->cdev, devno, 1);
	if (ret != 0)
		goto fail_cdev_add;

	my_devp->class = class_create(THIS_MODULE, "my_class");
	if (IS_ERR(my_devp->class)) {
		ret = PTR_ERR(my_devp->class);
		goto fail_create_class;
	}

	my_devp->dev = device_create(my_devp->class, NULL,
			devno, NULL, "my_dev");
	if (IS_ERR(my_devp->dev)) {
		ret = PTR_ERR(my_devp->dev);
		goto fail_create_device;
	}

	langtype = english;
	inbuffer = (char *)kmalloc(IN_BUF_LEN, GFP_KERNEL);
	if (!inbuffer) {
		ret = -ENOMEM;
		goto fail_malloc_inbuffer;
	}
	outbuffer = (char *)kmalloc(OUT_BUF_LEN, GFP_KERNEL);
	if (!outbuffer) {
		ret = -ENOMEM;
		goto fail_malloc_outbuffer;
	}

	init_MUTEX(&my_devp->sem);

	return 0;

fail_malloc_outbuffer:
	kfree(inbuffer);
fail_malloc_inbuffer:
	device_destroy(my_devp->class, devno);
fail_create_device:
	class_destroy(my_devp->class);
fail_create_class:
	cdev_del(my_devp->cdev);
fail_cdev_add:
	kfree(my_devp);
fail_malloc:
	unregister_chrdev_region(devno, 1);
fail_register_chrdev:
	return ret;
}

static void __exit my_exit(void)
{
	dev_t devno = MKDEV(mydev_major, mydev_minor);

	printk("my_exit\n");

	kfree(outbuffer);
	kfree(inbuffer);	
	device_destroy(my_devp->class, devno);
	class_destroy(my_devp->class);
	cdev_del(my_devp->cdev);
	kfree(my_devp);
	unregister_chrdev_region(devno, 1);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chen Wenxin <chenwenxin2004@163.com>");
MODULE_DESCRIPTION(THIS_DESCRIPTION);
