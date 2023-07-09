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
void Conv2D_3x3(float x_sm[X_CHANNEL*X_SIZE*X_SIZE], float w_sm[W_CHANNEL*W_SIZE*W_SIZE], 
float b_sm[B_CHANNEL], int x_w, int x_h, int x_c, int z_c, float z_out[Z_CHANNEL*Z_SIZE*Z_SIZE])
{

    // Initialize output tensor to zero
    for(int i = 0; i < z_c * Z_SIZE * Z_SIZE; i++){
        z_out[i] = 0;
    }

    /* access data in DRAM to BRAM*/
    float x[KERNEL_SIZE*KERNEL_SIZE];
    float w[KERNEL_SIZE*KERNEL_SIZE];
    for (int row = 0; row < KERNEL_SIZE; row++)
    {
        for (int column = 0; column < KERNEL_SIZE; column++)
        {
            
        }
    }

    // Perform convolution
    for(int i = 0; i < z_c; i++) {
        for(int j = 0; j < Z_SIZE; j++) {
            for(int k = 0; k < Z_SIZE; k++) {
                for(int c = 0; c < x_c; c++) {
                    for(int p = 0; p < KERNEL_SIZE; p++) {
                        for(int q = 0; q < KERNEL_SIZE; q++) {

                            // load w for each kernel
                            get_w(w, p, q) = get_W(w_sm, i, c, p, q);

                            z_out[(i * Z_SIZE + j) * Z_SIZE + k] += x_sm[(c * x_h + (j + p)) * x_w + (k + q)] * w_sm[(i * x_c + c) * KERNEL_SIZE * KERNEL_SIZE + p * KERNEL_SIZE + q];
                        }
                    }
                }
                z_out[(i * Z_SIZE + j) * Z_SIZE + k] += b_sm[i];
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
