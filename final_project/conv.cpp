#include "conv.h"

void Conv2D_3x3(float x_in[X_SIZE], float w_in[W_SIZE], 
float b_in[B_SIZE], int x_w, int x_h, int x_c, int z_c, float z_out[Z_SIZE], int padding)
{
    
    int z_w = x_w - KERNEL_SIZE + 1;
    int z_h = x_h - KERNEL_SIZE + 1;


    // Initialize output tensor to zero
    for(int i = 0; i < z_c * z_h * z_w; i++){
        z_out[i] = 0;
    }

    // Perform convolution
    for(int i = 0; i < z_c; i++) {
        for(int j = 0; j < z_h; j++) {
            for(int k = 0; k < z_w; k++) {
                for(int c = 0; c < x_c; c++) {
                    for(int p = 0; p < KERNEL_SIZE; p++) {
                        for(int q = 0; q < KERNEL_SIZE; q++) {
                            z_out[(i * z_h + j) * z_w + k] += x_in[(c * x_h + (j + p)) * x_w + (k + q)] * w_in[(i * x_c + c) * KERNEL_SIZE * KERNEL_SIZE + p * KERNEL_SIZE + q];
                        }
                    }
                }
                z_out[(i * z_h + j) * z_w + k] += b_in[i];
            }
        }
    }
}


