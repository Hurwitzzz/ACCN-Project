// Description: header file for conv.c
#ifndef _CONV_H_
#define _CONV_H_

#define KERNEL_SIZE 3
#define IN_SIZE 140 // if possible, set to 128 
#define IN_CHANNEL 512
#define OUT_SIZE (IN_SIZE - KERNEL_SIZE + 1)
#define OUT_CHANNEL 512

void EntryConv(float IN[IN_CHANNEL*IN_SIZE*IN_SIZE],
	float W[OUT_CHANNEL*IN_CHANNEL*KERNEL_SIZE*KERNEL_SIZE],
	float B[OUT_CHANNEL],
	int in_w, int in_h, int in_c, int out_c,
	float OUT[OUT_CHANNEL*OUT_SIZE*OUT_SIZE]);


#endif
