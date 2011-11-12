#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include "../drv/dummy_dev.h"

int main(int argc, char **argv)
{
	int fd;
	lang_t langtype = english;

	if (argc == 1) {
		printf("Usage: %s langtype(english/chinese/pinyin)\n", argv[0]);
		return -1;
	}

	if (!strncmp(argv[1], "english", 7))
		langtype = english;
	else if (!strncmp(argv[1], "chinese", 7))
		langtype = chinese;
	else if (!strncmp(argv[1], "pinyin", 6))
		langtype = pinyin;
	else {
		printf("error langtype!\n");
		return -1;
	}

	fd = open("/dev/my_dev", O_RDWR);
	if (fd < 0) {
		printf("open failed.\n");
		return -1;
	}

	ioctl(fd, DUMMY_IOCTL_SETLANG, &langtype);
	ioctl(fd, DUMMY_IOCTL_GETLANG, &langtype);
	printf("langtype=%s\n",
		(langtype==pinyin)?"pinpin":
		((langtype==english)?"english":
		((langtype==chinese)?"chinese":"errtype")));

	printf("Reset:\n");
	ioctl(fd, DUMMY_IOCTL_RESETLANG, &langtype);
	ioctl(fd, DUMMY_IOCTL_GETLANG, &langtype);
	printf("langtype=%s\n",
		(langtype==pinyin)?"pinpin":
		((langtype==english)?"english":
		((langtype==chinese)?"chinese":"errtype")));

	close(fd);

	return 0;
}
