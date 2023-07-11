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
    // Initialize output tensor to zero
    for(int i = 0; i < out_c * out_h * out_w; i++){
	    OUT[i] = 0;
    }

    /* Store in BRAM */
    float in[KERNEL_SIZE*KERNEL_SIZE];
    float w[KERNEL_SIZE*KERNEL_SIZE];
    float b[OUT_CHANNEL];
    float out[OUT_SIZE*OUT_SIZE]; // due to the limitation of BRAM, we can only store one channel of OUT

    // Perform convolution
L1:		for(int oc = 0; oc < out_c; oc++) {
			int oc_W_idx = oc * in_c * KERNEL_SIZE * KERNEL_SIZE;
			int oc_OUT_idx = oc * out_w * out_h;
            float acc_channel[IN_CHANNEL][OUT_SIZE*OUT_SIZE];
            for (int ic = 0; ic < in_c; ic++) {
                int ic_IN_idx = ic * in_w * in_w;
                int ic_W_idx = ic * KERNEL_SIZE * KERNEL_SIZE;
                
                // load w
                for(int p = 0; p < KERNEL_SIZE; p++) {
                    int p_idx = p * KERNEL_SIZE;
                    for(int q = 0; q < KERNEL_SIZE; q++) {
                        w[p_idx+q] = W[oc_W_idx+ic_W_idx+p_idx+q]; // w[p][q] = W[oc][ic][p][q];
                    }
                }

    L2:			for(int y = 0; y < out_h; y++) {
                    int y_idx = y * out_w;
    L3:             for(int x = 0; x < out_w; x++) {
                        // float acc_channel[IN_CHANNEL];      // create buffer for each channel  
                        float acc_kernel[KERNEL_SIZE * KERNEL_SIZE]; // create buffer for each kernel_size conv        
                            
                        // load x for each kernel
                        for(int p = 0; p < KERNEL_SIZE; p++) {
                            int p_idx = p * KERNEL_SIZE;
                            int y_plus_p_IN_idx = (y+p) * in_w;
                            for(int q = 0; q < KERNEL_SIZE; q++) {
                                if (q>0 && x==0){
                                    in[p_idx+q] = IN[ic_IN_idx+y_plus_p_IN_idx+q-1];      // in[p][q] = IN[ic][y + p][q - 1];
                                }
                            }
                        }
                    
                        // Conv calculation
                        for (int p = 0; p < KERNEL_SIZE; p++) {
                            int p_idx = p * KERNEL_SIZE;
                            int y_plus_p_IN_idx = (y+p) * in_w;
                            for (int q = 0; q < KERNEL_SIZE; q++) {
                                // reuse the data in BRAM
                                in[p_idx+q] = (q == KERNEL_SIZE - 1) ? IN[ic_IN_idx+y_plus_p_IN_idx+x+q] : in[p_idx+q+1];   // in[p][q] = (q == KERNEL_SIZE - 1) ? IN[ic][y + p][x + q] : in[p][q + 1];
                                acc_kernel[p * KERNEL_SIZE + q] = in[p_idx+q] * w[p_idx+q]; // in[p][q] * w[p][q];
                            }
                        }
                        // sum up the results of one kernel
                        acc_channel[ic][y_idx + x] = 0;
                        for(int p = 0; p < KERNEL_SIZE * KERNEL_SIZE; p++) {
                            acc_channel[ic][y_idx + x] += acc_kernel[p];
                        }
                    } // for x
                        
                } // for y
            } //for ic

            // sum up the result of one output channel
            // send the result to OUT (in DRAM)
            for(int y = 0; y < out_h; y++) {
                int y_idx = y * out_w;
                for(int x = 0; x < out_w; x++) {
                    out[y_idx+x] = 0;
                    for(int ic = 0; ic < in_c; ic++) {
                        out[y_idx+x] += acc_channel[ic][y_idx+x]; // out[y][x] += acc_channel[ic];
                    }
                    // add bias
                    out[y_idx+x]+=B[oc];
                    OUT[oc_OUT_idx+y_idx+x] = out[y_idx+x]; // OUT[oc][y][x] = out[y][x];
                }
            }
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
