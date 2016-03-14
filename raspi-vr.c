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
#define CAMERA_WIDTH  1280
#define CAMERA_HEIGHT 480
#define MMAP_COUNT    2
#define PICTURE_NUM   1

//pre procedure difinition

//structure difinition

//global variables


int xioctl(int fd, int request, void *arg)
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

struct v4l2_format fmt;
struct v4l2_requestbuffers req;
struct v4l2_buffer buf;
enum v4l2_buf_type type;
void *mmap_p[MMAP_COUNT];
__u32 mmap_l[MMAP_COUNT];
struct rusage usage;
int main(int argc, char ** argv) {
	  int fd, width, height, length, count, ret, i;
	  uint8_t *yuyvbuf, *rgbbuf, *pyuyv, *prgb;
	  double t;

	  fd = open(CAMERA_DEV, O_RDWR, 0);
	  if (fd < 0) {
	    perror("open");
	    return -1;
	  }

	  memset(&fmt, 0, sizeof(fmt));
	  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	  fmt.fmt.pix.width = CAMERA_WIDTH;
	  fmt.fmt.pix.height = CAMERA_HEIGHT;
	  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
	  ret = xioctl(fd, VIDIOC_S_FMT, &fmt);
	  if (ret < 0 || fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_RGB24 ||
	      fmt.fmt.pix.width <= 0 || fmt.fmt.pix.height <= 0) {
	    perror("ioctl(VIDIOC_S_FMT)");
	    return -1;
	  }
	  width = fmt.fmt.pix.width;
	  height = fmt.fmt.pix.height;
	  length = width * height;

	  yuyvbuf = malloc(3 * length * PICTURE_NUM);
	  if (!yuyvbuf) {
	    perror("malloc");
	    return -1;
	  }

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

	  for (i = 0; i < count; i++) {
	    memset(&buf, 0, sizeof(buf));
	    buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	    buf.memory = V4L2_MEMORY_MMAP;
	    buf.index  = i;
	    ret = xioctl(fd, VIDIOC_QUERYBUF, &buf);
	    if (ret < 0) {
	      perror("ioctl(VIDIOC_QUERYBUF)");
	      return -1;
	    }

	    mmap_p[i] = mmap(NULL, buf.length, PROT_READ, MAP_SHARED, fd, buf.m.offset);
	    if (mmap_p[i] == MAP_FAILED) {
	      perror("mmap");
	      return -1;
	    }
	    mmap_l[i] = buf.length;
	  }

	  for (i = 0; i < count; i++) {
	    memset(&buf, 0, sizeof(buf));
	    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	    buf.memory = V4L2_MEMORY_MMAP;
	    buf.index = i;
	    ret = xioctl(fd, VIDIOC_QBUF, &buf);
	    if (ret < 0) {
	      perror("ioctl(VIDIOC_QBUF)");
	      return -1;
	    }
	  }

	  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	  ret = xioctl(fd, VIDIOC_STREAMON, &type);
	  if (ret < 0) {
	    perror("ioctl(VIDIOC_STREAMON)");
	    return -1;
	  }

	  for (i = 0, pyuyv = yuyvbuf; i < PICTURE_NUM; i++) {
	    fd_set fds;

	    FD_ZERO(&fds);
	    FD_SET(fd, &fds);
	    for (; ; ) {
	      ret = select(fd + 1, &fds, NULL, NULL, NULL);
	      if (ret < 0) {
		if (errno == EINTR)
		  continue;
		perror("select");
		return -1;
	      }
	      break;
	    }

	    if (FD_ISSET(fd, &fds)) {
	      memset(&buf, 0, sizeof(buf));
	      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	      buf.memory = V4L2_MEMORY_MMAP;
	      ret = xioctl(fd, VIDIOC_DQBUF, &buf);
	      if (ret < 0 || buf.bytesused < (__u32)(3 * length)) {
		perror("ioctl(VIDOC_DQBUF)");
		return -1;
	      }

	      memcpy(pyuyv, mmap_p[buf.index], 3 * length);
	      pyuyv += 3 * length;

	      ret = xioctl(fd, VIDIOC_QBUF, &buf);
	      if (ret < 0) {
		perror("ioctl(VIDIOC_QBUF)");
		return -1;
	      }
	    }
	  }

	  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	  xioctl(fd, VIDIOC_STREAMOFF, &type);
	  for (i = 0; i < count; i++)
	    munmap(mmap_p[i], mmap_l[i]);
	  close(fd);

	  rgbbuf = malloc(3 * length * PICTURE_NUM);
	  if (!rgbbuf) {
	    perror("malloc");
	    return -1;
	  }

	  getrusage(RUSAGE_SELF, &usage);
	  t = ((double)usage.ru_utime.tv_sec * 1e+3 +
	       (double)usage.ru_utime.tv_usec * 1e-3);

	  for (i = 0, pyuyv = yuyvbuf, prgb = rgbbuf; i < PICTURE_NUM;
	       i++, pyuyv += 3 * length, prgb += 3 * length)
	  {
		  FILE *fp = NULL;
		  fp = fopen("/home/pi/git/raspi-vr/test_1.raw", "w");
		  if(fp != NULL)
		  {
			  fwrite(pyuyv, 3 * length, 1, fp);
			  fclose(fp);
		  }
	  }
	    //yuyv_to_rgb(pyuyv, prgb, length);

	  getrusage(RUSAGE_SELF, &usage);
	  t = ((double)usage.ru_utime.tv_sec * 1e+3 +
	       (double)usage.ru_utime.tv_usec * 1e-3) - t;
	  printf("convert time: %3.3lf msec/flame\n", t / (double)PICTURE_NUM);

	  free(yuyvbuf);

	  //ppm_writefile(rgbbuf, width, height, PICTURE_NUM);

	  free(rgbbuf);

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

