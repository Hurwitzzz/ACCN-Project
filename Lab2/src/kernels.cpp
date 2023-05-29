#include "kernels.h"
#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>

/* 
 * Applies a 2d convolution on a 3D X using W: Z= W (conv) X + b
 * Tensor * X:		Input Tensor
 * Tensor * W:		Array of N weight Tensors (N == Z.size[0]) 
 * Tensor * Z:		Output Tensor 
 * Tensor * b:		Bias 
 */
void conv2d(Tensor * X, Tensor * W ,  Tensor * b, Tensor * Z)
{
    // printf("W.size= %d, %d, %d, %d \n",W->size[0],W[0].size[0],W[0].size[1],W[0].size[2]);
    printf("number of weight tensors = %d, number of output channel = %d \n",W->size[0], Z->size[0]);
    // printf("X.size= %d, %d, %d\n",X->size[0],X->size[1],X->size[2]);
    // printf("Z.size= %d, %d, %d\n",Z->size[0],Z->size[1],Z->size[2]);
    // printf("b.size= %d, %d, %d\n",b->size[0],b->size[1],b->size[2]);

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
                    Z->data[i][j][k] += b->data[0][0][i]; // Why not set b.size = channel, 1, 1 ?
                }
                
            }
        }
}


/*
 * Applies a max pool layer on X (size = stride = 2)
 * Tensor * X:	input Tensor
 * Tensor * Z:	output Tensor
 */
void maxPool(Tensor * X, Tensor * Z)   // How about the odd width or height case?
{   
    uint32_t z_width = Z->size[2];
    uint32_t z_height = Z->size[1];
    uint32_t z_channel = Z->size[0];
    

    for (uint32_t k=0; k<z_channel; k++)
    {
        for (uint32_t i=0; i<z_height; i++)
        {
            for (uint32_t j=0; j<z_width; j++)
            {
                Z->data[k][i][j] = std::max({
                    X->data[k][2*i][2*j], X->data[k][2*i][2*j+1],
                    X->data[k][2*i+1][2*j], X->data[k][2*i+1][2*j+1]});
            }
        }
    }
    
}


/*
 * Applies a Linear layer: z = Wx + b 
 * Flatten the input if required 
 * Tensor *	X: input Tensor
 * Tensor *	W: weight Matrix (in Tensor form)
 * Tensor *	B: bias array (in Tensor form)
 * Tensor *	Z: output array (in Tensor form)
 */

void Linear(Tensor * X, Tensor * W, Tensor * B, Tensor * Z)
{
    uint32_t flatten_size = X->size[0] * X->size[1] * X->size[2];
    // X->resize(1,1,flatten_size);   //What is the sence of te resize()?? It makes all data to ZERO!! 
    uint32_t output_size = Z->size[0] * Z->size[1] * Z->size[2];

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

    for (uint32_t i=0; i<output_size; i++)
    {
        for (uint32_t j=0; j<flatten_size; j++)
        {
            Z->data[0][0][i] += W->data[0][i][j] * X->data[0][0][j];
            // Z[0][0][i] += W[0][i][j] * X[0][0][j];  // Why can't I implement it in such way.
        }
        Z->data[0][0][i] += B->data[0][0][i];
    }
}


/*
 * Applies the ReLU activation function: Z = ReLU(X)
 * Tensor * X: input Tensor
 * Tensor * Z: output Tensor
 */
void ReLU(Tensor * X , Tensor * Z)
{
    uint32_t input_size = X->size[0] * X->size[1] * X->size[2];
    uint32_t output_size = Z->size[0] * Z->size[1] * Z->size[2];
    if (input_size != output_size)
    {
        printf("The input size and output size do not match");
    }
    for (uint32_t i=0; i<input_size; i++)
    {
        Z->data[0][0][i] = std::max(0.0f, X->data[0][0][i]);
    }

}

void ReLu_inplace(Tensor * X)
{
    uint32_t input_size = X->size[0] * X->size[1] * X->size[2];
    for (uint32_t i=0; i<input_size; i++)
    {
        X->data[0][0][i] = std::max(0.0f, X->data[0][0][i]);
    }
}

/*
 * Applies the Softmax activation function z = exp(x_i)/sum(exp(x_j))
 * This is a stable Softmax implementation
 * Tensor * X: input vector in Tensor form
 * Tensor * Z: output vector in Tensor form
 */
void Softmax(Tensor * X, Tensor * Z)
{
    uint32_t input_size = X->size[0] * X->size[1] * X->size[2];
    uint32_t output_size = Z->size[0] * Z->size[1] * Z->size[2];

    if (input_size != output_size)
    {
        printf("The input size and output size do not match");
    }
    // calculate the sum(exp(x_j))
    float mysum = 0;
    for (uint32_t i=0; i<input_size; i++)
    {
        mysum += exp(X->data[0][0][i]);  // Why can't I use X[0][0][i] here? We've redefined the operator.
    }
    for (uint32_t i=0; i<input_size; i++)
    {
        Z->data[0][0][i] = exp(X->data[0][0][i]) / mysum;
    }
}
