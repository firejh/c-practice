#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>

#define	u8	unsigned long long

#define TIME_DIFF_USEC(a,b) ({  \
    u8 a_usec = ((u8)a.tv_sec * 1000000 + a.tv_usec);   \
    u8 b_usec = ((u8)b.tv_sec * 1000000 + b.tv_usec);   \
    if (b_usec < a_usec) {    \
        u8 tmp = a_usec;    \
        a_usec = b_usec;    \
        b_usec = tmp;   \
    }   \
    (u8)(b_usec - a_usec);  \
})

#define	SIZE_K(x)	(x) * 1024
#define	SIZE_M(x)	(x) * 1024 * 1024
#define	MAX_SIZE	SIZE_M(400)

int main(int argc, char** argv)
{
	int fd;
	int idx;
	int max;
	int ret;
	u8	diff;
	char* file_name;
	char buf[SIZE_K(128)];
	struct timeval start;
	struct timeval end;

	file_name = "./tmp";
	fd = open(file_name, O_RDWR | O_CREAT | O_NONBLOCK,
				S_IRWXU | S_IRWXG | S_IROTH | S_IWOTH);
	if (fd < 0)	{
		fprintf(stderr, "open(file_name = %s) = %d\n", file_name, fd);
		return -1;
	}

	gettimeofday(&start, NULL);
	max = MAX_SIZE / sizeof(buf);
	for (idx = 0; idx < max; idx++)	{
		ret = (int)write(fd, buf, sizeof(buf));
		if (ret != sizeof(buf)) {
			printf("idx = %d, write = %d\n", idx, ret);
			break;
		}
	}

	gettimeofday(&end, NULL);

	close(fd);

	diff = TIME_DIFF_USEC(start, end);
	printf("idx = %d, max = %d, diff{sec = %llu, msec = %llu}\n",
			idx, max, diff / 1000000ULL, diff % 1000000ULL / 1000ULL);

	return 0;
}

