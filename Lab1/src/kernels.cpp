#include "kernels.h"
#include <vector>
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
    X->resize(1,1,flatten_size);

    uint32_t output_size = Z->size[0] * Z->size[1] * Z->size[2];
    for (uint32_t i=0; i<output_size; i++)
    {
        for (uint32_t j=0; j<flatten_size; j++)
        {
            Z[0][0][i] += X[0][0][j] * W[0][j][i];
        }
    }
}


/*
 * Applies the ReLU activation function: Z = ReLU(X)
 * Tensor * X: input Tensor
 * Tensor * Z: output Tensor
 */
void ReLU(Tensor * X , Tensor * Z)
{
}

/*
 * Applies the Softmax activation function z = exp(x_i)/sum(exp(x_j))
 * This is a stable Softmax implementation
 * Tensor * X: input vector in Tensor form
 * Tensor * Z: output vector in Tensor form
 */
void Softmax(Tensor * X, Tensor * Z)
{

}
