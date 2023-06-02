#include "conv.h"
#include <math.h>

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
	int k_size = W->size[1];
    const WINOGRAD_STRUCT * wino = getWino(k_size);
    if(wino == NULL)
        return NULL;
    int tile_size = wino->tile_size;
	int input_channels = W->size[0];

	// Create a new array to store the transformed weights and allocate memory for it.
	Tensor * U = new Tensor[output_channels];
	for (int i = 0; i < output_channels; i++) {
		U[i].allocate (input_channels, tile_size, tile_size);  // Use dot(.) rather than arrow(->) because U[i] is a tensor, not a pointer.
	}
	// for each weight tensor
	for (int oc = 0; oc < output_channels; oc++) {
		// for each weight channel
		for (int wc = 0; wc < input_channels; wc++) {
			// for each row in the converted weight channel
			for (int i = 0; i < tile_size; i++) {
				// for each column in the converted weight channel
				for (int j = 0; j < tile_size; j++) {
					// for each row in the kernel
					for (int k = 0; k < k_size; k++) {
						// for each column in the kernel
						for (int l = 0; l < k_size; l++) {
							U[oc].data[wc][i][j] += wino->G[i][k] * W[oc].data[wc][k][l] * wino->G[j][l];
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
    // print *B.shape
    printf("U_wino.shape: %d %d %d\n", U_wino->size[0], U_wino->size[1], U_wino->size[2]);
    // printf("B.shape: %d %d %d\n", B->size[0], B->size[1], B->size[2]); // shape = (1,1,output_channel). It seems that for each output feature map, there is only one number as bias.
	printf("Z.size= %d, %d, %d\n",Z->size[0],Z->size[1],Z->size[2]);
    const WINOGRAD_STRUCT * wino = getWino(k_size);
    if(wino == NULL)
        return;
    int tile_size = wino->tile_size;
    int output_channels = Z->size[0];
    int input_channels = X->size[0];
    int input_width = X->size[2];
    int output_width = Z->size[2];
    int tile_stride = wino->tile_stride;
    int num_tiles = ceil(1.0*(input_width - tile_size)/tile_stride + 1); // each row or col
    printf("num_tiles: %d\n", num_tiles);


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
    Tensor * T = new Tensor (input_channels, tile_size * num_tiles, tile_size * num_tiles);
    // If the input size is not divisible by the tile size, we need to pad the input with zeros
    if (num_tiles != (input_width - tile_size)/tile_stride + 1){
        for (int c = 0; c < input_channels; c++) {
            for (int t_row = 0; t_row < num_tiles; t_row++){
                for (int t_col = 0; t_col < num_tiles; t_col++){
                    for (int i = 0; i < tile_size; i++) {
                        for (int j = 0; j < tile_size; j++) {
                            for (int k = 0; k < tile_size; k++) {
                                for (int l = 0; l < tile_size; l++) {
                                    if (k+ (tile_size - k_size +1) * t_row < input_width && l+ (tile_size - k_size +1) * t_col < input_width){
                                        T->data[c][i + tile_size * t_row][j + tile_size * t_col]+= wino->Bt[i][k] * X->data[c][k+ (tile_size - k_size +1) * t_row][l+ (tile_size - k_size +1) * t_col] * wino->Bt[j][l];
                                    }
                                    else {
                                        T->data[c][i + tile_size * t_row][j + tile_size * t_col]+= wino->Bt[i][k] * 0 * wino->Bt[j][l];
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
                    for (int i = 0; i < tile_size; i++) {
                        for (int j = 0; j < tile_size; j++) {
                            for (int k = 0; k < tile_size; k++) {
                                for (int l = 0; l < tile_size; l++) {
                                    T->data[c][i + tile_size * t_row][j + tile_size * t_col]+= wino->Bt[i][k] * X->data[c][k+ (tile_size - k_size +1) * t_row][l+ (tile_size - k_size +1) * t_col] * wino->Bt[j][l];
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
                Tensor * m = new Tensor (1, tile_size, tile_size);
                // for each input channel, add up
                for (int ic = 0; ic < input_channels; ic++){
                    // for each row in tile
                    for (int i = 0; i < tile_size; i++){
                        // for each col in tile
                        for (int j = 0; j < tile_size; j++){
                            // eliment-wise multi, Bias
                            // printf("i: %d, j: %d, t_row: %d, t_col: %d, w: %d, ic: %d\n", i, j, t_row, t_col, w, ic);
                            m->data[0][i][j] += T->data[ic][i + tile_size * t_row][j + tile_size * t_col] * U_wino[w].data[ic][i][j];
                        }
                    }
                }
                // convert m from winograd domain back, add the bias, and then allocate to Z
                for (int i = 0; i < wino->out_size; i++){
                    for (int j = 0; j < wino->out_size; j++){
                        if (i + wino->out_size * t_row < z_height && j + wino->out_size * t_col < z_width){
                            for (int k =0; k < tile_size; k++){
                                for (int l = 0; l < tile_size; l++){
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

    int k_size = W->size[1];
    const FFT_STRUCT * fft = getFFT(k_size);
    if(fft == NULL)
        return NULL;
    int tile_size = fft->tile_size;
    int pad_size = tile_size - k_size;
    int input_channels = W[0].size[0];

    // First flip the weight tensors
    // W_flipped contains all flipped weight tensors
    C_Tensor * W_flipped = new C_Tensor[output_channels];
    for (int i = 0; i < output_channels; i++) {
        W_flipped[i].allocate(input_channels, k_size, k_size);
        for (int j = 0; j < input_channels; j++) {
            // temp stores the one channel flipped weight tensor
            C_Tensor * temp = new C_Tensor(1, k_size, k_size);
            flip_Matrix(&W[i], temp, j);
            // copy the flipped weight tensor to W_flipped
            for (int k = 0; k < k_size; k++) {
                for (int l = 0; l < k_size; l++) {
                    W_flipped[i].data[j][k][l] = temp->data[0][k][l];
                }
            }
            delete temp;
        }
    }

    /*///////////////////////////////////////////////////////////////////
    For test, DELETE!!!
    */
//    //print out the original weight tensor with a good visualization
//     for (int i = 0; i < output_channels; i++) {
//         for (int j = 0; j < input_channels; j++) {
//             printf("W_original[%d][%d]:\n", i, j);
//             for (int k = 0; k < k_size; k++) {
//                 for (int l = 0; l < k_size; l++) {
//                     printf("%f ", W[i].data[j][k][l]);
//                 }
//                 printf("\n");
//             }
//             printf("\n");

//             printf("W_flipped[%d][%d]:\n", i, j);
//             for (int k = 0; k < k_size; k++) {
//                 for (int l = 0; l < k_size; l++) {
//                     printf("%f ", W_flipped[i].data[j][k][l].real());
//                 }
//                 printf("\n");
//             }
//             printf("\n");
//         }
//     }

//     return W_flipped;

   ///////////////////////////////////////////////////////////////////////


    // Pad the weight matrix with zeros to size "tile_size * tile_size"
    C_Tensor * W_padded = new C_Tensor[output_channels];
    for (int i = 0; i < output_channels; i++) {
        W_padded[i].allocate(input_channels, tile_size, tile_size);
        for (int j = 0; j < input_channels; j++) {
            for (int k = 0; k < tile_size; k++) {
                for (int l = 0; l < tile_size; l++) {
                    if (k < k_size && l < k_size)
                        W_padded[i].data[j][k][l] = W_flipped[i].data[j][k][l];
                    else
                        W_padded[i].data[j][k][l] = 0;
                }
            }
        }
    }

    // Perform 2D-FFT on the padded weight matrix
    C_Tensor * U_fft = new C_Tensor[output_channels];
    for (int i = 0; i < output_channels; i++) {
        U_fft[i].allocate(input_channels, tile_size, tile_size);
        for (int j = 0; j < input_channels; j++) {
            fft2d(&W_padded[i], &U_fft[i]);
        }
    }

    delete [] W_padded;
    return U_fft;
}


/*
 * Convolution using inputs and converted Weights
 * FFT_STRUCT 		*fft	: Struct containing tile size (N), overlap and stride
 * C_Tensor			*U_fft:	: Complex Transformed Weight Tensor
 * Tensor			*B		: Bias 
 * Tensor			*Z		: Output Tensor
 * int 			k_size		: Width and Height of weight kernel
 */
//conv.cpp

void convFFT(Tensor * X, C_Tensor * U_fft, Tensor * B, Tensor * Z, int k_size)
{
    const FFT_STRUCT * fft = getFFT(k_size);
    if(fft == NULL)
        return;
    int tile_size = fft->tile_size;
    int overlap = fft->overlap;
    int stride = tile_size - overlap;
    int output_channels = Z->size[0];
    int input_channels = X->size[0];
    int input_width = X->size[2];
    int output_width = Z->size[2];
    int num_tiles = ceil(1.0*(input_width - tile_size)/stride + 1);

    // Loop through output feature map
    for (int w = 0; w < output_channels; w++){
        // Loop through input feature map with Stride N − (M − 1)
        for (int t_row = 0; t_row < num_tiles; t_row++) {
            for (int t_col = 0; t_col < num_tiles; t_col++) {
                // Convert input tile (N × N ) 
                // T_fft contains one transformed input tile (all channels)
                C_Tensor * T_fft = new C_Tensor(input_channels, tile_size, tile_size);
                for (int c = 0; c < input_channels; c++) {
                    // temp_in is the untransformed, padded input tile (1 channel)
                    C_Tensor * temp_in = new C_Tensor(1, tile_size, tile_size);
                    for (int i = 0; i < tile_size; i++) {
                        for (int j = 0; j < tile_size; j++) {
                            // For boundary cases. If the input size is not divisible by the tile size. Pad the input tile with zeros
                            if (i + stride * t_row < input_width && j + stride * t_col < input_width) {
                                temp_in->data[0][i][j] = X->data[c][i + stride * t_row][j + stride * t_col];
                            }
                            else {
                                temp_in->data[0][i][j] = 0;
                            }
                        }
                    }
                    C_Tensor * temp_out = new C_Tensor(1, tile_size, tile_size);
                    fft2d(temp_in, temp_out);
                    
                    // Copy the transformed input tile (temp_out) to the transformed input tensor (T_fft)
                    for (int i = 0; i < tile_size; i++) {
                        for (int j = 0; j < tile_size; j++) {
                            T_fft->data[c][i][j] = temp_out->data[0][i][j];
                        }
                    }
                    delete temp_in;
                    delete temp_out;
                }

                // Perform element-wise multiplication with converted weight
                // Add up tiles from different input channels in frequency domain, and store the result in m_fft
                C_Tensor * m_fft = new C_Tensor(1, tile_size, tile_size);
                for (int w = 0; w < output_channels; w++) {
                    for (int i = 0; i < tile_size; i++) {
                        for (int j = 0; j < tile_size; j++) {
                            for (int c = 0; c < input_channels; c++) {
                                m_fft->data[0][i][j] += T_fft->data[c][i][j] * U_fft[w].data[c][i][j];
                            }
                        }
                    }
                }

                // Perform inverse 2D-FFT on output tile and store N − (M − 1) values in the output matrix
                Tensor * m = new Tensor(1, tile_size, tile_size);
                ifft2d(m_fft, m);
                for (int i = 0; i < tile_size - overlap; i++) {
                    for (int j = 0; j < tile_size - overlap; j++) {
                        if (i + stride * t_row < output_width && j + stride * t_col < output_width) {
                            Z->data[0][i + stride * t_row][j + stride * t_col] += m->data[0][i][j] + B->data[0][0][0];
                        }
                    }
                }

                delete T_fft;
                delete m_fft;
                delete m;
            }
        }
    }
    
}
  


/*--------------------------------------- Basic ------------------------------------------*/
/* Copy your basic function in here! */
void convBasic(Tensor * X, Tensor * W ,  Tensor * b, Tensor * Z)
{
	// printf("W.size= %d, %d, %d, %d \n",W->size[0],W[0].size[0],W[0].size[1],W[0].size[2]);
    printf("number of weight tensors = %d, number of output channel = %d \n",W->size[0], Z->size[0]);
    // printf("X.size= %d, %d, %d\n",X->size[0],X->size[1],X->size[2]);
    // printf("Z.size= %d, %d, %d\n",Z->size[0],Z->size[1],Z->size[2]);
    printf("b.size= %d, %d, %d\n",b->size[0],b->size[1],b->size[2]);

    // printf("----------\n");

    uint32_t z_width = Z->size[2];
    uint32_t z_height = Z->size[1];
    uint32_t z_channel = Z->size[0];

    uint32_t xc = X->size[0];
    // uint32_t wm = W[0].size[1];
    // uint32_t wn = W[0].size[2];

    // initialize output tensor to zero
    for (uint32_t i = 0; i < z_channel; i++) {
        for (uint32_t j = 0; j < z_height; j++) {
            for (uint32_t k = 0; k < z_width; k++) {
                Z->data[i][j][k] = 0;
            }
        }
    }

    for (uint32_t i=0; i<z_channel; i++)
        {
            
            for (uint32_t j=0; j<z_height; j++)
            {
                // printf("j=%d \n",j);
                for (uint32_t k=0; k<z_width; k++)
                {
                    // printf("k=%d \n",k);
                    for (uint32_t c=0; c<xc; c++)
                    {
                        for (uint32_t p=0; p<W[i].size[1]; p++)
                        {
                            for (uint32_t q=0; q<W[i].size[2]; q++)
                            {
                                // printf("k=%d \n",k);
                                Z->data[i][j][k] += X->data[c][j+p][k+q] * W[i].data[c][p][q];
                                // Z->data[i][j][k] +=1;
                            }
                        }
                    }
                    Z->data[i][j][k] += b->data[0][0][i]; 
                }
                
            }
        }
}
