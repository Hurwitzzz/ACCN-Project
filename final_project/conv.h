#ifndef _CONV_H_
#define _CONV_H_

//#include "ap_fixed.h"

//typedef ap_fixed<16,9> fixed;
#define dt float

#define MAX_KERNEL_SIZE 3
#define MIN_KERNEL_SIZE 3
#define MAX_IN_SIZE 128
#define MAX_IN_CHANNEL 512
#define MAX_OUT_SIZE (MAX_IN_SIZE - MIN_KERNEL_SIZE + 1)
#define OUT_CHANNEL 384

void EntryConv(dt IN[MAX_IN_CHANNEL*MAX_IN_SIZE*MAX_IN_SIZE],
               dt W[OUT_CHANNEL*MAX_IN_CHANNEL*MAX_KERNEL_SIZE*MAX_KERNEL_SIZE],
               dt B[OUT_CHANNEL],
               dt OUT[OUT_CHANNEL*MAX_OUT_SIZE*MAX_OUT_SIZE],
               int block);


#endif
