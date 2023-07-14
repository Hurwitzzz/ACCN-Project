#include "conv.h"
#include <stdio.h>
#include <cstdlib>

/*-------------------------------- Winograd -------------------------------------------*/
/* 
 * Pre Transform the Weights
 * WINOGRAD_STRUCT 	*wino 	: Struct containing tile size, A^T, G, B^T
 * Tensor 			*W		: Untransformed Weight Tensor
 * int		        out_c	: Number of output channels
 * Return:		    Tensor *	: New Tensor containing transformed Weights
 */

dt * winoWeights(dt * W, int ic, int ks, int out_c)
{
    const WINOGRAD_STRUCT * wino = getWino(ks);
    if(wino == NULL)
        return NULL;
    int tile_size = wino->tile_size;

	// Create a new array to store the transformed weights and allocate memory for it.
    dt * U = new dt [out_c][ic][tile_size][tile_size];

	// for each weight tensor
	for (int oc = 0; oc < out_c; oc++) {
		// for each weight channel
		for (int wc = 0; wc < ic; wc++) {
			// for each row in the converted weight channel
			for (int i = 0; i < tile_size; i++) {
				// for each column in the converted weight channel
				for (int j = 0; j < tile_size; j++) {
					// for each row in the kernel
					for (int k = 0; k < ks; k++) {
						// for each column in the kernel
						for (int l = 0; l < ks; l++) {
							U[oc][wc][i][j] += wino->G[i][k] * W[oc][wc][k][l] * wino->G[j][l];
						}
					}
				}
			}
		}
	}
	return U;	
}


/*
 * Convolution using inputs and converted Weights
 * Tensor 			*W_wino	: Transformed Weight Tensor
 * Tensor			*B		: Bias 
 * Tensor			*OUT		: Output Tensor
 * int 			k_size		: Width and Height of weight kernel
 */


#define IS (OS+KS-1)
template<int KS, int TS, int IN_C, int OS, int out_c>   // TS: tile size

void convWinograd(dt IN[IN_C][IS][IS], dt W_wino[out_c][IN_C][TS][TS] , dt B[out_c], dt OUT[out_c][OS][OS], int k_size)
{
    const WINOGRAD_STRUCT * wino = getWino(k_size);
    if(wino == NULL)
        return;





    int tile_stride = wino->tile_stride;
    int num_tiles = ceil(1.0*(input_width - TS)/tile_stride + 1); // each row or col
    // printf("num_tiles: %d\n", num_tiles);


    uint32_t z_width = Z->size[2];
    uint32_t z_height = Z->size[1];
    uint32_t z_channel = Z->size[0];

    // initialize output tensor to zero
    for (uint32_t i = 0; i < z_channel; i++) {
        for (uint32_t j = 0; j < z_height; j++) {
            for (uint32_t k = 0; k < z_width; k++) {
                Z->data[i][j][k] = 0;
            }
        }
    }

    // Convert the input to the winograd domain
    Tensor * T = new Tensor (input_channels, TS * num_tiles, TS * num_tiles);
    // If the input size is not divisible by the tile size, we need to pad the input with zeros
    if (num_tiles != (input_width - TS)/tile_stride + 1){
        for (int c = 0; c < input_channels; c++) {
            for (int t_row = 0; t_row < num_tiles; t_row++){
                for (int t_col = 0; t_col < num_tiles; t_col++){
                    for (int i = 0; i < TS; i++) {
                        for (int j = 0; j < TS; j++) {
                            for (int k = 0; k < TS; k++) {
                                for (int l = 0; l < TS; l++) {
                                    if (k+ (TS - k_size +1) * t_row < input_width && l+ (TS - k_size +1) * t_col < input_width){
                                        T->data[c][i + TS * t_row][j + TS * t_col]+= wino->Bt[i][k] * X->data[c][k+ (TS - k_size +1) * t_row][l+ (TS - k_size +1) * t_col] * wino->Bt[j][l];
                                    }
                                    else {
                                        T->data[c][i + TS * t_row][j + TS * t_col]+= wino->Bt[i][k] * 0 * wino->Bt[j][l];
                                    }
                                }
                            }
                        }
                    }
                }
            }
	    }
    }
    // If the input size is divisible by the tile size, we don't need to pad the input with zeros
    else {
        for (int c = 0; c < input_channels; c++) {
            for (int t_row = 0; t_row < num_tiles; t_row++){
                for (int t_col = 0; t_col < num_tiles; t_col++){
                    for (int i = 0; i < TS; i++) {
                        for (int j = 0; j < TS; j++) {
                            for (int k = 0; k < TS; k++) {
                                for (int l = 0; l < TS; l++) {
                                    T->data[c][i + TS * t_row][j + TS * t_col]+= wino->Bt[i][k] * X->data[c][k+ (TS - k_size +1) * t_row][l+ (TS - k_size +1) * t_col] * wino->Bt[j][l];
                                }
                            }
                        }
                    }
                }
            }
	    }
    }
    


    // Convolution in the winograd domain
    // for each weight tensor
    for (int w = 0; w < output_channels; w++){
        // for each tile in row
        for (int t_row = 0; t_row < num_tiles; t_row++){
            for (int t_col = 0; t_col < num_tiles; t_col++){
                // eliment-wise multi T with weight, then add up all channels 
                // Initiate m for every tile. m is the output of one tile and one weight tensor, channels are added up, include Bias
                Tensor * m = new Tensor (1, TS, TS);
                // for each input channel, add up
                for (int ic = 0; ic < input_channels; ic++){
                    // for each row in tile
                    for (int i = 0; i < TS; i++){
                        // for each col in tile
                        for (int j = 0; j < TS; j++){
                            // eliment-wise multi, Bias
                            // printf("i: %d, j: %d, t_row: %d, t_col: %d, w: %d, ic: %d\n", i, j, t_row, t_col, w, ic);
                            m->data[0][i][j] += T->data[ic][i + TS * t_row][j + TS * t_col] * U_wino[w].data[ic][i][j];
                        }
                    }
                }
                // convert m from winograd domain back, add the bias, and then allocate to Z
                for (int i = 0; i < wino->out_size; i++){
                    for (int j = 0; j < wino->out_size; j++){
                        if (i + wino->out_size * t_row < z_height && j + wino->out_size * t_col < z_width){
                            for (int k =0; k < TS; k++){
                                for (int l = 0; l < TS; l++){
                                    // printf("w: %d, i: %d, j: %d, k: %d, l: %d\n", w, i, j, k, l);
                                    Z->data[w][i + wino->out_size * t_row][j + wino->out_size * t_col] += wino->At[i][k] * m->data[0][k][l] * wino->At[j][l];
                                }
                            }
                            Z->data[w][i + wino->out_size * t_row][j + wino->out_size * t_col] += B->data[0][0][w];
                        }
                    }
                }
                delete m;
            }
        }
    }
    delete T;
}

