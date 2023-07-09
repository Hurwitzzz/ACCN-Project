#include "conv.h"
#include <stdio.h>

/* in_w, in_h, etc. are the actual size */
void Conv2D_3x3(float in_sm[IN_CHANNEL*IN_SIZE*IN_SIZE], 
	float w_sm[OUT_CHANNEL*IN_CHANNEL][KERNEL_SIZE][KERNEL_SIZE], 
	float b_sm[OUT_CHANNEL],
	int in_w, int in_h, int in_c, int out_c, 
	float out_sm[OUT_CHANNEL*OUT_SIZE*OUT_SIZE])
{
	int output_height = in_h - KERNEL_SIZE + 1;
	int output_width = in_w - KERNEL_SIZE + 1;
    // Initialize output tensor to zero
    for(int i = 0; i < out_c * output_height * output_width; i++){
	    out_sm[i] = 0;
    }

    /* Store in BRAM */
    float x[KERNEL_SIZE][KERNEL_SIZE];
    float w[KERNEL_SIZE][KERNEL_SIZE];
    float b[OUT_CHANNEL];
    float z[OUT_SIZE*OUT_SIZE]; // due to the limitation of BRAM, we can only store one channel of z

    // Perform convolution
L1:    for(int i = 0; i < out_c; i++) {
L2:        for(int j = 0; j < OUT_SIZE; j++) {
L3:             for(int k = 0; k < OUT_SIZE; k++) {

                    float acc_channel[IN_CHANNEL];      // create buffer for each channel  
                    float acc_kernel[KERNEL_SIZE * KERNEL_SIZE]; // create buffer for each kernel_size conv        
L4:                 for(int c = 0; c < in_c; c++) {
#pragma HLS PIPELINE II=1
                        // load x for each kernel, and load w
                        for(int p = 0; p < KERNEL_SIZE; p++) {
                            for(int q = 0; q < KERNEL_SIZE; q++) {
                                w[p][q] = w_sm[i][c][p][q];
                                if (q > 0 && k ==0){
                                    x[p][q] = in_sm[c][j + p][q - 1];
                                }
                            }
                        }

                        for (int p = 0; p < KERNEL_SIZE; p++) {
                            for (int q = 0; q < KERNEL_SIZE; q++) {
                                // reuse the data in BRAM
                                x[p][q] = (q == KERNEL_SIZE - 1) ? in_sm[c][j + p][k + q] : x[p][q + 1];
                                acc_kernel[p * KERNEL_SIZE + q] = x[p][q] * w[p][q];
                            }
                        }  
                        // sum up the result of each kernel
                        for(int p = 0; p < KERNEL_SIZE * KERNEL_SIZE; p++) {
                            acc_channel[c] += acc_kernel[p];
                        } 
                    } 
                    // sum up the result of each channel
                    for(int c = 0; c < in_c; c++) {
                        z[j][k] += acc_channel[c];
                    }
                    // add bias
                    z[j][k] += b_sm[i];
            }
        }
        // send the result to out_sm (in DRAM)
        for(int j = 0; j < OUT_SIZE; j++) {
            for(int k = 0; k < OUT_SIZE; k++) {
                out_sm[i][j][k] = z[j][k];
            }
        }
    }
}


void EntryConv(float in_sm[IN_CHANNEL*IN_SIZE*IN_SIZE], 
	float w_sm[OUT_CHANNEL*IN_CHANNEL][KERNEL_SIZE][KERNEL_SIZE], 
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






/* Given index, get value
    arg: 
        float tensor[]: the tensor to get value from
        int ch: channle
        int row: 
        int column
*/
// float get_x(float tensor[KERNEL_SIZE*KERNEL_SIZE],int row, int column)
// {
//     if (row < 0 || row >= KERNEL_SIZE || column < 0 || column >= KERNEL_SIZE)
//     {
//         printf("Error: get_x out of bound\n");
//         return 0;
//     }
//     else
//     {
//         return tensor[row * KERNEL_SIZE + column];
//     }
// }
// float get_X(float tensor[IN_CHANNEL*IN_SIZE*IN_SIZE], int ch, int row, int column)
// {

//     if (ch < 0 || ch >= IN_CHANNEL || row < 0 || row >= IN_SIZE || column < 0 || column >= IN_SIZE)
//     {
//         printf("Error: get_X out of bound\n");
//         return 0;
//     }
//     else
//     {
//         return tensor[(ch * IN_SIZE + row) * IN_SIZE + column];
//     }
// }

// float get_w(float tensor[KERNEL_SIZE*KERNEL_SIZE], int row, int column)
// {
//     if (row < 0 || row >= KERNEL_SIZE || column < 0 || column >= KERNEL_SIZE)
//     {
//         printf("Error: get_w out of bound\n");
//         return 0;
//     }
//     else
//     {
//         return tensor[row * KERNEL_SIZE + column];
//     }
// }
// float get_W(float tensor[], int z_ch, int ch, int row, int column)
// {
//     if (z_ch < 0 || z_ch >= OUT_CHANNEL || ch < 0 || ch >= W_CHANNEL || row < 0 || row >= W_SIZE || column < 0 || column >= W_SIZE)
//     {
//         printf("Error: get_W out of bound\n");
//         return 0;
//     }
//     else
//     {
//         return tensor[(z_ch * W_CHANNEL + ch) * W_SIZE * W_SIZE + (row * W_SIZE + column)];
//     }
// }
// float get_z(float tensor[], int ch, int row, int column)
// {
    
// }
