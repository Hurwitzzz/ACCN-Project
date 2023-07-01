#include "cnn.h"
#include "kernels.h"

namespace ml {

CNN::CNN(std::vector<CNN_layer_struct> in_layers)
{
	uint32_t insize = 0;
	layers = in_layers;
	// The tricky part is allocating the proper tensors
	for(int i = 0; i < layers.size(); i++){
		CNN_layer_struct & lay = layers[i];
		// lay->X is the input lay->Z is the output
		switch(lay.type){
			case Layer_Type::ReLU: case Layer_Type::Softmax:
				lay.output_size[0] = layers[i - 1].output_size[0];
				lay.output_size[1] = layers[i - 1].output_size[1];
				lay.output_size[2] = layers[i - 1].output_size[2];
				if(lay.in_place)
					lay.Z = layers[i - 1].Z;
				else
					lay.Z = new Tensor(layers[i-1].output_size[0],lay.output_size[1],lay.output_size[2]);
				break;
			case Layer_Type::Pool:
				lay.Z = new Tensor(lay.output_size[0],lay.output_size[1],lay.output_size[2]);
				break;
			case Layer_Type::Conv:
				lay.Z = new Tensor(lay.output_size[0],lay.output_size[1],lay.output_size[2]);
				lay.W = new Tensor[lay.output_size[0]]();
				for(int i =0 ; i < lay.output_size[0]; i++){
					lay.W[i].allocate(lay.input_channels,lay.kernel_width,lay.kernel_width);
				}
				lay.B = new Tensor(1,1,lay.output_size[0]);
				break;
			case Layer_Type::Linear:
				insize = layers[i-1].output_size[0] * layers[i-1].output_size[1] * layers[i-1].output_size[2];
				lay.Z = new Tensor(1,1,lay.output_size[2]);
				lay.W = new Tensor(1,lay.output_size[2],insize);
				lay.B = new Tensor(1,1,lay.output_size[2]);
				break;
			default:
				throw std::runtime_error("Layer not implemented !\n");
		}
	}
}



CNN::~CNN()
{
	for(int i = 0; i < layers.size(); i++){
		CNN_layer_struct & lay = layers[i];
		// lay->X is the input lay->Z is the output
		switch(lay.type){
			case Layer_Type::ReLU:
				if(!(lay.in_place))
					delete lay.Z;
				break;
			case Layer_Type::Softmax:
			case Layer_Type::Pool:
				delete lay.Z;
				break;
			case Layer_Type::Conv:
				delete lay.Z;
				if(lay.optimized == FFT) {
    				delete [] lay.W_fft;
				} else {
    				delete [] lay.W;
				}
				delete lay.B;
				break;
			case Layer_Type::Linear:
				delete lay.Z;
				delete lay.W;
				delete lay.B;
				break;
			default:
				printf("Rogue unimplemented layer found during deallocation !\n");
		}
	}
}



Tensor * CNN::inference(Tensor * input)
{
	Tensor * X = input;
	for(int i = 0; i < layers.size(); i++){
		CNN_layer_struct & lay = layers[i];
		// lay->X is the input lay->Z is the output
		auto start = mtick();
		switch(lay.type){
			case Layer_Type::ReLU:
				if(lay.in_place) {
    				ReLUInplace(X);
				} else {
    				ReLU(X, lay.Z);
    				X = lay.Z;
				}
				runtime[2] += mtock(start);
				break;
			case Layer_Type::Softmax:
				Softmax(X, lay.Z);
				X = lay.Z;
				runtime[4] += mtock(start);
				break;
			case Layer_Type::Pool:
				maxPool(X, lay.Z);
				X = lay.Z;
				runtime[1] += mtock(start);
				break;
			case Layer_Type::Conv:
				if(lay.pad > 0) {
					X = padTensor(X, lay.pad);
				}
				if(lay.optimized) {
					if(lay.optimized == Optimization::Wino) {
						convWinograd(X, lay.W, lay.B, lay.Z, lay.kernel_width);
						X = lay.Z;
					} else if(lay.optimized == Optimization::FFT) {
						convFFT(X, lay.W_fft, lay.B, lay.Z, lay.kernel_width);
						X = lay.Z;
					}
				} else {
    				convBasic(X, lay.W, lay.B, lay.Z);
    				X = lay.Z;
				}
				runtime[3] += mtock(start);
				break;
			case Layer_Type::Linear:
				Linear(X, lay.W, lay.B, lay.Z);
				X = lay.Z;
				runtime[0] += mtock(start);
				break;
			default:
				printf("Rogue unimplemented layer found during inference !\n");
		}
	}
	return X;
}

Optimization autoDecide(uint32_t output_size[3], uint32_t kernel_width, uint32_t input_channels) {
    uint32_t output_channels = output_size[0];
	//printf("Automatically deciding: \n");
	// TODO: Use results of SVM
	if(input_channels < 6) {
		return Optimization::None;
	} else if(kernel_width > 7 && input_channels > 6) {
		return Optimization::FFT;
	} else if(input_channels > 6 && kernel_width < 7) {
		return Optimization::Wino;
	} else {
		return Optimization::None;
	}

	/* Worse:
	if(kernel_width >= 11) { // TODO
		return Optimization::FFT;
	} else if(kernel_width < 5 && output_channels + input_channels >= 40) { // TODO
		return Optimization::Wino;
	} else {
		return Optimization::None;
	}
	*/
}

void CNN::prepare(Optimization optim /*= Optimization::Auto*/)
{
	for(int i = 0; i < layers.size(); i++){
		CNN_layer_struct & lay = layers[i];
		switch(lay.type){
			case Layer_Type::Conv: {
            	Optimization thisOptim = optim;
    			if(thisOptim == Optimization::Auto) {
        			// Automatically decide which optimization to use
        			thisOptim = autoDecide(lay.output_size, lay.kernel_width, lay.input_channels);
                    printf("Decided on %d\n", thisOptim);
				}

    			if(thisOptim == Optimization::FFT) {
					// Pre Transform the Weights for fft convolution
    				lay.W_fft = fftWeights(lay.W, lay.output_size[0]);
    			} else if(thisOptim == Optimization::Wino) {
					// Pre Transform the Weights for winograd convolution
    				lay.W = winoWeights(lay.W, lay.output_size[0]);
    			}

				lay.optimized = thisOptim;
				break;
			}
			case Layer_Type::ReLU:
			case Layer_Type::Softmax:
			case Layer_Type::Pool:
			case Layer_Type::Linear:
				break; // We only pretransform weights for the convolution layers
			default:
				printf("Rogue unimplemented layer found during preparation !\n");
		}
	}
}


}




