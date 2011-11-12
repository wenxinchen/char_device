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

#define THIS_DESCRIPTION "This is a simple char device"

int mydev_major = 0;
int mydev_minor =0;

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
static ssize_t dummy_write(struct file *filp, const char *bp, size_t count, loff_t *ppos)
{
	return 0;
}

/*
 * the read routine of 'dummy_dev'
 */
static ssize_t dummy_read(struct file *filp, char *bp, size_t count, loff_t *ppos)
{
	return 0;
}

/*
 * the ioctl routine of 'dummy_dev'
 */
static int dummy_ioctl(struct inode *inode, struct file *filep,
            unsigned int cmd, unsigned long arg)
{
	return 0;
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

/*
 * struct cdev of 'dummy_dev'
 */
struct my_dev {
	struct cdev *cdev;
	struct class *class;
	struct device *dev;
};
struct my_dev *my_devp;

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
	
	return 0;

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
