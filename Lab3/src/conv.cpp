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
    // printf("U_wino.shape: %d %d %d\n", U_wino->size[0], U_wino->size[1], U_wino->size[2]);
    // printf("B.shape: %d %d %d\n", B->size[0], B->size[1], B->size[2]); // shape = (1,1,output_channel). It seems that for each output feature map, there is only one number as bias.
	// printf("Z.size= %d, %d, %d\n",Z->size[0],Z->size[1],Z->size[2]);
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
    
//    //print out the original weight tensor with a good visualization
    // for (int i = 0; i < output_channels; i++) {
    //     for (int j = 0; j < input_channels; j++) {
            // printf("W_original[%d][%d]:\n", i, j);
            // for (int k = 0; k < k_size; k++) {
            //     for (int l = 0; l < k_size; l++) {
            //         printf("%f ", W[i].data[j][k][l]);
            //     }
            //     printf("\n");
            // }
            // printf("\n");

            // printf("W_flipped[%d][%d]:\n", i, j);
    //         for (int k = 0; k < k_size; k++) {
    //             for (int l = 0; l < k_size; l++) {
    //                 printf("%f ", W_flipped[0].data[0][k][l].real());
    //             }
    //             printf("\n");
    //     //     }
    //     //     printf("\n");
    //     // }
    // }

    // // print out the W_flipped tensor
    // printf("W_flipped.shape: %d, %d, %d\n", W_flipped[0].size[0], W_flipped[0].size[1], W_flipped[0].size[2]);
    // for (int k = 0; k < k_size; k++) {
    //     for (int l = 0; l < k_size; l++) {
    //         printf("%f + i %f, ", W_flipped[0].data[0][k][l].real(), W_flipped[0].data[0][k][l].imag());
    //     }
    //     printf("\n");
    // }

    // // calculate the FFT of the flipped weight tensor
    // C_Tensor * W_fft = new C_Tensor(1,k_size,k_size);
    // fft2d(W_flipped, W_fft);
    // // print out the FFT of the flipped weight tensor
    // printf("W_fft:\n");
    // for (int k = 0; k < k_size; k++) {
    //     for (int l = 0; l < k_size; l++) {
    //         printf("%f + i %f, ", W_fft[0].data[0][k][l].real(), W_fft[0].data[0][k][l].imag());
    //     }
    //     printf("\n");
    // }



    // return W_flipped;
    */
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

    delete [] W_flipped;

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
    
    // temp is the padded input tile
    C_Tensor * temp = new C_Tensor(input_channels, tile_size, tile_size); 
    // T_fft contains one transformed input tile (all channels)
    C_Tensor * T_fft = new C_Tensor(input_channels, tile_size, tile_size);
    // m contains the time domain output tile. Discard the first M-1 of m.
    C_Tensor * m = new C_Tensor(output_channels, tile_size, tile_size);
    
    // Loop through input feature map with Stride N − (M − 1)
    for (int t_row = 0; t_row < num_tiles; t_row++) {
        for (int t_col = 0; t_col < num_tiles; t_col++) {
            // Loop through output feature map
            C_Tensor * m_fft = new C_Tensor(output_channels, tile_size, tile_size);
            
            // Convert input tile (N × N ) 
            for (int c = 0; c < input_channels; c++) {
                for (int i = 0; i < tile_size; i++) {
                    for (int j = 0; j < tile_size; j++) {
                        // For boundary cases. If the input size is not divisible by the tile size. Pad the input tile with zeros
                        if (i + stride * t_row < input_width && j + stride * t_col < input_width) {
                            temp->data[c][i][j] = X->data[c][i + stride * t_row][j + stride * t_col];
                        }
                        else {
                            temp->data[c][i][j] = 0;
                        }
                    }
                }
            }
            // Perform 2D-FFT on input tile
            fft2d(temp, T_fft);
                        
            // Perform element-wise multiplication with converted weight
            // Add up tiles from different input channels in frequency domain, and store the result in m_fft
            // m_fft is a temporary tensor, it will be replaced for every weight tensor 
            for (int w = 0; w < output_channels; w++){              
                for (int i = 0; i < tile_size; i++) {
                    for (int j = 0; j < tile_size; j++) {
                        for (int c = 0; c < input_channels; c++) {
                            m_fft->data[w][i][j] += T_fft->data[c][i][j] * U_fft[w].data[c][i][j];  // "+=" is for adding up tiles from different input channels
                        }
                    }
                }
            }
                
            // Perform inverse 2D-FFT on output tile            
            ifft2d(m_fft, m);
            // Discard the first M-1, and store N − (M − 1) values in the output matrix
            for (int w = 0; w < output_channels; w++) {
                for (int i = 0; i < tile_size - overlap; i++) {
                    for (int j = 0; j < tile_size - overlap; j++) {
                        // printf("w = %d, i = %d, j = %d, t_row = %d, t_col = %d, i+overlap = %d, j+overlap = %d, tile_size = %d, stride = %d, overlap = %d, Z.shape = %d, %d, %d\n", w, i, j, t_row, t_col, i + overlap, j + overlap, tile_size, stride, overlap, Z->size[0], Z->size[1], Z->size[2]);
                        if (i + t_row * stride < output_width && j + t_col * stride < output_width){
                            Z->data[w][i + t_row * stride][j + t_col * stride] = m->data[w][i + overlap][j + overlap].real() + B->data[0][0][w];
                        }                       
                    }
                }
            }
            delete m_fft;          
        }
    }
    
    delete temp;
    delete T_fft;
    delete m;
    
}
    
    /*
    // ////////////////////////////////////////////////
    // // //         Test fft and ifft
    // ////////////////////////////////////////////////

    // int N = 8;
    // // Create a C_FLOAT array containing 1,2,3,4,5,6, 7 ,8 to test fft and ifft
    // C_FLOAT * arr1d = new C_FLOAT[N]{1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0};
    // // apply fft to arr1d
    // C_FLOAT * arr1d_fft = new C_FLOAT[N];

    // myfft(arr1d, arr1d_fft, N);
    // printf("arr1d_fft: ");
    // for (int i = 0; i < N; i++){
    //     printf("%f+j%f, ", arr1d_fft[i].real(), arr1d_fft[i].imag());
    // }
    // printf("\n");


    // ifft(arr1d_fft, arr1d, N);
    // printf("arr1d_ifft: ");
    // for (int i = 0; i < N; i++){
    //     printf("%f+j%f, ", arr1d[i].real(), arr1d[i].imag());
    // }
    // printf("\n");
    // printf("\n");


    // delete [] arr1d;
    // delete [] arr1d_fft;

    // ////////////////////////////////////////////////
    // // //       Test fft2d and ifft2d
    // ////////////////////////////////////////////////

    // // Creating a 2 by 2 2D array
    // C_Tensor * arr = new C_Tensor(1,4,4);
    // // Assigning values to the array
    // arr->data[0][0][0] = 1.0;
    // arr->data[0][0][1] = 2.0;
    // arr->data[0][0][2] = 3.0;
    // arr->data[0][0][3] = 4.0;
    // arr->data[0][1][0] = 5.0;
    // arr->data[0][1][1] = 6.0;
    // arr->data[0][1][2] = 7.0;
    // arr->data[0][1][3] = 8.0;
    // arr->data[0][2][0] = 9.0;
    // arr->data[0][2][1] = 10.0;
    // arr->data[0][2][2] = 11.0;
    // arr->data[0][2][3] = 12.0;
    // arr->data[0][3][0] = 13.0;
    // arr->data[0][3][1] = 14.0;
    // arr->data[0][3][2] = 15.0;
    // arr->data[0][3][3] = 16.0;


    // // apply fft2d to arr
    // C_Tensor * arr_fft = new C_Tensor(1,4,4);
    // fft2d(arr, arr_fft);

    // // visualize the arr_fft
    // printf("arr_fft2d: \n");
    // for (int i = 0; i < 4; i++){
    //     for (int j = 0; j < 4; j++) {
    //         printf("%f+i%f, ",arr_fft->data[0][i][j].real(), arr_fft->data[0][i][j].imag());
    //     }
    //     printf("\n");
    // }
    // printf("\n");

    // // apply ifft2d to arr_fft
    // C_Tensor * arr_ifft = new C_Tensor(1,4,4);
    // ifft2d(arr_fft, arr_ifft);

    // // visualize the arr_ifft
    // printf("arr_ifft2d: \n");
    // for (int i = 0; i < 4; i++){
    //     for (int j = 0; j < 4; j++) {
    //         printf("%f+i%f, ",arr_ifft->data[0][i][j].real(), arr_ifft->data[0][i][j].imag());
    //     }
    //     printf("\n");
    // }
    // printf("\n");

    // delete arr;
    // delete arr_fft;
    // delete arr_ifft;
    */


  


/*--------------------------------------- Basic ------------------------------------------*/
/* 
 * Applies a basic/naive 2d convolution on a 3D X using W: Z= W (conv) X + b
 * Tensor * X:		Input Tensor
 * Tensor * W:		Array of N weight Tensors (N == Z.size[0]) 
 * Tensor * Z:		Output Tensor 
 * Tensor * b:		Bias 
 */
void convBasic(Tensor * X, Tensor * W ,  Tensor * b, Tensor * Z)
{
	// printf("%d %d %d\n", X->size[0], X->size[1], X->size[2]);
	// printf("%d %d %d\n", W->size[0], W->size[1], W->size[2]);
	// printf("%d %d %d\n", b->size[0], b->size[1], b->size[2]);
	// printf("%d %d %d\n", Z->size[0], Z->size[1], Z->size[2]);
	int in_chan = X->size[0];
	int out_chan = Z->size[0];
	int wx = X->size[1];
	int wy = X->size[2];
	for (int ochan = 0; ochan < out_chan; ochan++) {
    	int weight_wx = W[ochan].size[1];
    	int weight_wy = W[ochan].size[2];
    	for (int x = 0; x < wx - weight_wx + 1; x++) {
    		for (int y = 0; y < wy - weight_wy + 1; y++) {
        		FLOAT sum = 0;
           		for (int ichan = 0; ichan < in_chan; ichan++) {
        			for (int ix = 0; ix < weight_wx; ix++) {
        				for (int iy = 0; iy < weight_wy; iy++) {
        					sum += X->data[ichan][x+ix][y+iy] * 
        					  W[ochan].data[ichan][ix][iy];
        				}
        			}
           		}
    			Z->data[ochan][x][y] = sum + b->data[0][0][ochan];
    		}
    	}
	}

}

