#include "conv.h"
#include <stdio.h>

/* Given index, get value
    arg: 
        float tensor[]: the tensor to get value from
        int ch: channle
        int row: 
        int column
*/


/* x_w, x_h, etc. are the actual size */
void Conv2D_3x3(float x_sm[X_CHANNEL][X_SIZE][X_SIZE], 
float w_sm[W_CHANNEL][KERNEL_SIZE][KERNEL_SIZE][KERNEL_SIZE], 
float b_sm[B_CHANNEL], 
int x_w, int x_h, int x_c, int z_c, 
float z_sm[Z_CHANNEL][Z_SIZE][Z_SIZE])
{

    // Initialize output tensor to zero
    for(int i = 0; i < z_c; i++){
        for(int j = 0; j < Z_SIZE; j++){
            for(int k = 0; k < Z_SIZE; k++){
                z_sm[i][j][k] = 0;
            }
        }
    }

    /* Store in BRAM */
    float x[KERNEL_SIZE][KERNEL_SIZE];
    float w[KERNEL_SIZE][KERNEL_SIZE];
    float b[B_CHANNEL];
    float z[Z_SIZE][Z_SIZE]; // due to the limitation of BRAM, we can only store one channel of z

    // Perform convolution
L1:    for(int i = 0; i < z_c; i++) {
L2:        for(int j = 0; j < Z_SIZE; j++) {
L3:             for(int k = 0; k < Z_SIZE; k++) {

                    float acc_channel[X_CHANNEL];      // create buffer for each channel  
                    float acc_kernel[KERNEL_SIZE * KERNEL_SIZE]; // create buffer for each kernel_size conv        
L4:                 for(int c = 0; c < x_c; c++) {
#pragma HLS PIPELINE II=1
                        // load x for each kernel, and load w
                        for(int p = 0; p < KERNEL_SIZE; p++) {
                            for(int q = 0; q < KERNEL_SIZE; q++) {
                                w[p][q] = w_sm[i][c][p][q];
                                if (q > 0 && k ==0){
                                    x[p][q] = x_sm[c][j + p][q - 1];
                                }
                            }
                        }

                        for (int p = 0; p < KERNEL_SIZE; p++) {
                            for (int q = 0; q < KERNEL_SIZE; q++) {
                                // reuse the data in BRAM
                                x[p][q] = (q == KERNEL_SIZE - 1) ? x_sm[c][j + p][k + q] : x[p][q + 1];
                                acc_kernel[p * KERNEL_SIZE + q] = x[p][q] * w[p][q];
                            }
                        }  
                        // sum up the result of each kernel
                        for(int p = 0; p < KERNEL_SIZE * KERNEL_SIZE; p++) {
                            acc_channel[c] += acc_kernel[p];
                        } 
                    } 
                    // sum up the result of each channel
                    for(int c = 0; c < x_c; c++) {
                        z[j][k] += acc_channel[c];
                    }
                    // add bias
                    z[j][k] += b_sm[i];
            }
        }
        // send the result to z_sm (in DRAM)
        for(int j = 0; j < Z_SIZE; j++) {
            for(int k = 0; k < Z_SIZE; k++) {
                z_sm[i][j][k] = z[j][k];
            }
        }
    }
}









float get_x(float tensor[KERNEL_SIZE*KERNEL_SIZE],int row, int column)
{
    if (row < 0 || row >= KERNEL_SIZE || column < 0 || column >= KERNEL_SIZE)
    {
        printf("Error: get_x out of bound\n");
        return 0;
    }
    else
    {
        return tensor[row * KERNEL_SIZE + column];
    }
}
float get_X(float tensor[X_CHANNEL*X_SIZE*X_SIZE], int ch, int row, int column)
{

    if (ch < 0 || ch >= X_CHANNEL || row < 0 || row >= X_SIZE || column < 0 || column >= X_SIZE)
    {
        printf("Error: get_X out of bound\n");
        return 0;
    }
    else
    {
        return tensor[(ch * X_SIZE + row) * X_SIZE + column];
    }
}

float get_w(float tensor[KERNEL_SIZE*KERNEL_SIZE], int row, int column)
{
    if (row < 0 || row >= KERNEL_SIZE || column < 0 || column >= KERNEL_SIZE)
    {
        printf("Error: get_w out of bound\n");
        return 0;
    }
    else
    {
        return tensor[row * KERNEL_SIZE + column];
    }
}
float get_W(float tensor[], int z_ch, int ch, int row, int column)
{
    if (z_ch < 0 || z_ch >= Z_CHANNEL || ch < 0 || ch >= W_CHANNEL || row < 0 || row >= W_SIZE || column < 0 || column >= W_SIZE)
    {
        printf("Error: get_W out of bound\n");
        return 0;
    }
    else
    {
        return tensor[(z_ch * W_CHANNEL + ch) * W_SIZE * W_SIZE + (row * W_SIZE + column)];
    }
}
float get_z(float tensor[], int ch, int row, int column)
{
    
}
