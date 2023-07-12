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
#pragma HLS array_partition variable=in complete
	float w[IN_CHANNEL*KERNEL_SIZE*KERNEL_SIZE];
	float b[OUT_CHANNEL];

	// Perform convolution
OC:	for(int oc = 0; oc < out_c; oc++) {
#pragma HLS loop_tripcount max=OUT_CHANNEL

		// load w
		int oc_W_idx = oc * in_c * KERNEL_SIZE * KERNEL_SIZE;
LW:		for(int i = 0; i < in_c * KERNEL_SIZE * KERNEL_SIZE; i++) {
#pragma HLS loop_tripcount max=IN_CHANNEL*KERNEL_SIZE*KERNEL_SIZE
			w[i] = W[oc_W_idx+i]; // w = W[oc];
		}

		int oc_OUT_idx = oc * out_w * out_h;
Y:		for(int y = 0; y < out_h; y++) {
#pragma HLS loop_tripcount max=OUT_SIZE
			float acc_row[OUT_SIZE]; // One row of output
			int y_idx = y * out_w;
ZR:			for(int x = 0; x < out_w; x++) {
#pragma HLS loop_tripcount max=OUT_SIZE
				acc_row[x] = 0;
			}

IC:			for(int ic = 0; ic < in_c; ic++) {
#pragma HLS loop_tripcount max=IN_CHANNEL
#pragma HLS pipeline II=3
				int ic_IN_idx = ic * in_w * in_w;
				int ic_W_idx = ic * KERNEL_SIZE * KERNEL_SIZE;


				// load all but one column for in
IY:				for(int p = 0; p < KERNEL_SIZE; p++) {
#pragma HLS pipeline II=2
					int p_idx = p * KERNEL_SIZE;
					int y_plus_p_IN_idx = (y+p) * in_w;
IX:					for(int q = 0; q < KERNEL_SIZE - 1; q++) {
#pragma HLS pipeline II=1
						in[p_idx+q] = IN[ic_IN_idx+y_plus_p_IN_idx+q];	  // in[p][q] = IN[ic][y + p][q];
						//in[p*KERNEL_SIZE+q] = IN[ic * in_w * in_w + (y+p) * in_w +q];
					}
				}

				int xmk = -1;
X:				for(int x = 0; x < out_w; x++) {
#pragma HLS loop_tripcount max=OUT_SIZE
#pragma HLS pipeline II=3
					float acc_kernel[KERNEL_SIZE * KERNEL_SIZE]; // create buffer for each kernel_size conv
#pragma HLS array_partition variable=acc_kernel complete

					xmk++;
					if(xmk >= KERNEL_SIZE) xmk -= KERNEL_SIZE;

					// Replace old column with new column for in
C1:					for(int p = 0; p < KERNEL_SIZE; p++) {
#pragma HLS pipeline II=1
						int p_idx = p * KERNEL_SIZE;
						int y_plus_p_IN_idx = (y+p) * in_w;
						int xmk_plus_KS_minus_1_mod_KS = xmk+KERNEL_SIZE-1;
						if(xmk_plus_KS_minus_1_mod_KS >= KERNEL_SIZE) xmk_plus_KS_minus_1_mod_KS -= KERNEL_SIZE;
						in[p_idx+ xmk_plus_KS_minus_1_mod_KS] = IN[ic_IN_idx+y_plus_p_IN_idx+x+KERNEL_SIZE-1];   // in[p][q] = (q == KERNEL_SIZE - 1) ? IN[ic][y + p][x + q] : in[p][q + 1];
					}

					// Conv calculation
C3:					for(int p = 0; p < KERNEL_SIZE; p++) {
						int p_idx = p * KERNEL_SIZE;
						int y_plus_p_IN_idx = (y+p) * in_w;
C4:						for (int q = 0; q < KERNEL_SIZE; q++) {
#pragma HLS pipeline II=1
							// reuse the data in BRAM
    						int xmk_plus_q_mod_KS = xmk+q;
    						if(xmk_plus_q_mod_KS >= KERNEL_SIZE) xmk_plus_q_mod_KS -= KERNEL_SIZE;
							acc_kernel[p_idx+q] = in[p_idx+ xmk_plus_q_mod_KS] * w[ic_W_idx+p_idx+q]; // in[p][q] * w[p][q];
						}
					}

					float acc = 0;
					// sum up the results of one kernel
AK:					for(int i = 0; i < KERNEL_SIZE * KERNEL_SIZE; i++) {
#pragma HLS unroll
						acc += acc_kernel[i];
					}
					acc_row[x] += acc;
				} // for x

			} // for ic

			// add bias and send one row to OUT (in DRAM)
WO:			for(int x = 0; x < out_w; x++) {
#pragma HLS loop_tripcount max=OUT_SIZE
#pragma HLS pipeline II=1
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
    #pragma HLS INTERFACE m_axi port=IN depth = 512
    #pragma HLS INTERFACE m_axi port=W depth = 512
    #pragma HLS INTERFACE m_axi port=B depth = 512
    #pragma HLS INTERFACE m_axi port=OUT depth = 512

    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS INTERFACE s_axilite port=IN,W
    #pragma HLS INTERFACE s_axilite port=B
    #pragma HLS INTERFACE s_axilite port=OUT

    #pragma HLS INTERFACE s_axilite port=in_w,in_h,in_c,out_c

    Conv2D_3x3(IN, W, B, in_w, in_h, in_c, out_c, OUT);
}
