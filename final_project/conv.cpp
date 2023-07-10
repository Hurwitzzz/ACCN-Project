#include "conv.h"
#include <stdio.h>
#include <cstdlib>

/* in_w, in_h, etc. are the actual size */
void Conv2D_3x3(float in_sm[IN_CHANNEL*IN_SIZE*IN_SIZE],
	float w_sm[OUT_CHANNEL*IN_CHANNEL*KERNEL_SIZE*KERNEL_SIZE],
	float b_sm[OUT_CHANNEL],
	int in_w, int in_h, int in_c, int out_c,
	float out_sm[OUT_CHANNEL*OUT_SIZE*OUT_SIZE])
{
	int out_h = in_h - KERNEL_SIZE + 1;
	int out_w = in_w - KERNEL_SIZE + 1;
    // Initialize output tensor to zero
    for(int i = 0; i < out_c * out_h * out_w; i++){
	    out_sm[i] = 0;
    }

    /* Store in BRAM */
    float x[KERNEL_SIZE*KERNEL_SIZE];
    float w[KERNEL_SIZE*KERNEL_SIZE];
    float b[OUT_CHANNEL];
    float z[OUT_SIZE*OUT_SIZE]; // due to the limitation of BRAM, we can only store one channel of z

    // Perform convolution
L1:		for(int oc = 0; oc < out_c; oc++) {
			int oc_W_idx = oc * in_c * KERNEL_SIZE * KERNEL_SIZE;
			int oc_OUT_idx = oc * out_w * out_h;
L2:			for(int j = 0; j < out_w; j++) {
				int j_idx = j * out_w;
L3:             for(int k = 0; k < out_h; k++) {
                    float acc_channel[IN_CHANNEL];      // create buffer for each channel  
                    float acc_kernel[KERNEL_SIZE * KERNEL_SIZE]; // create buffer for each kernel_size conv        
L4:                 for(int ic = 0; ic < in_c; ic++) {
#pragma HLS PIPELINE II=1
						int ic_IN_idx = ic * in_w * in_w;
						int ic_W_idx = ic * KERNEL_SIZE * KERNEL_SIZE;
                        // load x for each kernel, and load w
                        for(int p = 0; p < KERNEL_SIZE; p++) {
                            int p_idx = p * KERNEL_SIZE;
                            int j_plus_p_IN_idx = (j+p) * in_w;
                            for(int q = 0; q < KERNEL_SIZE; q++) {
                                w[p_idx+q] = w_sm[oc_W_idx+ic_W_idx+p_idx+q];  // w[p][q] = w_sm[oc][ic][p][q];
                                if (q>0 && k==0){
                                    x[p_idx+q] = in_sm[ic_IN_idx+j_plus_p_IN_idx+q-1];      // x[p][q] = in_sm[ic][j + p][q - 1];
                                }
                            }
                        }
                    

                        for (int p = 0; p < KERNEL_SIZE; p++) {
                            int p_idx = p * KERNEL_SIZE;
                            int j_plus_p_IN_idx = (j+p) * in_w;
                            for (int q = 0; q < KERNEL_SIZE; q++) {
                                // reuse the data in BRAM
                                x[p_idx+q] = (q == KERNEL_SIZE - 1) ? in_sm[ic_IN_idx+j_plus_p_IN_idx+k+q] : x[p_idx+q+1];   // x[p][q] = (q == KERNEL_SIZE - 1) ? in_sm[ic][j + p][k + q] : x[p][q + 1];
                                acc_kernel[p * KERNEL_SIZE + q] = x[p_idx+q] * w[p_idx+q]; // x[p][q] * w[p][q];
                            }
                        }
                        // sum up the results of one kernel
                        acc_channel[ic] = 0;
                        for(int p = 0; p < KERNEL_SIZE * KERNEL_SIZE; p++) {
                            acc_channel[ic] += acc_kernel[p];
                        }
                    }
                    // sum up the result of one channel
                    z[j_idx+k] = 0;

                    for(int ic = 0; ic < in_c; ic++) {
                        z[j_idx+k] += acc_channel[ic]; // z[j][k] += acc_channel[ic];
                    }
                    // add bias
                    z[j_idx+k] += b_sm[oc]; // z[j][k] += b_sm[oc];
                } 
                    
            }
            // send the result to out_sm (in DRAM)
            for(int j = 0; j < out_w; j++) {
				int j_idx = j * out_h;
                for(int k = 0; k < out_h; k++) {
                    out_sm[oc_OUT_idx+j_idx+k] = z[j_idx+k]; // out_sm[oc][j][k] = z[j][k];
                }
            }
        }
        
}


void EntryConv(float in_sm[IN_CHANNEL*IN_SIZE*IN_SIZE], 
	float w_sm[OUT_CHANNEL*IN_CHANNEL*KERNEL_SIZE*KERNEL_SIZE], 
	float b_sm[OUT_CHANNEL],
	int in_w, int in_h, int in_c, int out_c, 
	float out_sm[OUT_CHANNEL*OUT_SIZE*OUT_SIZE])
{
    #pragma HLS INTERFACE m_axi port=in_sm,w_sm depth = 512
    #pragma HLS INTERFACE m_axi port=b_sm depth = 10
    #pragma HLS INTERFACE m_axi port=out_sm depth = 512

    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS INTERFACE s_axilite port=in_sm,w_sm
    #pragma HLS INTERFACE s_axilite port=b_sm 
    #pragma HLS INTERFACE s_axilite port=out_sm 

    Conv2D_3x3(in_sm, w_sm, b_sm, in_w, in_h, in_c, out_c, out_sm);
}
