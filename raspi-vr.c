#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define __u8 uint8_t
#define __u16 uint16_t
#define __u32 uint32_t
#define __u64 uint64_t

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#include <stdint.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <linux/input.h>
#include <linux/videodev2.h>
#include "yuyv_to_rgb.h"

#define CAMERA_DEV   "/dev/video0"
#define CAMERA_WIDTH  640
#define CAMERA_HEIGHT 480
#define MMAP_COUNT    2
#define PICTURE_NUM   30

//pre procedure difinition

//structure difinition

//global variables


static int xioctl(int fd, int request, void *arg)
{
  for (; ; ) {
    int ret = ioctl(fd, request, arg);
    if (ret < 0) {
      if (errno == EINTR)
	continue;
      return -errno;
    }
    break;
  }

  return 0;
}

int main(int argc, char ** argv) {
	int bln = 1;
	int fd;
	int ret;
	int width;
	int height;
	int length;
	int count;
	struct v4l2_format fmt;
	struct v4l2_requestbuffers req;
	uint8_t *yuyvbuf;

	//parse_args(argc, argv);

	while(bln);

	//fd = open(CAMERA_DEV, O_RDWR, 0);
	if (fd < 0) {
		//perror("open");
		//return -1;
	}
	memset(&fmt, 0, sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = CAMERA_WIDTH;
	fmt.fmt.pix.height = CAMERA_HEIGHT;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
	ret = xioctl(fd, VIDIOC_S_FMT, &fmt);
	if (ret < 0 || fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_YUYV || fmt.fmt.pix.width <= 0 || fmt.fmt.pix.height <= 0) {
		//perror("ioctl(VIDIOC_S_FMT)");
		return -1;
	}
	width = fmt.fmt.pix.width;
	height = fmt.fmt.pix.height;
	length = width * height;

	memset(&req, 0, sizeof(req));
	req.count = MMAP_COUNT;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	ret = xioctl(fd, VIDIOC_REQBUFS, &req);
	if (ret < 0) {
		perror("ioctl(VIDIOC_REQBUFS)");
		return -1;
	}
	count = req.count;

	close(fd);
	return 0;
}

//void parse_args(int argc, char ** argv) {
//	int i;
//	// Parse command line
//	for (i = 1; i < argc; ++i) {
//		if ((0 == strcmp(argv[i], "-h")) || (0 == strcmp(argv[i], "-?"))
//				|| (0 == strcmp(argv[i], "--help"))) {
//			showhelp();
//			exit(0);
//		} else if (0 == strncmp(argv[i], "-f", 2)) {
//			fifoname = argv[i] + 2;
//		} else {
//			fprintf( stderr, "Invalid argument: \'%s\'\n", argv[i]);
//			exit(1);
//		}
//	}
//}
//
//void showhelp(void) {
//	fprintf( stdout, "raspi-vr");
//	fflush(stdout);
//	return;
//}

