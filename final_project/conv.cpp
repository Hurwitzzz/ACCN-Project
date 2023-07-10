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
                                get_w(w,p,q) = get_W(w_sm,i,c,p,q,in_c);  // w[p][q] = w_sm[i][c][p][q];   
                                    get_in(x,p,q) =  get_IN(in_sm,c,(j+p),(q-1),in_w);      // x[p][q] = in_sm[c][j + p][q - 1];  
                                }
                        }
                    

                        for (int p = 0; p < KERNEL_SIZE; p++) {
                            for (int q = 0; q < KERNEL_SIZE; q++) {
                                // reuse the data in BRAM
                                get_in(x,p,q) = (q == KERNEL_SIZE - 1) ? get_IN(in_sm,c,(j+p),(k+q),in_w) : get_in(x,p,(q+1));   // x[p][q] = (q == KERNEL_SIZE - 1) ? in_sm[c][j + p][k + q] : x[p][q + 1];
                                acc_kernel[p * KERNEL_SIZE + q] = get_in(x,p,q) * get_w(w,p,q); // x[p][q] * w[p][q];
                            }
                        }
                        // sum up the results of one kernel
                        acc_channel[c] = 0;
                        for(int p = 0; p < KERNEL_SIZE * KERNEL_SIZE; p++) {
                            acc_channel[c] += acc_kernel[p];
                        } 
                    }
                    // sum up the result of one channel
                    get_z(z,j,k,in_w) = 0;
                    for(int c = 0; c < in_c; c++) {
                        get_z(z,j,k,in_w) += acc_channel[c]; // z[j][k] += acc_channel[c];
                    }
                    // add bias
                    get_z(z,j,k,in_w) += b_sm[i]; // z[j][k] += b_sm[i];                      
                } 
                    
            }
            // send the result to out_sm (in DRAM)
            for(int j = 0; j < OUT_SIZE; j++) {
                for(int k = 0; k < OUT_SIZE; k++) {
                    get_OUT(out_sm,i,j,k,out_w) = get_z(z,j,k,in_w); // out_sm[i][j][k] = z[j][k];
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






/* Given index, get value
    arg: 
        float tensor[]: the tensor to get value from
        int ch: channle
        int row: 
        int column
*/

// will memory be allocated when compiling? If yes, these functions waste too much memory

float& get_in(float tensor[KERNEL_SIZE*KERNEL_SIZE],int row, int column)
{
    if (row < 0 || row >= KERNEL_SIZE || column < 0 || column >= KERNEL_SIZE)
    {
        printf("Error: get_x out of bound\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        return tensor[row * KERNEL_SIZE + column];
    }
}
float& get_IN(float tensor[IN_CHANNEL*IN_SIZE*IN_SIZE], int ch, int row, int column, int in_w)
{

    if (ch < 0 || ch >= IN_CHANNEL || row < 0 || row >= IN_SIZE || column < 0 || column >= IN_SIZE)
    {
        printf("Error: get_X out of bound\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        return tensor[(ch * in_w + row) * in_w + column];
    }
}

float& get_w(float tensor[KERNEL_SIZE*KERNEL_SIZE], int row, int column)
{
    if (row < 0 || row >= KERNEL_SIZE || column < 0 || column >= KERNEL_SIZE)
    {
        printf("Error: get_w out of bound\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        return tensor[row * KERNEL_SIZE + column];
    }
}
float& get_W(float tensor[OUT_CHANNEL * IN_CHANNEL * KERNEL_SIZE * KERNEL_SIZE], int z_ch, int ch, int row, int column, int in_c)
{
    if (z_ch < 0 || z_ch >= OUT_CHANNEL || ch < 0 || ch >= IN_CHANNEL || row < 0 || row >= KERNEL_SIZE || column < 0 || column >= KERNEL_SIZE)
    {
        printf("Error: get_W out of bound\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        return tensor[(z_ch * in_c + ch) * KERNEL_SIZE * KERNEL_SIZE + (row * KERNEL_SIZE + column)];
    }
}
float& get_z(float tensor[OUT_SIZE * OUT_SIZE], int row, int column, int in_w)
{
    if (row < 0 || row >= OUT_SIZE || column < 0 || column >= OUT_SIZE)
    {
        printf("Error: get_z out of bound\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        return tensor[row * in_w + column];
    }
}

float& get_OUT(float tensor[OUT_CHANNEL*OUT_SIZE*OUT_SIZE], int ch, int row, int column, int out_w)
{
    if (ch < 0 || ch >= OUT_CHANNEL || row < 0 || row >= OUT_SIZE || column < 0 || column >= OUT_SIZE)
    {
        printf("Error: get_OUT out of bound\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        return tensor[(ch * out_w + row) * out_w + column];
    }
}
