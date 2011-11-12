#ifndef __DUMMY_DEV_H__
#define __DUMMY_DEV_H__

#define THIS_DESCRIPTION "This is a simple char device"

int mydev_major = 0;
int mydev_minor =0;

/*
 * struct cdev of 'dummy_dev'
 */
struct my_dev {
	struct cdev *cdev;
	struct class *class;
	struct device *dev;
};
struct my_dev *my_devp;

#endif
