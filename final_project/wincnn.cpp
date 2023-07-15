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
 * 
 * ts = os + ks -1 
 * overlap = ks - 1
 * stride = ts - overlap = os
 * 
 * OS is the size of one output_tile, not the size of one output_channel
 */


#define IS (OS+KS-1)
#define REAL_OS (IS-KS+1)
template<int KS, int TS, int IN_C, int OS, int out_c>   // TS: tile size
// hardcode to call convWinograd() for different layers
void convWinograd(dt IN[IN_C][IS][IS], dt W_wino[out_c][IN_C][TS][TS] , dt B[out_c], dt OUT[out_c][REAL_OS][REAL_OS], int KS)
{
    const WINOGRAD_STRUCT * wino = getWino(KS);
    int stride = OS;
    int overlap = KS-1;
    int num_tiles = ceil(1.0*(IS - TS)/stride + 1); // each row or col
    // printf("num_tiles: %d\n", num_tiles);

    // initialize output tensor to zero
    for (uint32_t i = 0; i < out_c; i++) {
        for (uint32_t j = 0; j < REAL_OS; j++) {
            for (uint32_t k = 0; k < REAL_OS; k++) {
                OUT[i][j][k] = 0;
            }
        }
    }

    // Store in BRAM
    dt w[IN_C[TS][TS];
    dt in[TS][TS];
    dt in_wino [TS][TS];
    dt b[out_c];

    // load bias
    for (int oc=0; oc < out_c; oc++){
        #pragma HLS pipeline II=1
		b[oc] = B[oc];
    }
    for (int oc=0;oc < out_c; oc++){

        // load w_wino
        int oc_W_idx = oc * IN_C * TS * TS;
        for (int ic=0; ic < IN_C; ic++){
            #pragma HLS LOOP_FLATTEN
            for (int p=0; p < TS; p++){
                for (int q=0; q < TS; q++){
                    #pragma HLS pipeline II=1
                    w[ic][p][q] = W_wino[oc][ic][p][q];
                }
            }
        }

        for (int y =0; y < num_tiles; y++){
            dt acc_row[OS][num_tiles * OS];
            int y_idx = y * num_tiles;

            // zero initial acc_roow
            for (int i=0; i < TS; i++){
                for (int j=0; j < num_tiles * TS; j++){
                    #pragma HLS pipeline II=1
                    acc_row[i][j] = 0;
                }
            }

            for (int ic = 0; ic < IN_C; ic++){
                // initial input, zeros[TS][stride].catenate(in[TS][overlap])              
                for (int p=0; p< TS; p++){
                    for (int q=0; q < TS; q++){
                        #pragma HLS pipeline II=1
                        if(q>=stride){
                            in[p][q] = IN[ic][p][q - stride];
                        }
                    }
                }
                // each output col
                for (int x = 0; x < num_tiles; x++){
                    dt acc[OS][OS];
                    // load in, reuse
                    for (int p=0; p<TS; p++){
                        for (int q=0; q<TS; q++){
                            #pragma HLS pipeline II=1
                            if (k+ (tile_size - k_size +1) * t_row < input_width && 
                            l+ (tile_size - k_size +1) * t_col < input_width){
                                in[p][q] = (q >= overlap) ? IN[ic][p][x*TS - overlap*(x-1)] : 0;
                            }
                            
                            }
                        }
                    }
                    // convert in to in_wino

                    // multiply in_wino with w_wino, store in acc

                    // acc_row += acc
                }
            }
            // convert acc_row back

            // add bias

            // store back to OUT


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
                        if (i + wino->out_size * t_row < OS && j + wino->out_size * t_col < OS){
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

