#include "common.h"

void init_tty(int fd)
{    
	struct termios termios_new;
	bzero( &termios_new, sizeof(termios_new));
	cfmakeraw(&termios_new);
	cfsetispeed(&termios_new, B9600);
	cfsetospeed(&termios_new, B9600);
	termios_new.c_cflag |= CLOCAL | CREAD;
	termios_new.c_cflag &= ~CSIZE;
	termios_new.c_cflag |= CS8; 
	termios_new.c_cflag &= ~PARENB;
	termios_new.c_cflag &= ~CSTOPB;
	tcflush(fd,TCIFLUSH);
	termios_new.c_cc[VTIME] = 10;
	termios_new.c_cc[VMIN] = 1;
	tcflush (fd, TCIFLUSH);
	
	if(tcsetattr(fd,TCSANOW,&termios_new) )
		printf("Setting the serial1 failed!\n");

}

unsigned char CalBCC(unsigned char *buf, int n)
{
	int i;
	unsigned char bcc=0;
	for(i = 0; i < n; i++)
	{
		bcc ^= *(buf+i);
	}
	return (~bcc);
}

int PiccRequest(int fd)
{
	unsigned char WBuf[128], RBuf[128];
	int  ret, i,len;
	fd_set rdfd;
	
	memset(WBuf, 0, 128);
	memset(RBuf,0,128);
	WBuf[0] = 0x07;
	WBuf[1] = 0x02;
	WBuf[2] = 0x41;
	WBuf[3] = 0x01;
	WBuf[4] = 0x52;
	WBuf[5] = CalBCC(WBuf, WBuf[0]-2);
	WBuf[6] = 0x03;

	FD_ZERO(&rdfd);
	FD_SET(fd,&rdfd);
	tcflush (fd, TCIFLUSH);
	write(fd, WBuf, 7);;
	ret = select(fd + 1,&rdfd, NULL,NULL,&timeout);
	switch(ret)
	{
		case -1:
			perror("select error\n");
			break;
		case  0:
			printf("Request timed out.\n");
			break;
		default:
			usleep(10000);
			ret = read(fd, RBuf, 8);
			len = ret;
			ret = read(fd, RBuf+len, 8);
			len +=ret;
			if (len < 0)
			{
				printf("len = %d, %m\n", len, errno);
				break;
			}
			if (RBuf[2] == 0x00)
			{
				return 0;
			}
			break;
	}
	return -1;
}

int PiccAnticoll(int fd)
{
	unsigned char WBuf[128], RBuf[128];
	int ret, i,len;
	fd_set rdfd;;
	memset(WBuf, 0, 128);
	memset(RBuf,0,128);
	WBuf[0] = 0x08;
	WBuf[1] = 0x02;
	WBuf[2] = 0x42;
	WBuf[3] = 0x02;
	WBuf[4] = 0x93;
	WBuf[5] = 0x00;
	WBuf[6] = CalBCC(WBuf, WBuf[0]-2);
	WBuf[7] = 0x03;
	tcflush (fd, TCIFLUSH);
	FD_ZERO(&rdfd);
	FD_SET(fd,&rdfd);
	write(fd, WBuf, 8);
	
	ret = select(fd + 1,&rdfd, NULL,NULL,&timeout);
	switch(ret)
	{
		case -1:
			perror("select error\n");
			break;
		case  0:
			perror("Timeout:");
			break;
		default:
			usleep(10000);
                        ret = read(fd, RBuf, 10);
                        len = ret;
                        ret = read(fd, RBuf+len, 10);
                        len +=ret;

			if (len < 0)
			{
				printf("len = %d, %m\n", len, errno);
				break;
			}
			if (RBuf[2] == 0x00)
			{
				cardid = (RBuf[4]<<24) | (RBuf[5]<<16) | (RBuf[6]<<8) | RBuf[7];
				return 0;
			}
	}
	return -1;
}


volatile unsigned int read_rfid(void)
{
	int ret, i;
	int fd;

	fd = open(DEV_PATH, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (fd < 0)
	{
		fprintf(stderr, "Open Gec210_ttySAC1 fail!\n");
		exit(0);
	}

	init_tty(fd);
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	int number = 10;

	printf("put on your idcard!\n十秒中后关闭读取！\n");
	while(1)
	{
		if(number == 0)
		{
			printf("请求超时，请重试!\n");
			printf("==============================================\n");
			break;
		}
		fprintf(stderr,"%d\t",number);

		if(!PiccRequest(fd) && !PiccAnticoll(fd))
		{
			printf("\n");
			break;
		}
		
		sleep(1);
		number--;
	}
	
	close(fd);
	return cardid;
}
