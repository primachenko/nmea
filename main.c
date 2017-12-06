#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <poll.h>
#include <fcntl.h>
#include <stdlib.h>

#define BUF_SIZE 512
#define MAX_NMEA_LEN 82

typedef struct
{
	int hh;
	int mm;
	int ss;
	int sss;
	char A;
	float lantitude;
	char P;
	float longtitude;
	char J;
	float vv;
	float bb;
	int DD;
	int MM;
	int YY;
	float xx;
	char n;
	char m;

}nmea_rmc_t;

int main(int argc, char const *argv[])
{
	if (argc != 2)
	{
		return -1;
	}

	int rc;
	int fd;
	int lfd;
	char buf[BUF_SIZE]; memset(buf, 0, BUF_SIZE);
	char * path;

	path = (char *) argv[1];
	fd = (int) open(path, O_RDWR);
	lfd = (int) open("/tmp/raw_data.gnss", O_RDWR | O_CREAT);

	struct termios options;
	tcgetattr(fd, &options);
	cfsetispeed(&options, B4800);
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	tcsetattr(fd, TCSANOW, &options);
	int i = 0;

	while (i < BUF_SIZE)
	{
		rc = read(fd, &buf[i], 1);
		if (buf[i] == '\r')
		{
			if (buf[1] != '$')
			{
				memset(buf, 0, BUF_SIZE);
				i = 0;
				continue;
			}
			// printf("%s", buf);
			write(lfd, buf, i);
			/*parse GSV остальное лень делать*/
			if (NULL == strtok(buf, "GSV"))
			{
				memset(buf, 0, BUF_SIZE);
				i = 0;
				continue;
			}
			nmea_rmc_t msg; memset(&msg, 0, sizeof(nmea_rmc_t));
			sscanf(buf, "$GPRMC,%2d%2d%2d.%3d,%c,%6f,%c,%6f,%c,%2f,%2f,%2d%2d%2d,%2f*",
				&msg.hh, &msg.mm, &msg.ss, &msg.sss, &msg.A, &msg.lantitude, &msg.P,
				&msg.longtitude, &msg.J, &msg.vv, &msg.bb, &msg.DD, &msg.MM,
				&msg.YY, &msg.xx);
			memset(buf, 0, BUF_SIZE);
			sprintf(buf, "$GPRMC,%2d%2d%2d.%3d,%c,%6f,%c,%6f,%c,%2f,%2f,%2d%2d%2d,%2f*\n",
				msg.hh, msg.mm, msg.ss, msg.sss, msg.A, msg.lantitude, msg.P,
				msg.longtitude, msg.J, msg.vv, msg.bb, msg.DD, msg.MM,
				msg.YY, msg.xx);
			i = 0;
			continue;
		}
		i++;
	}
	close(fd);
	close(lfd);

	return 0;
}
