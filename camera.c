#include "common.h"

void *camera(void *arg)
{
	pthread_detach(pthread_self());

	int fd,cam_fd,jpg_fd;
	int ret,jpg_size;
	int x,y,i=0;
	char pic_name[10];
	char *jpg_mem;
	
	fd = open("/dev/fb0", O_RDWR);
	if(fd < 0)
	{
		printf(" open lcd  Failed !\n");
		exit(0);
	}
	
	fb_mem = (unsigned int*)mmap(NULL, 800*480*4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	cam_fd = open("/dev/video7",O_RDWR);
	if(cam_fd < 0)
	{
		printf(" open  camera Failed !\n");
		exit(0);		
	}

	
	struct v4l2_format fmt;

	struct v4l2_capability cap;
	if (-1 == ioctl (cam_fd, VIDIOC_QUERYCAP, &cap))
	{
		printf("  camera  VIDIOC_QUERYCAP  Failed !\n");
		exit(0);		
	}
	
	printf("  camera  VIDIOC_QUERYCAP version : %x \n",cap.version);

	int index;
	index = 0;
	if (-1 == ioctl (cam_fd, VIDIOC_S_INPUT, &index))
	{
		printf("  camera  VIDIOC_S_INPUT  Failed !\n");
		exit(0);
	}
	
	memset(&fmt,0,sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (-1 == ioctl (cam_fd, VIDIOC_G_FMT, &fmt))
	{
		printf("  camera  VIDIOC_G_FMT  Failed !\n");
		exit(0);
	}
	
	printf("  camera  VIDIOC_G_FMT width : %d \n",fmt.fmt.pix.width);
	printf("  camera  VIDIOC_G_FMT height : %d \n",fmt.fmt.pix.height );

	if( V4L2_PIX_FMT_JPEG == fmt.fmt.pix.pixelformat )
	printf("  camera  VIDIOC_G_FMT pixelformat : V4L2_PIX_FMT_JPEG \n");

	memset(&fmt,0,sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = 640;
	fmt.fmt.pix.height = 480;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;
	
	if (-1 == ioctl (cam_fd, VIDIOC_S_FMT, &fmt))
	{
		printf("  camera  VIDIOC_S_FMT  Failed !\n");
		exit(0);
	}

	memset(&fmt,0,sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == ioctl (cam_fd, VIDIOC_G_FMT, &fmt))
	{
		printf("  final camera  VIDIOC_G_FMT  Failed !\n");
		exit(0);
	}
	
	printf("  final camera  VIDIOC_G_FMT width : %d \n",fmt.fmt.pix.width);
	printf("  final camera  VIDIOC_G_FMT height : %d \n",fmt.fmt.pix.height );

	if( V4L2_PIX_FMT_JPEG == fmt.fmt.pix.pixelformat )
	printf("  final camera  VIDIOC_G_FMT pixelformat : V4L2_PIX_FMT_JPEG \n");

	struct v4l2_requestbuffers reqbuf;

	memset (&reqbuf, 0, sizeof (reqbuf));

	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	reqbuf.count = 4;

	if (-1 == ioctl (cam_fd, VIDIOC_REQBUFS, &reqbuf))
	{
		printf ("Video capturing or mmap-streaming is not supported\n");
		exit(0);
	}

	buffers = calloc (reqbuf.count, sizeof (*buffers));
	struct v4l2_buffer buffer;

	for (i = 0; i < reqbuf.count; i++)
	{
		memset (&buffer, 0, sizeof (buffer));
		buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buffer.memory = V4L2_MEMORY_MMAP;
		buffer.index = i;
		
		if (-1 == ioctl (cam_fd, VIDIOC_QUERYBUF, &buffer))
		{
			printf ("Video VIDIOC_QUERYBUF failed !\n");
			exit(0);
		}

		buffers[i].length = buffer.length;
		buffers[i].start = mmap (NULL, buffer.length,
							PROT_READ | PROT_WRITE,
							MAP_SHARED,
							cam_fd, buffer.m.offset);

		if (buffers[i].start == MAP_FAILED)
		{
			printf ("Video mmap failed !\n");
			exit(0);
		}

		ret = ioctl(cam_fd , VIDIOC_QBUF, &buffer);
		printf("VIDIOC_QBUF  buffer.index (%d) size  (%d)\n", buffer.index, buffer.length);
		if (ret < 0)
		{
			printf("VIDIOC_QBUF (%d) failed (%d)\n", i, ret);
			 exit(0);
		}
	}

	struct v4l2_buffer v4lbuf;
	memset(&v4lbuf,0,sizeof(v4lbuf));
	v4lbuf.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4lbuf.memory=V4L2_MEMORY_MMAP;

	enum v4l2_buf_type vtype= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == ioctl (cam_fd, VIDIOC_STREAMON, &vtype))
	{
		printf ("Video VIDIOC_STREAMON failed !\n");
		exit(0);
	}

	char jpg_name[20] = {0};
	int jpeg_fd;

	while(1)
	{
		if(action == 10)
			break;

		if(action2 == 2)
		{
			sleep(5);
			action2 = 0;
		}
	
		for(i=0;i<4;i++)
		{
			v4lbuf.index = i;
			ret = ioctl(cam_fd , VIDIOC_DQBUF, &v4lbuf);
			if (ret < 0)
			{
				printf("VIDIOC_DQBUF (%d) failed (%d)\n", i, ret);
				 exit(0);
			}
			
			Showjpeg(buffers[v4lbuf.index].start,buffers[v4lbuf.index].length,fb_mem);			
			ret = ioctl(cam_fd , VIDIOC_QBUF, &v4lbuf);
			if (ret < 0)
			{
				printf("VIDIOC_QBUF (%d) failed (%d)\n", i, ret);
				exit(0);
			}

			if(action2 == 1)
			{
				sprintf(jpg_name, "%d.jpg", jpg_num);
				jpeg_fd = open(jpg_name, O_RDWR|O_CREAT, 0666);
				ret = write(jpeg_fd, buffers[v4lbuf.index].start,
					    buffers[v4lbuf.index].length);

				if(ret != buffers[v4lbuf.index].length)
				{
					perror("failed");
					exit(0);
				}

				close(jpeg_fd);
				
				action2 = 0;
			}
		}
	}
	
	close(jpg_fd);

	vtype = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == ioctl (cam_fd, VIDIOC_STREAMOFF, &vtype))
	{
		printf ("Video VIDIOC_STREAMOFF failed !\n");
		exit(0);
	}

	for (i = 0; i < reqbuf.count; i++)
	munmap (buffers[i].start, buffers[i].length);

	ret = munmap(fb_mem, 800*480*4);
	if(ret == -1)
	{
		printf(" munmap  Failed !\n");
		exit(0);
	}

	ret = close(fd);
	if(ret == -1)
	{
		printf(" close Failed !\n");
		exit(0);
	}

	ret = close(cam_fd);
	if(ret == -1)
	{
		printf(" close  camera Failed !\n");
		exit(0);
	}

}


