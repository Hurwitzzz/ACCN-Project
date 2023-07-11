#include "conv.h"
#include <stdio.h>
#include <cstdlib>

/* in_w, in_h, etc. are the actual size */
void Conv2D_3x3(float IN[IN_CHANNEL*IN_SIZE*IN_SIZE],
	float W[OUT_CHANNEL*IN_CHANNEL*KERNEL_SIZE*KERNEL_SIZE],
	float B[OUT_CHANNEL],
	int in_w, int in_h, int in_c, int out_c,
	float OUT[OUT_CHANNEL*OUT_SIZE*OUT_SIZE])
{
	int out_h = in_h - KERNEL_SIZE + 1;
	int out_w = in_w - KERNEL_SIZE + 1;

	/* Store in BRAM */
	float in[KERNEL_SIZE*KERNEL_SIZE];
	float w[IN_CHANNEL*KERNEL_SIZE*KERNEL_SIZE];
	float b[OUT_CHANNEL];

	// Perform convolution
OC:	for(int oc = 0; oc < out_c; oc++) {

		// load w
		int oc_W_idx = oc * in_c * KERNEL_SIZE * KERNEL_SIZE;
LW:		for(int i = 0; i < in_c * KERNEL_SIZE * KERNEL_SIZE; i++) {
			w[i] = W[oc_W_idx+i]; // w = W[oc];
		}

		int oc_OUT_idx = oc * out_w * out_h;
Y:		for(int y = 0; y < out_h; y++) {
			float acc_row[OUT_SIZE]; // One row of output
			int y_idx = y * out_w;
ZR:			for(int x = 0; x < out_w; x++) {
				acc_row[x] = 0;
			}

IC:			for(int ic = 0; ic < in_c; ic++) {
				int ic_IN_idx = ic * in_w * in_w;
				int ic_W_idx = ic * KERNEL_SIZE * KERNEL_SIZE;


				// load in
IY:				for(int p = 0; p < KERNEL_SIZE; p++) {
					int p_idx = p * KERNEL_SIZE;
					int y_plus_p_IN_idx = (y+p) * in_w;
IX:					for(int q = 1; q < KERNEL_SIZE; q++) {
						in[p_idx+q] = IN[ic_IN_idx+y_plus_p_IN_idx+q-1];	  // in[p][q] = IN[ic][y + p][q - 1];
					}
				}

X:				for(int x = 0; x < out_w; x++) {
					float acc_kernel[KERNEL_SIZE * KERNEL_SIZE]; // create buffer for each kernel_size conv

					// Conv calculation
C1:					for(int p = 0; p < KERNEL_SIZE; p++) {
						int p_idx = p * KERNEL_SIZE;
						int y_plus_p_IN_idx = (y+p) * in_w;
C2:						for (int q = 0; q < KERNEL_SIZE - 1; q++) {
							// reuse the data in BRAM
							in[p_idx+q] = in[p_idx+q+1];   // in[p][q] = (q == KERNEL_SIZE - 1) ? IN[ic][y + p][x + q] : in[p][q + 1];
						}
						in[p_idx+KERNEL_SIZE-1] = IN[ic_IN_idx+y_plus_p_IN_idx+x+KERNEL_SIZE-1];   // in[p][q] = (q == KERNEL_SIZE - 1) ? IN[ic][y + p][x + q] : in[p][q + 1];
					}

					// Conv calculation
C3:					for(int p = 0; p < KERNEL_SIZE; p++) {
						int p_idx = p * KERNEL_SIZE;
						int y_plus_p_IN_idx = (y+p) * in_w;
C4:						for (int q = 0; q < KERNEL_SIZE; q++) {
							// reuse the data in BRAM
							acc_kernel[p_idx+q] = in[p_idx+q] * w[ic_W_idx+p_idx+q]; // in[p][q] * w[p][q];
						}
					}

					// sum up the results of one kernel
AK:					for(int i = 0; i < KERNEL_SIZE * KERNEL_SIZE; i++) {
						acc_row[x] += acc_kernel[i];
					}
				} // for x

			} // for ic

			// add bias and send one row to OUT (in DRAM)
WO:			for(int x = 0; x < out_w; x++) {
				OUT[oc_OUT_idx+y_idx+x] = acc_row[x] + B[oc]; // OUT[oc][y][x] = acc_row[x] + B[oc];
			}
		} //for y
	}
}


void EntryConv(float IN[IN_CHANNEL*IN_SIZE*IN_SIZE],
	float W[OUT_CHANNEL*IN_CHANNEL*KERNEL_SIZE*KERNEL_SIZE],
	float B[OUT_CHANNEL],
	int in_w, int in_h, int in_c, int out_c, 
	float OUT[OUT_CHANNEL*OUT_SIZE*OUT_SIZE])
{
    #pragma HLS INTERFACE m_axi port=IN,W depth = 512
    #pragma HLS INTERFACE m_axi port=B depth = 10
    #pragma HLS INTERFACE m_axi port=OUT depth = 512

    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS INTERFACE s_axilite port=IN,W
    #pragma HLS INTERFACE s_axilite port=B
    #pragma HLS INTERFACE s_axilite port=OUT

    Conv2D_3x3(IN, W, B, in_w, in_h, in_c, out_c, OUT);
}
