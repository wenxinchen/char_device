#ifndef __DUMMY_DEV_H__
#define __DUMMY_DEV_H__

#define THIS_DESCRIPTION "This is a simple char device"

int mydev_major = 0;
int mydev_minor =0;

/* Use 'k' as magic number */
#define DUMMY_MAGIC		'k'
#define DUMMY_IOCTL_RESETLANG	_IO(DUMMY_MAGIC,0)
#define DUMMY_IOCTL_GETLANG	_IOR(DUMMY_MAGIC,1,int)
#define DUMMY_IOCTL_SETLANG	_IOW(DUMMY_MAGIC,2,int)

typedef enum _lang_t
{
	english,
	chinese,
	pinyin
}lang_t;

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
