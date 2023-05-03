#include "kernels.h"
#include <vector>
#include <iostream>
#include <cmath>
/* 
 * Applies a 2d convolution on a 3D X using W: Z= W (conv) X + b
 * Tensor * X:		Input Tensor
 * Tensor * W:		Array of N weight Tensors (N == Z.size[0]) 
 * Tensor * Z:		Output Tensor 
 * Tensor * b:		Bias 
 */
void conv2d(Tensor * X, Tensor * W ,  Tensor * b, Tensor * Z)
{

}

/*
 * Applies a max pool layer on X (size = stride = 2)
 * Tensor * X:	input Tensor
 * Tensor * Z:	output Tensor
 */
void maxPool(Tensor * X, Tensor * Z)
{
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
        Z->data[0][0][i] = X->data[0][0][i] / mysum;
    }
}
