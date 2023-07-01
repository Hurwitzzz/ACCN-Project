#ifndef CNN_H
#define CNN_H


#define LAYER_TYPE_FC 		0
#define LAYER_TYPE_POOL		1
#define LAYER_TYPE_RELU		2
#define LAYER_TYPE_CONV		3
#define LAYER_TYPE_SOFTMAX	4


#define CNN_RETURN_SUCCESS 0
#define CNN_RETURN_FAILED 1

#include "tensor.h"
#include "kernels.h"
#include "stdint.h"
#include <vector>
#include <cmath>

namespace ml{

enum struct Layer_Type : uint32_t {Linear = 0,Pool = 1 ,ReLU = 2,Conv = 3,Softmax = 4};
enum Optimization {None = 0, FFT = 1, Wino = 2, Auto = 3};

typedef struct{
	Layer_Type type;
	uint32_t output_size[3];
	uint32_t kernel_width;
	uint32_t input_channels;
	uint32_t pad;
	bool in_place = false;
	Optimization optimized = Optimization::None; // Stores which optimization will be used for convolutional layers. Default is None, not Auto!
	Tensor * Z;
	union {
		Tensor * W;
		C_Tensor * W_fft; // For convinience when storing the transformed weights for fft
	};
	Tensor * B;
} CNN_layer_struct;


class CNN{
	public:
	double runtime[5] = {0};
	std::vector<CNN_layer_struct> layers;
	CNN(std::vector<CNN_layer_struct> in_layers);
	~CNN();
	int read(const char * infile);
	Tensor * inference(Tensor * input);
    void prepare(Optimization optim = Optimization::Auto);
	void print_timing();
};

/* ----------------------------- Functions to initialize Layers -----------------------------*/

CNN_layer_struct LinearLayer(uint32_t outputs);

CNN_layer_struct PoolLayer(uint32_t output_channels, uint32_t output_height, uint32_t output_width);

CNN_layer_struct ConvLayer(uint32_t input_channels, uint32_t output_channels, uint32_t output_height, 
		uint32_t output_width, uint32_t kernel_width, uint32_t pad);

CNN_layer_struct ReLULayer(bool in_place = true);

CNN_layer_struct SoftmaxLayer();

/* Decides which convolution method to use */
Optimization autoDecide(uint32_t output_size[3], uint32_t kernel_width, uint32_t input_channels);

}


#endif
