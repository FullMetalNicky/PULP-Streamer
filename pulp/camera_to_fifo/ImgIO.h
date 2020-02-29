/*
 * Copyright (C) 2017 GreenWaves Technologies
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 *
 */

#ifndef __IMG_IO_H__
#define __IMG_IO_H__

#include "rt/rt_api.h"

unsigned char *ReadImageFromFile(char *ImageName, unsigned int *W, unsigned int *H, unsigned char *InBuffer, unsigned int BuffSize);
int WriteImageToFile(char *ImageName, unsigned int W, unsigned int H, unsigned char *OutBuffer);
int WriteImageToFifo(char *fifoName, unsigned int W, unsigned int H, unsigned char *OutBuffer);
int PushImageToFifo(int fifo, unsigned int W, unsigned int H, unsigned char *OutBuffer);
#endif //__IMG_IO_H__
