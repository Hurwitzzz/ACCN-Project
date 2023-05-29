#include "conv.h"

/*----------------------------- Helper Functions -------------------------------------*/
/*
 * This takes the input matrix c (*W_in)[c] flips it and stores it
 * in the real values of the complex Matrix W_out (1 channel)
 * Inputs:
 * Tensor	*	W_in:	Pointer to input Tensor (real)
 * C_Tensor	*	W_out:	Pointer to the output Tensor values stored in real part (complex) 
 * int 			c:		Channel of input to use
 */
void flip_Matrix(Tensor * W_in, C_Tensor * W_out, int c)
{
	int k_width = W_in->size[1];
	int upper_lim = k_width/2 + k_width%2;
	int extra = -1 + k_width%2;
	for(int i =-k_width/2; i < upper_lim; i++){
		for(int j =-k_width/2; j<upper_lim; j++){
			(*W_out)[0][i+k_width/2][j+k_width/2].real( 
				(*W_in)[c][-i + k_width/2 + extra][-j + k_width/2 + extra]);
		}
	}
}

/* You can experiment with these as well */
const static FFT_STRUCT 	FFT_3 = {8,2,6};
const static FFT_STRUCT 	FFT_5 = {16,4,12};
const static FFT_STRUCT 	FFT_7 = {16,6,10};
const static FFT_STRUCT 	FFT_11= {32,10,22};
const FFT_STRUCT * getFFT(uint32_t k_size)
{
	const FFT_STRUCT * fft;
	switch(k_size){
		case 3:
			fft = &FFT_3;
			break;
		case 5:
			fft = &FFT_5;
			break;
		case 7:
			fft = &FFT_7;
			break;
		case 11:
			fft = &FFT_11;
			break;
		default:
			printf("Kernel Size %d not supported by FFT\n",k_size);
			return NULL;
	}
	return fft;
}

const WINOGRAD_STRUCT * getWino(uint32_t k_size)
{
	const WINOGRAD_STRUCT * wino;
	switch(k_size){
		case 3:
			wino = &Wino_F2_3; // Try Wino F4_3
			break;
		case 5:
			wino = &Wino_F4_5;
			break;
		case 7:
			wino = &Wino_F4_7;
			break;
		case 11:
			wino = &Wino_F4_11;
			break;
		default:
			printf("Kernel Size %d not supported by Winograd \n",k_size);
			return NULL;
	}
	return wino;
}


/*
Question:
1. Is the output_channels the Z->size[0]? or just the W->size[0]?
*/

/*-------------------------------- Winograd -------------------------------------------*/
/* 
 * Pre Transform the Weights
 * WINOGRAD_STRUCT 	*wino 	: Struct containing tile size, A^T, G, B^T
 * Tensor 			*W		: Untransformed Weight Tensor
 * int		output_channels	: Number of output channels
 * Return:		Tensor *	: New Tensor containing transformed Weights
 */
Tensor * winoWeights(Tensor * W, int output_channels)
{
	// Note: According to the 97 line in lab3.cpp, W is one of the weight tensors, but not all of them.
	// No, above thing is not true. We should see line 145 in lab3.cpp.
	printf("W.shape: %d %d %d\n", W->size[0], W->size[1], W->size[2]);
	printf("output_channels: %d\n", output_channels);
	int k_size = W->size[1];
    const WINOGRAD_STRUCT * wino = getWino(k_size);
    if(wino == NULL)
        return NULL;
    int tile_size = wino->tile_size;

	// Each weight tensor may have multiple channels.
    Tensor * U = new Tensor (W->size[0], tile_size, tile_size);
	// for each weight channel
	for (int wc = 0; wc < W->size[0]; wc++) {
		for (int i = 0; i < tile_size; i++) {
			for (int j = 0; j < tile_size; j++) {
				for (int k = 0; k < k_size; k++) {
					for (int l = 0; l < k_size; l++) {
						(*U)[wc][i][j] += wino->G[i][k] * (*W)[wc][k][l] * wino->G[j][l];
					}
				}
			}
		}
	}
        
    return U;
	
}

/*
 * Convolution using inputs and converted Weights
 * Tensor 			*U_wino	: Transformed Weight Tensor
 * Tensor			*B		: Bias 
 * Tensor			*Z		: Output Tensor
 * int 			k_size		: Width and Height of weight kernel
 */

/*
Question: 
1. How to calculate the height and width of the output tensor? Calculate like direct convolution?
2. We only have one weight tensor, how could we get more than one output channel?
3. Is the stride = r-1? If yes, the input matrix in slide 24 is not big enough for stride = 2.
*/ 

void convWinograd(Tensor * X, Tensor * U_wino , Tensor * B, Tensor * Z, int k_size)
{
	// /*
	// Note: According to lab3.cpp:
	// for(int i =0; i < N ; i++){
	// 		convWinograd(&(X[i]),U[i],&(B[i]),&Z,W->size[2]);}
	

	// */
	// const WINOGRAD_STRUCT * wino = getWino(k_size);
    // if(wino == NULL)
    //     return;
    // int tile_size = wino->tile_size;
    // int output_channels = Z->size[0];
    // int input_channels = X->size[0];
    // int input_width = X->size[1];
    // int output_width = Z->size[1];
    // int num_tiles = output_width / (tile_size - k_size + 1);
    // // Convert the input to the winograd domain
    // Tensor * T = new Tensor (input_channels, tile_size, tile_size);
	// for (int t = 0; t < num_tiles; t++) {
	// 	for (int c = 0; c < input_channels; c++){
	// 		for (int i = 0; i < tile_size; i++) {
	// 			for (int j = 0; j < tile_size; j++) {
	// 				for (int k = 0; k < tile_size; k++) {
	// 					for (int l = 0; l < tile_size; l++) {
	// 						// (*T)[c][i][j] += wino->Bt[i][k] * (*X)[c][k+l][l];
	// 						(*T)[c][i + (tile_size - k_size +1) * t][j + (tile_size - k_size +1) * t]+= wino->Bt[i][k] * (*X)[c][k+ (tile_size - k_size +1) * t][l+ (tile_size - k_size +1) * t] * wino->Bt[j][l];
	// 					}
	// 				}
	// 			}
	// 		}
	// 	}
	// }
    // // Convolve the input tiles with the transformed weights
    // for (int t = 0; t < num_tiles; t++) {
    //     Tensor * Z_tile = new Tensor (output_channels, tile_size, tile_size);
    //     for (int c = 0; c < output_channels; c++) {
    //         for (int i = 0; i < tile_size; i++) {
    //             for (int j = 0; j < tile_size; j++) {
    //                 for (int k = 0; k < tile_size; k++) {
    //                     (*Z_tile)[c][i][j] += (*U_wino)[c][k][j] * (*T)[k][i][t*tile_size+k];
    //                 }
    //             }
    //         }
    //     }
    //     // Add bias and store the output tile
    //     for (int c = 0; c < output_channels; c++) {
    //         for (int i = 0; i < tile_size; i++) {
    //             for (int j = 0; j < tile_size; j++) {
    //                 (*Z)[c][t*tile_size+i][j] = (*Z_tile)[c][i][j] + (*B)[c][0][0];
    //             }
    //         }
    //     }
    //     delete Z_tile;
    // }
    // delete T;
}



/*-------------------------------- FFT  -----------------------------------------------*/
/*
 * Pre Transform the weights depending on the tile size
 * FFT_STRUCT 		*fft	: Struct containing tile size (N), overlap and stride
 * Tensor 			*W		: Untransformed Weight Tensor
 * int		output_channels	: Number of output channels
 * Return:		C_Tensor *	: New Tensor containing transformed Weights
 */
C_Tensor * fftWeights(Tensor * W, int output_channels)
{
}


/*
 * Convolution using inputs and converted Weights
 * FFT_STRUCT 		*fft	: Struct containing tile size (N), overlap and stride
 * C_Tensor			*U_fft:	: Complex Transformed Weight Tensor
 * Tensor			*B		: Bias 
 * Tensor			*Z		: Output Tensor
 * int 			k_size		: Width and Height of weight kernel
 */
void convFFT(Tensor * X, C_Tensor * U_fft, Tensor * B, 
		Tensor * Z, int k_size)
{

}


/*--------------------------------------- Basic ------------------------------------------*/
/* Copy your basic function in here! */
void convBasic(Tensor * X, Tensor * W ,  Tensor * b, Tensor * Z)
{
	// int output_channels = Z->size[0];
	// int input_channels = X->size[0];
	// int input_width = X->size[1];
	// int input_height = X->size[2];
	// int output_width = Z->size[1];
	// int output_height = Z->size[2];
	// int k_size = W->size[1];
	// int stride = 1;
	// int pad = 0;
	// int num_tiles = output_width / (k_size - 1);
	// for (int t = 0; t < num_tiles; t++) {
	// 	for (int c = 0; c < output_channels; c++) {
	// 		for (int i = 0; i < k_size; i++) {
	// 			for (int j = 0; j < k_size; j++) {
	// 				for (int k = 0; k < input_channels; k++) {
	// 					for (int l = 0; l < k_size; l++) {
	// 						(*Z)[c][t * (k_size - 1) + i][t * (k_size - 1) + j] += (*W)[c][k][i][j] * (*X)[k][t * (k_size - 1) + i][t * (k_size - 1) + j];
	// 					}
	// 				}
	// 			}
	// 		}
	// 		(*Z)[c][t * (k_size - 1) + i][t * (k_size - 1) + j] += (*b)[c][0][0];
	// 	}
	// }
}
