#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

int main(int argc, char **argv)
{
	int fd;
	int ret;
	char *file_name = "/dev/helloTest0";

	fd = open(file_name, O_RDWR|O_NOCTTY|O_NDELAY);
	if(fd < 0)
	{
		printf("can't open file %s\n", file_name);
		return -1;
	}

	close(fd);

	return 0;
}

