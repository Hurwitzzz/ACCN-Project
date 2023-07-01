#include "kernels.h"

// Convolution implemented in conv

/*
 * Applies a max pool layer on X (size = stride = 2)
 * Tensor * X:	input Tensor
 * Tensor * Z:	output Tensor
 */
void maxPool(Tensor * X, Tensor * Z)
{
	// printf("%d %d %d\n", X->size[0], X->size[1], X->size[2]);
	// printf("%d %d %d\n", Z->size[0], Z->size[1], Z->size[2]);
	int num_chan = X->size[0];
	int wx = X->size[1];
	int wy = X->size[2];
	for (int chan = 0; chan < num_chan; chan++) {
		for (int x = 0; x < wx; x += 2) {
			for (int y = 0; y < wy; y += 2) {
			FLOAT max = 0; //X->data[chan][x][y] and then skip that one in the loop
				for (int ix = 0; ix < 2/* && x + ix < wx*/; ix++) {
					for (int iy = 0; iy < 2/* && y + iy < wy*/; iy++) {
						FLOAT curr = X->data[chan][x+ix][y+iy];
						if (curr > max) {
							max = curr;
						}
					}
				}
			Z->data[chan][x / 2][y / 2] = max;
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
	// printf("%d %d %d\n", X->size[0], X->size[1], X->size[2]);
	// printf("%d %d %d\n", W->size[0], W->size[1], W->size[2]);
	// printf("%d %d %d\n", B->size[0], B->size[1], B->size[2]);
	// printf("%d %d %d\n", Z->size[0], Z->size[1], Z->size[2]);
	if(X->size[0] > 1) { //reshape into 1, 1, _
		X->size[2] = X->size[1] * X->size[2] * X->size[0];
		X->size[1] = 1;
		X->size[0] = 1; // channel to 1
	}
	int wx = X->size[1];
	int wy = X->size[2];
	int wwx = W->size[1];
	for (int x = 0; x < wx; x++) {
		for (int y = 0; y < wwx; y++) {
			FLOAT sum = 0;
			for (int i = 0; i < wy; i++) {
				sum += X->data[0][x][i] * W->data[0][y][i];
			}
			Z->data[0][x][y] = sum + B->data[0][x][y];
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
	// printf("%d %d %d\n", X->size[0], X->size[1], X->size[2]);
	// printf("%d %d %d\n", Z->size[0], Z->size[1], Z->size[2]);
    uint32_t input_size = X->size[0] * X->size[1] * X->size[2];
    for (uint32_t i=0; i<input_size; i++) {
		FLOAT max = X->data[0][0][i];
		if(max < 0) {
			Z->data[0][0][i] = 0;
		} else {
			Z->data[0][0][i] = max;
		}
	}
}

/*
 * Applies the ReLU activation function inplace: X = ReLU(X)
 * Tensor * X: in/output Tensor
 */
void ReLUInplace(Tensor * X)
{
	// printf("%d %d %d\n", X->size[0], X->size[1], X->size[2]);
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
	// printf("%d %d %d\n", X->size[0], X->size[1], X->size[2]);
	// printf("%d %d %d\n", Z->size[0], Z->size[1], Z->size[2]);
	int num_chan = X->size[0];
	int wx = X->size[1];
	int wy = X->size[2];
	for (int chan = 0; chan < num_chan; chan++) {
		for (int x = 0; x < wx; x++) {
    		FLOAT sum = 0;
			for (int y = 0; y < wy; y++) {
				sum += exp(X->data[chan][x][y]);
			}
			for (int y = 0; y < wy; y++) {
    			FLOAT max = exp(X->data[chan][x][y]) / sum;
				Z->data[chan][x][y] = max;
			}
		}
	}
}
