#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#include <sys/time.h>
#include <sys/stat.h>
#include <linux/input.h>

#define SEND_EVENT(_type, _code, _value) do{\
	struct input_event eve;\
	eve.type = _type;\
	eve.code = _code;\
	eve.value = _value;\
	int n = write(fd, &eve, sizeof(struct input_event));\
	if (n < 0) {\
		perror("write");\
		exit(-1);\
	}\
}while(0)

//pre procedure difinition
void showhelp(void);
void parse_args(int argc, char ** argv);

//structure difinition
typedef struct _Frame {
	struct timeval time;
	unsigned char *frame_buffer;
	int width;
	int stride;
	int height;
	int frame_num;
	int flag;
} Frame;

//global variables
static int running = 1;
static const char *fifoname = NULL;
static int frame_w = 640;
static int frame_h = 480;
static int screen_w = 1920;
static int screen_h = 1080;
static int last_screen_x = 0;
static int last_screen_y = 0;
static int last_x = 0;
static int last_y = 0;
static int last_c = 0;
static int repeat_num = 0;
static int now_on = 0;

static uint32_t frames = 0;
static uint32_t skip_frames = 0;
static uint32_t frame_size;
static struct timeval last_detected;
static struct timezone tzone;

void image_process(unsigned char *imageData, int width, int widthStep,
		int height, int nChannels, struct timeval now, int frame) {

}

int main(int argc, char ** argv) {
	parse_args(argc, argv);

	return 0;
}

void parse_args(int argc, char ** argv) {
	int i;
	// Parse command line
	for (i = 1; i < argc; ++i) {
		if ((0 == strcmp(argv[i], "-h")) || (0 == strcmp(argv[i], "-?"))
				|| (0 == strcmp(argv[i], "--help"))) {
			showhelp();
			exit(0);
		} else if (0 == strncmp(argv[i], "-f", 2)) {
			fifoname = argv[i] + 2;
		} else {
			fprintf( stderr, "Invalid argument: \'%s\'\n", argv[i]);
			exit(1);
		}
	}
}

void showhelp(void) {
	fprintf( stdout, "raspi-vr");
	return;
}

