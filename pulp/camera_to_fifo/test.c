/* I got help from ETH guys, but I did write most of it myself
 or at least, copy-pasted cleverly from different projects :) */


#include "rt/rt_api.h"
#include "rt/rt_time.h"
#include "rt/data/rt_data_camera.h"
// #include "rt/rt_himax.h"
#include "ImgIO.h"


// This strange resolution comes from the himax camera
#define WIDTH     324
#define HEIGHT    244

#ifdef USE_RAW
#define FB_FORMAT RT_FB_FORMAT_RAW
#else
#define FB_FORMAT RT_FB_FORMAT_GRAY
#endif


static unsigned char* 	L2_image;
static int				imgTransferDone = 0;
static rt_event_t *		event_capture;
static rt_camera_t *	camera;
static int frame_id = 0;


#define CAM_FULLRES_W 324     // HiMax full width 324
#define CAM_FULLRES_H 312     // HiMax full height 244
#define CAM_CROP_W    324     // Cropped camera width
#define CAM_CROP_H    180     // Cropped camera height

#define LL_X      ((CAM_FULLRES_W-CAM_CROP_W)/2)  // left x coordinate 62
#define LL_Y      ((CAM_FULLRES_H-CAM_CROP_H)/2)  // up y coordinate 22
#define DSMPL_RATIO 3
#define CAM_DSMPL_W   108               //it's  on you to calculate the final size after cropping and downsampling
#define CAM_DSMPL_H   60                //it's  on you to calculate the final size after cropping and downsampling


static void end_of_frame()
{

	rt_cam_control(camera, CMD_PAUSE, NULL);

	//WriteImageToFifo("../../../image_pipe", WIDTH, HEIGHT, L2_image);
	unsigned char * origin    = (unsigned char *) L2_image;
  unsigned char * ptr_crop  = (unsigned char *) L2_image;
  int       init_offset = CAM_FULLRES_W * LL_Y + LL_X;
  int       outid     = 0;

  for(int i=0; i<CAM_CROP_H; i+= DSMPL_RATIO) {
    rt_event_execute(NULL, 0);
    unsigned char * line = ptr_crop + init_offset + CAM_FULLRES_W * i;
    for(int j=0; j<CAM_CROP_W; j+= DSMPL_RATIO) {
      origin[outid] = line[j];
      outid++;
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


	WriteImageToFifo("/tmp/image_pipe", CAM_DSMPL_W, CAM_DSMPL_H, L2_image);

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
  cam_conf.control_id = 0; //0 for crazy
  cam_conf.type = RT_CAM_TYPE_HIMAX;
  cam_conf.resolution = QVGA;
  cam_conf.format = HIMAX_MONO_COLOR;
  cam_conf.fps = fps10;
  cam_conf.slice_en = DISABLE;
  cam_conf.shift = 0;
  cam_conf.frameDrop_en = ENABLE;
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
	himaxRegWrite(camera, AE_TARGET_MEAN, 0x70);

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
		printf("Transferred frame %d @ %d \n", frame_id, rt_time_get_us());


		event_capture = rt_event_get(NULL, enqueue_capture, NULL);
		rt_event_push(event_capture);
  }

  rt_camera_close(camera, 0);
	rt_free(RT_ALLOC_L2_CL_DATA, L2_image, WIDTH*HEIGHT*sizeof(unsigned char));


  return 0;
}
