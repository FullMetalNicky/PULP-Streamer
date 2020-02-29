/* I got help from ETH guys, but I did write most of it myself
 or at least, copy-pasted cleverly from different projects :) */


#include "rt/rt_api.h"
#include "rt/rt_time.h"
#include "rt/data/rt_data_camera.h"
// #include "rt/rt_himax.h"
#include "ImgIO.h"
#include <fcntl.h>

// This strange resolution comes from the himax camera
#define WIDTH     324
#define HEIGHT    244

#ifdef USE_RAW
#define FB_FORMAT RT_FB_FORMAT_RAW
#else
#define FB_FORMAT RT_FB_FORMAT_GRAY
#endif


static unsigned char*   L2_image;
static int        imgTransferDone = 0;
static rt_event_t *    event_capture;
static rt_camera_t *  camera;
static int frame_id = 0;

int pipe;

#define SQUARE 2

// #define CAM_FULLRES_W 162     // HiMax full width 324
// #define CAM_FULLRES_H 162     // HiMax full height 244
// #define CAM_CROP_W    324     // Cropped camera width
// #define CAM_CROP_H    180     // Cropped camera height
//
// #define LL_X      ((CAM_FULLRES_W-CAM_CROP_W)/2)  // left x coordinate 62
// #define LL_Y      ((CAM_FULLRES_H-CAM_CROP_H)/2)  // up y coordinate 22
// #define DSMPL_RATIO 3
// #define CAM_DSMPL_W   108               //it's  on you to calculate the final size after cropping and downsampling
// #define CAM_DSMPL_H   60                //it's  on you to calculate the final size after cropping and downsampling

unsigned int CAM_CROP_W;
unsigned int CAM_CROP_H;
unsigned int LL_X;
unsigned int LL_Y;
unsigned int DSMPL_RATIO;
unsigned int CAM_DSMPL_W;
unsigned int CAM_DSMPL_H;
unsigned int camera_id;
unsigned int format;
unsigned int CAM_FULLRES_W;
unsigned int CAM_FULLRES_H;
unsigned int target_level;




// static void readline(int file, unsigned char *buffer)
// {
// 	printf("readline %d\n", file);
// 	unsigned char *n = buffer;
// 	// rt_event_t *event = rt_event_get_blocking(NULL);
// 	while(1)
// 	{
// 		int res = rt_bridge_read(file, n, 50, NULL);
// 		// rt_bridge_read_wait(event);
// 		if(n == '\n') break;
// 		printf("%d: rc %x %c\n", res, *n, *n);
// 		n++;
// 	}
// 	printf("line %s", buffer);
// }

unsigned char *readint(unsigned char *s, int *value)
{
	unsigned char *e = s;
	while(1)
	{
		if(*e < '0' || *e > '0' + 10)
		{
			break;
		}
		e++;
	}
	unsigned char *next = e+1;
	e--;
	int power = 1;
	*value = 0;
	while(e >= s)
	{
		int digit = (int)(*e-- - '0');
		*value += power * digit;
		power *= 10;
	}
	// printf("Read int %d value -> %x\n", *value, next);
	return next;
}


static void read_config(char *path)
{
  int file = rt_bridge_open(path, 0, 0, NULL);
	if(file == 0){
		printf("Failed to open file, %s\n", path);
	}
	unsigned char line[100];
	unsigned char *s = line;

	int res = rt_bridge_read(file, s, 100, NULL);
	// printf("Read config -> %d\n%s\n", res, line);

	int top, right, bottom, left, step;

	s = readint(s, &top);
	s = readint(s, &right);
	s = readint(s, &bottom);
	s = readint(s, &left);
	s = readint(s, &step);
	s = readint(s, &camera_id);
	s = readint(s, &format);
  s = readint(s, &target_level);


	// readline(file, line);
	// *width = readint(line);
	// readline(file, line);
	// *height = readint(line);
	// readline(file, line);
	// *sampling = readint(line);

	switch(format){
		case QVGA:
			CAM_FULLRES_W = 324;
			CAM_FULLRES_H = 244;
			break;
		case QQVGA:
			CAM_FULLRES_W = 162;
			CAM_FULLRES_H = 122;
			break;
		case SQUARE:
			CAM_FULLRES_W = 162;
			CAM_FULLRES_H = 162;
			break;
	}


	DSMPL_RATIO = step;
	LL_X = left;
	LL_Y = top;

	CAM_CROP_W = CAM_FULLRES_W - left - right;
	CAM_CROP_H = CAM_FULLRES_H - bottom - top;
	CAM_DSMPL_W = CAM_CROP_W / step;
	CAM_DSMPL_H = CAM_CROP_H / step;

	printf("Will capture frames from camera %d (%d) with margins of %d %d %d %d pixels, sampling every %d pixels for a total size of %d x %d\n",
	camera_id, format, top, right, bottom, left, step, CAM_DSMPL_W, CAM_DSMPL_H);

  rt_bridge_close(file,NULL);
}


static void end_of_frame()
{
	// int a = rt_time_get_us();
  rt_cam_control(camera, CMD_PAUSE, NULL);

  //WriteImageToFifo("../../../image_pipe", WIDTH, HEIGHT, L2_image);
  unsigned char * origin    = (unsigned char *) L2_image;
  unsigned char * ptr_crop  = (unsigned char *) L2_image;
  int       init_offset = CAM_FULLRES_W * LL_Y + LL_X;
  int       outid     = 0;

	if(CAM_DSMPL_H != CAM_FULLRES_H || CAM_DSMPL_W != CAM_FULLRES_W)
	{
	  for(int i=0; i<CAM_CROP_H; i+= DSMPL_RATIO) {
	    rt_event_execute(NULL, 0);
	    unsigned char * line = ptr_crop + init_offset + CAM_FULLRES_W * i;
	    for(int j=0; j<CAM_CROP_W; j+= DSMPL_RATIO) {
	      origin[outid] = line[j];
	      outid++;
	    }
	  }
	}
  //
  // unsigned char reg = himaxRegRead(camera, ANALOG_GAIN);
  // printf("ANALOG_GAIN 0x%x\n", reg);
  // reg = himaxRegRead(camera, DIGITAL_GAIN_H);
  // printf("DIGITAL_GAIN_H 0x%x\n", reg);
  // reg = himaxRegRead(camera, DIGITAL_GAIN_L);
  // printf("DIGITAL_GAIN_L 0x%x\n", reg);
  // reg = himaxRegRead(camera, INTEGRATION_H);
  // printf("INTEGRATION_H 0x%x\n", reg);
  // reg = himaxRegRead(camera, INTEGRATION_L);
  // printf("INTEGRATION_L 0x%x\n", reg);

	// printf("Process: %d\n", rt_time_get_us() - a);
	// a = rt_time_get_us();
  // WriteImageToFifo("/tmp/image_pipe", CAM_DSMPL_W, CAM_DSMPL_H, L2_image);
	PushImageToFifo(pipe, CAM_DSMPL_W, CAM_DSMPL_H, L2_image);
	// printf("Transmit: %d\n", rt_time_get_us() - a);

  imgTransferDone = 1;
}

static void enqueue_capture()
{

  rt_cam_control(camera, CMD_START, NULL);

  rt_camera_capture(camera, (unsigned char*)L2_image, WIDTH*HEIGHT*sizeof(unsigned char), rt_event_get(NULL, end_of_frame, NULL));
}


int main()
{
  printf("Entering main controller\n");


  // First wait until the external bridge is connected to the platform
  printf("Connecting to bridge\n");
  rt_bridge_connect(1, NULL);
  printf("Connection done\n");

	pipe = rt_bridge_open("/tmp/image_pipe", O_WRONLY, S_IRWXU, NULL);

	read_config("/tmp/config.txt");
	// printf("Config: width %d, height %d, sampling %d\n", width, height, sampling);


	// #define DSMPL_RATIO 3
	// #define CAM_DSMPL_W   108               //it's  on you to calculate the final size after cropping and downsampling
	// #define CAM_DSMPL_H   60                //it's  on you to calculate the final size after cropping and downsampling


  // Allocate 3 buffers, there will be 2 for the camera double-buffering and one
  // for flushing an image to the framebuffer.

  L2_image = rt_alloc(RT_ALLOC_L2_CL_DATA, WIDTH*HEIGHT*sizeof(unsigned char));

  printf("Finished allocation\n");

  // We'll need one event per buffer
  if (rt_event_alloc(NULL,1)) return -1;

  // Configure Himax camera on interface 0
  rt_cam_conf_t cam_conf;
  rt_camera_conf_init(&cam_conf);
  cam_conf.id = 0;
  // cam_conf.control_id = 0; //0 for crazy
	cam_conf.control_id = camera_id; //0 for crazy
  cam_conf.type = RT_CAM_TYPE_HIMAX;

	// this does NOT impact the format!!!
	if(format==QVGA) cam_conf.resolution = QVGA;
	else cam_conf.resolution = QQVGA;

  cam_conf.format = HIMAX_MONO_COLOR;
  cam_conf.fps = fps10;
  cam_conf.slice_en = DISABLE;
  cam_conf.shift = 0;
  cam_conf.frameDrop_en = DISABLE;
  cam_conf.frameDrop_value = 0;
  cam_conf.cpiCfg = UDMA_CHANNEL_CFG_SIZE_8;

  // rt_img_slice_t slicer;
  // slicer.slice_ll.x = 16;
  // slicer.slice_ll.y = 16;
  // slicer.slice_ur.x = 241;
  // slicer.slice_ur.y = 321;
  // _camera_extract(&cam_conf, &slicer);

  printf("camera configured\n");
  // Open the camera


  camera = rt_camera_open(NULL, &cam_conf, 0);
  if (camera == NULL) return -1;

  printf("camera opened\n");

  rt_cam_control(camera, CMD_INIT, 0);
  //rt_time_wait_us(1000000); //Wait camera calibration

  printf("start camera\n");

	if(format != QVGA)
	{
		himaxRegWrite(camera, READOUT_X, 0x03);
		himaxRegWrite(camera, READOUT_Y, 0x03);
		himaxRegWrite(camera, BINNING_MODE, 0x03);
	}
	if(format == SQUARE)
	{
		himaxRegWrite(camera, QVGA_WIN_EN, 0x00);
	}

  himaxRegWrite(camera, AE_CTRL, 0x01);

  // himaxRegWrite(camera, ANALOG_GAIN, 0x0);
  // himaxRegWrite(camera, INTEGRATION_L, 0xFF);
  // himaxRegWrite(camera, INTEGRATION_H, 0xFF);
  // himaxRegWrite(camera, DIGITAL_GAIN_H, 0xFF);
  // himaxRegWrite(camera, DIGITAL_GAIN_L, 0xFF);

  // himaxGrayScale(camera, 0x0);

// ANALOG_GAIN 0x0
// DIGITAL_GAIN_H 0x1
// DIGITAL_GAIN_L 0x0
// INTEGRATION_H 0x0
// INTEGRATION_L 0xaa



  // 0x01
  // himaxRegWrite(camera, ANALOG_GAIN, 0x80);
  // himaxRegWrite(camera, DIGITAL_GAIN_H, 0x00);
  // himaxRegWrite(camera, DIGITAL_GAIN_L, 0x80);


  //0x3C
  himaxRegWrite(camera, AE_TARGET_MEAN, (char)target_level); //0x70


  // himaxRegWrite(camera, READOUT_Y, 0xFF);
  // himaxRegWrite(camera, READOUT_X, 0xFF);
  // 0x0A
  // himaxRegWrite(camera, AE_MIN_MEAN, 0x50);

  // himaxRegWrite(camera, MIN_DGAIN, 0x70);
  // himaxRegWrite(camera, MAX_AGAIN_FULL, 0x03);
  // himaxRegWrite(camera, MIN_AGAIN, 0x03);





  // Start it
  rt_cam_control(camera, CMD_START, 0);
  if(rt_platform() == ARCHI_PLATFORM_BOARD)
    rt_time_wait_us(1000000);
  printf("camera started\n");


  enqueue_capture();

  // wait on input image transfer
  while(imgTransferDone==0)
  {
    rt_event_yield(NULL);
  }

  // The main loop is not doing anything, everything will be done through event callbacks
  while(1)
  {
    // wait on input image transfer
    while(imgTransferDone==0)
    {
      rt_event_yield(NULL);
    }
    imgTransferDone=0;

    ++frame_id;
    // printf("Transferred frame %d @ %d \n", frame_id, rt_time_get_us());


    event_capture = rt_event_get(NULL, enqueue_capture, NULL);
    rt_event_push(event_capture);
  }

  rt_camera_close(camera, 0);
  rt_free(RT_ALLOC_L2_CL_DATA, L2_image, WIDTH*HEIGHT*sizeof(unsigned char));

	rt_bridge_close(pipe, NULL);

  return 0;
}
