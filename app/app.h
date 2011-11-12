#ifndef __DUMMY_DEV_H__
#define __DUMMY_DEV_H__

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

#endif
