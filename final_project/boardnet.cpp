#include <cstdio>
#include <stdint.h>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "conv.h"
#include "common.h"

#define FPGA

#ifdef FPGA
extern "C"{
#include "pynq_api.h"
}
#endif

#include "stdint.h"
#include <vector>
#include <cmath>



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
	float * Z;
	float * W;
	float * B;
#ifdef FPGA
	PYNQ_SHARED_MEMORY sm_w, sm_b, sm_z;
#endif
} CNN_layer_struct;

/* ----------------------------- Functions to initialize Layers -----------------------------*/
CNN_layer_struct LinearLayer(uint32_t outputs);
CNN_layer_struct PoolLayer(uint32_t output_channels, uint32_t output_height, uint32_t output_width);
CNN_layer_struct ConvLayer(uint32_t input_channels, uint32_t output_channels, uint32_t output_height, 
		uint32_t output_width, uint32_t kernel_width, uint32_t pad);
CNN_layer_struct ReLULayer(bool in_place = true);
CNN_layer_struct SoftmaxLayer();

CNN_layer_struct LinearLayer(uint32_t outputs)
{
	CNN_layer_struct linear;
	linear.type = Layer_Type::Linear;
	linear.output_size[0] = 1;
	linear.output_size[1] = 1;
	linear.output_size[2] = outputs;
	return linear;
}

CNN_layer_struct PoolLayer(uint32_t output_channels, uint32_t output_height, uint32_t output_width)
{
	CNN_layer_struct pool;
	pool.type = Layer_Type::Pool;
	pool.output_size[0] = output_channels;
	pool.output_size[1] = output_height;
	pool.output_size[2] = output_width;
	return pool;
}

CNN_layer_struct ConvLayer(uint32_t input_channels, uint32_t output_channels, uint32_t output_height, 
		uint32_t output_width, uint32_t kernel_width, uint32_t pad)
{
	CNN_layer_struct conv;
	conv.type = Layer_Type::Conv;
	conv.kernel_width = kernel_width;
	conv.pad = pad;
	conv.output_size[0] = output_channels;
	conv.output_size[1] = output_height;
	conv.output_size[2] = output_width;
	conv.input_channels = input_channels;
	return conv;
}


CNN_layer_struct ReLULayer(bool in_place )
{
	CNN_layer_struct relu;
	relu.type = Layer_Type::ReLU;
	relu.in_place = in_place;
	return relu;
}

CNN_layer_struct SoftmaxLayer()
{
	CNN_layer_struct lay;
	lay.type = Layer_Type::Softmax;
	lay.in_place = false;
	return lay;
}

void allocLayers(std::vector<CNN_layer_struct> &layers)
{
	uint32_t insize = 0;
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
					lay.Z = new float[ layers[i-1].output_size[0] * lay.output_size[1] * lay.output_size[2] ];
				break;
			case Layer_Type::Pool:
				lay.Z = new float[ lay.output_size[0] * lay.output_size[1] * lay.output_size[2] ];
				break;
			case Layer_Type::Conv: {
#ifdef FPGA
            	int res = PYNQ_allocatedSharedMemory(&lay.sm_z, lay.output_size[0] * lay.output_size[1] * lay.output_size[2] * sizeof(float), 0);
				printf("Z res: %d\n", res);
            	res = PYNQ_allocatedSharedMemory(&lay.sm_w, lay.output_size[0] * lay.input_channels * lay.kernel_width * lay.kernel_width * sizeof(float), 0);
				printf("W res: %d\n", res);
            	res = PYNQ_allocatedSharedMemory(&lay.sm_b, lay.output_size[0] * sizeof(float), 0);
				printf("B res: %d\n", res);
            	lay.Z = (float *) lay.sm_z.pointer;
            	lay.W = (float *) lay.sm_w.pointer;
            	lay.B = (float *) lay.sm_b.pointer;
#else
				lay.Z = new float[ lay.output_size[0] * lay.output_size[1] * lay.output_size[2] ];
				lay.W = new float[ lay.output_size[0] * lay.input_channels * lay.kernel_width * lay.kernel_width ];
				lay.B = new float[ lay.output_size[0] ];
#endif
				break;
			}
			case Layer_Type::Linear:
				insize = layers[i-1].output_size[0] * layers[i-1].output_size[1] * layers[i-1].output_size[2];
				lay.Z = new float[ 1 * 1 * lay.output_size[2] ];
				lay.W = new float[ 1 * lay.output_size[2] * insize ];
				lay.B = new float[ 1 * 1 * lay.output_size[2] ];
				break;
			default:
				printf("Layer not implemented !\n");
		}
	}
}

void freeLayers(std::vector<CNN_layer_struct> &layers)
{
	uint32_t insize = 0;
	// The tricky part is allocating the proper tensors
	for(int i = 0; i < layers.size(); i++){
		CNN_layer_struct & lay = layers[i];
		// lay->X is the input lay->Z is the output
		switch(lay.type){
			case Layer_Type::ReLU: case Layer_Type::Softmax:
				if(!lay.in_place)
					delete [] lay.Z;
				break;
			case Layer_Type::Pool:
				delete [] lay.Z;
				break;
			case Layer_Type::Conv: {
#ifdef FPGA
            	int res = PYNQ_freeSharedMemory(&lay.sm_z);
				printf("Zfree res: %d\n", res);
            	res = PYNQ_freeSharedMemory(&lay.sm_w);
				printf("Wfree res: %d\n", res);
            	res = PYNQ_freeSharedMemory(&lay.sm_b);
				printf("Bfree res: %d\n", res);
#else
				delete [] lay.Z;
				delete [] lay.W;
				delete [] lay.B;
#endif
				break;
			}
			case Layer_Type::Linear:
				delete [] lay.Z;
				delete [] lay.W;
				delete [] lay.B;
				break;
			default:
				printf("Layer not implemented !\n");
		}
	}
}

int checkLayerMeta(CNN_layer_struct * lay, FILE * f)
{
	CNN_layer_struct tmp;
	if(fread(&(tmp.type),				sizeof(tmp.type),		1,f) == 0){
		printf("Meta Check failed!\n");
		return 0;
	}
	if(fread(&(tmp.input_channels),	sizeof(tmp.input_channels),1,f) ==0){
		printf("Meta Check failed!\n");
		return 0;
	}
	if(fread(&(tmp.output_size[0]),			sizeof(tmp.output_size[0]),	1,f) == 0 ){
		printf("Meta Check failed!\n");
		return 0;
	}
	if(fread(&(tmp.kernel_width),		sizeof(tmp.kernel_width),	1,f) == 0 ){
		printf("Meta Check failed!\n");
		return 0;
	}
	if(fread(&(tmp.pad),				sizeof(tmp.pad),		1,f) == 0 ){
		printf("Meta Check failed!\n");
		return 0;
	}
	// Check correctness
	if(!(tmp.type == lay->type))
		return 0;
	if(lay->type == Layer_Type::Conv){
		if(((tmp.input_channels == lay->input_channels) &&
			(tmp.kernel_width == lay->kernel_width) &&
			(tmp.pad == lay->pad) &&
			(tmp.output_size[0] == lay->output_size[0])) == 0){
			return 0;
		}
	}
	if(lay->type == Layer_Type::Linear){
		if(!(tmp.output_size[0] == tmp.output_size[0]))
			return 0;
	}
	return 1;
}

int readConvWandB(FILE * f,
                  float * W, uint32_t exp_W_size[4],
                  float * B, uint32_t exp_B_size[3])
{
    uint32_t W_size[4] = {exp_W_size[0], exp_W_size[1], exp_W_size[2], exp_W_size[3]};
    if(!readTensorSize(f, &W_size[1])) return 0;

    if(W_size[0] != exp_W_size[0] ||
		W_size[1] != exp_W_size[1] ||
		W_size[2] != exp_W_size[2] ||
		W_size[3] != exp_W_size[3]) {
    		printf("Dimensions don't match");
    }

    //resizeTensor(W, W_size, new_W_size, 4);
	for(uint32_t i = 0; i < W_size[0] ; i++){
    	if(i != 0) {
        	if(!readTensorSize(f, &W_size[1])) return 0;
    	}

        if(W_size[0] != exp_W_size[0] ||
    		W_size[1] != exp_W_size[1] ||
    		W_size[2] != exp_W_size[2] ||
    		W_size[3] != exp_W_size[3]) {
        		printf("Dimensions don't match");
        }

		if(!readTensorRaw(f, &(W)[i*W_size[1]*W_size[2]*W_size[3]], &W_size[1])) return 0;
	}

    uint32_t B_size[3];
    if(!readTensorSize(f, B_size)) return 0;

    if(B_size[0] != exp_B_size[0] ||
		B_size[1] != exp_B_size[1] ||
		B_size[2] != exp_B_size[2]) {
    	printf("B dims don't match");
    }

    //resizeTensor(B, B_size, new_B_size, 3);
    if(!readTensorRaw(f, B, B_size)) return 0;

	return 1; //CNN_RETURN_SUCCESS;
}

int readFCWandB(FILE * f,
                float * W, uint32_t exp_W_size[3],
                float * B, uint32_t exp_B_size[3])
{
    uint32_t W_size[3];
    if(!readTensorSize(f, W_size)) return 0;
    if(W_size[0] != exp_W_size[0] ||
		W_size[1] != exp_W_size[1] ||
		W_size[2] != exp_W_size[2]) {
    		printf("Dimensions don't match");
    }

    //resizeTensor(W, W_size, W_size, 3); 
    if(!readTensorRaw(f, W, W_size)) return 0;

    uint32_t B_size[3];
    if(!readTensorSize(f, B_size)) return 0;
    if(B_size[0] != exp_B_size[0] ||
		B_size[1] != exp_B_size[1] ||
		B_size[2] != exp_B_size[2]) {
    	printf("B dims don't match");
    }
    // resizeTensor(B, B_size, B_size, 3); 
    if(!readTensorRaw(f, B, B_size)) return 0;

	return 1; //CNN_RETURN_SUCCESS;
}

std::vector<CNN_layer_struct> mediumNet= {
	ConvLayer(3,96,128,128,7,3),
	ReLULayer(),
	PoolLayer(96,64,64),
	// Block 2
	ConvLayer(96,256,64,64,5,2),
	ReLULayer(),
	PoolLayer(256,32,32),
	// Block 3
	ConvLayer(256,384,32,32,3,1),
	ReLULayer(),
	PoolLayer(384,16,16),
	// Block 4
	ConvLayer(384,384,14,14,3,0),
	ReLULayer(),
	// Block 5
	ConvLayer(384,256,12,12,3,0),
	ReLULayer(),
	PoolLayer(256,6,6),
	// Classifier
	LinearLayer(1024),
	ReLULayer(),
	LinearLayer(1024),
	ReLULayer(),
	LinearLayer(100),
	SoftmaxLayer(),
};

int readMediumNet(FILE * f,	std::vector<CNN_layer_struct> &layers) {

	uint32_t nlayers;
	if(fread(&(nlayers),sizeof(nlayers),1,f) == 0){
		printf("Reading number of layers failed!\n");
		return 0; //CNN_RETURN_FAILED;
	}
	if(layers.size() != nlayers){
		printf("Layer sizes do not match reading weights failed!\n");
		return 0; //CNN_RETURN_FAILED;
	}
	uint32_t prev_size = 0;
	for(int i = 0; i < nlayers; i++){
		CNN_layer_struct * lay = &(layers[i]);
		if(checkLayerMeta(lay,f) == 0){
			printf("Layer check failed!\n");
			return 0;
		} 
		switch(lay->type){ 
			case Layer_Type::Pool:
			case Layer_Type::ReLU:
			case Layer_Type::Softmax:
				break;
			case Layer_Type::Conv: {
				uint32_t W_size[4] = {lay->output_size[0], lay->input_channels, lay->kernel_width, lay->kernel_width};
				uint32_t B_size[3] = {1, 1, lay->output_size[0]};
				if(!readConvWandB(f,
				                  lay->W, W_size,
				                  lay->B, B_size))
					return 0;
				break;
			}
			case Layer_Type::Linear: {

				//insize = layers[i-1].output_size[0] * layers[i-1].output_size[1] * layers[i-1].output_size[2];
				// lay.Z = new float[ 1 * 1 * lay.output_size[2] ];
				// lay.W = new float[ 1 * lay.output_size[2] * insize ];
				// lay.B = new float[ 1 * 1 * lay.output_size[2] ];
				uint32_t W_size[3] = {1, lay->output_size[2], prev_size};
				uint32_t B_size[3] = {1, 1, lay->output_size[2]};
				if(!readFCWandB(f,
				                lay->W, W_size,
				                lay->B, B_size))
					return 0;
				break;
			}
			default:
				printf("%d not a valid layer type!\n",(uint32_t) lay->type);
		}
		prev_size = lay->output_size[2] * lay->output_size[1] * lay->output_size[0];
	}
	fclose(f);
	return 1;
}


// Convolution implemented in conv

/*
 * Applies a max pool layer on X (size = stride = 2)
 * Tensor * X:	input Tensor
 * Tensor * Z:	output Tensor
 */
void maxPool(float * X, uint32_t X_size[3], float * Z, uint32_t Z_size[3])
{
	// printf("%d %d %d\n", X_size[0], X_size[1], X_size[2]);
	// printf("%d %d %d\n", Z_size[0], Z_size[1], Z_size[2]);
	int num_chan = X_size[0];
	int wx = X_size[1];
	int wy = X_size[2];
	for (int chan = 0; chan < num_chan; chan++) {
		for (int x = 0; x < wx; x += 2) {
			for (int y = 0; y < wy; y += 2) {
			float max = 0; //X->data[chan][x][y] and then skip that one in the loop
				for (int ix = 0; ix < 2/* && x + ix < wx*/; ix++) {
					for (int iy = 0; iy < 2/* && y + iy < wy*/; iy++) {
						if(chan*X_size[1]*X_size[2] + (x+ix)*X_size[2] + y+iy >=
						   X_size[0] * X_size[1] * X_size[2]) {
    						printf("x, y, ix, iy: %d %d %d %d\n",x,y,ix,iy);
						}
						float curr = X[chan*X_size[1]*X_size[2] + (x+ix)*X_size[2] + y+iy];
						if (curr > max) {
							max = curr;
						}
					}
				}
			Z[chan*Z_size[1]*Z_size[2] + (x / 2)*Z_size[2] + y / 2] = max;
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
void Linear(float * X, uint32_t X_size[3], float * W, uint32_t W_size[3], float * B, uint32_t B_size[3], float * Z, uint32_t Z_size[3])
{
	// printf("%d %d %d\n", X->size[0], X->size[1], X->size[2]);
	// printf("%d %d %d\n", W->size[0], W->size[1], W->size[2]);
	// printf("%d %d %d\n", B->size[0], B->size[1], B->size[2]);
	// printf("%d %d %d\n", Z->size[0], Z->size[1], Z->size[2]);
	uint32_t flat_X_size[3] = {X_size[0], X_size[1], X_size[2]};
	if(X_size[0] > 1) { //reshape into 1, 1, _
		flat_X_size[2] = X_size[1] * X_size[2] * X_size[0];
		flat_X_size[1] = 1;
		flat_X_size[0] = 1; // channel to 1
	}
	int wx = flat_X_size[1];
	int wy = flat_X_size[2];
	int wwx = W_size[1];
	for (int x = 0; x < wx; x++) {
		for (int y = 0; y < wwx; y++) {
			float sum = 0;
			for (int i = 0; i < wy; i++) {
				sum += X[0 + x*flat_X_size[2] + i] * W[0 + y*W_size[2] + i];
			}
			Z[0 + x*Z_size[2] + y] = sum + B[0 + x*B_size[2] + y];
		}
	}
}


/*
 * Applies the ReLU activation function: Z = ReLU(X)
 * Tensor * X: input Tensor
 * Tensor * Z: output Tensor
 */
void ReLU(float * X, float * Z, int input_size)
{
	// printf("%d %d %d\n", X->size[0], X->size[1], X->size[2]);
	// printf("%d %d %d\n", Z->size[0], Z->size[1], Z->size[2]);
    for (uint32_t i=0; i<input_size; i++) {
		float max = X[i];
		if(max < 0) {
			Z[i] = 0;
		} else {
			Z[i] = max;
		}
	}
}

/*
 * Applies the ReLU activation function inplace: X = ReLU(X)
 * Tensor * X: in/output Tensor
 */
void ReLUInplace(float * X, int input_size)
{
	// printf("%d %d %d\n", X->size[0], X->size[1], X->size[2]);
    for (uint32_t i=0; i<input_size; i++)
    {
        X[i] = std::max(0.0f, X[i]);
    }
}

/*
 * Applies the Softmax activation function z = exp(x_i)/sum(exp(x_j))
 * This is a stable Softmax implementation
 * Tensor * X: input vector in Tensor form
 * Tensor * Z: output vector in Tensor form
 */
void Softmax(float * X, float * Z, uint32_t size[3]) // Same in as out size
{
	// printf("%d %d %d\n", X->size[0], X->size[1], X->size[2]);
	// printf("%d %d %d\n", Z->size[0], Z->size[1], Z->size[2]);
	int num_chan = size[0];
	int wx = size[1];
	int wy = size[2];
	for (int chan = 0; chan < num_chan; chan++) {
		for (int x = 0; x < wx; x++) {
    		float sum = 0;
			for (int y = 0; y < wy; y++) {
				sum += exp(X[chan*size[1]*size[2] + x*size[2] +y]);
			}
			for (int y = 0; y < wy; y++) {
    			float max = exp(X[chan*size[1]*size[2] + x*size[2] +y]) / sum;
				Z[chan*size[1]*size[2] + x*size[2] +y] = max;
			}
		}
	}
}


float * inference(std::vector<CNN_layer_struct> &layers, float * input, double runtime[5]
#ifdef FPGA
                  , PYNQ_MMIO_WINDOW &hls
#endif
                  )
{
	float * X = input;
	int block = 1; // Which "block" of the CNN we are in, currently tracked hackily by counting conv layers
	for(int i = 0; i < layers.size(); i++){
		CNN_layer_struct & lay = layers[i];
		// lay->X is the input lay->Z is the output
		auto start = mtick();
		switch(lay.type){
			case Layer_Type::ReLU: {
    			printf("Relu\n");
    			int size = lay.output_size[0] * lay.output_size[1] * lay.output_size[2];
				if(lay.in_place) {
    				ReLUInplace(X, size);
				} else {
    				ReLU(X, lay.Z, size);
    				X = lay.Z;
				}
				runtime[2] += mtock(start);
				break;
			}
			case Layer_Type::Softmax: {
    			printf("Softmax\n");
				Softmax(X, lay.Z, lay.output_size);
				X = lay.Z;
				runtime[4] += mtock(start);
				break;
			}
			case Layer_Type::Pool: {
    			printf("Pool\n");
    			uint32_t in_size[3] = {lay.output_size[0], lay.output_size[1] * 2, lay.output_size[2] * 2};
				maxPool(X, in_size, lay.Z, lay.output_size);
				X = lay.Z;
				runtime[1] += mtock(start);
				break;
			}
			case Layer_Type::Conv: {
    			printf("Conv\n");
    			uint32_t padded_size[3] = {lay.input_channels, lay.output_size[1]+lay.kernel_width-1, lay.output_size[2]+lay.kernel_width-1};
                uint32_t unpadded_size[3] = {lay.input_channels, padded_size[1]-lay.pad*2, padded_size[2]-lay.pad*2};
                printf("padding: %d\n", lay.pad);
                printf("padded: %d %d %d \n", padded_size[0], padded_size[1], padded_size[2]);
				if(lay.pad > 0) { X = padTensor(X, unpadded_size, lay.pad); }

                uint32_t W_size[4] = {lay.output_size[0], lay.input_channels, lay.kernel_width, lay.kernel_width};
                uint32_t B_size[3] = {1, 1, lay.output_size[0]};
                //Reference impl:
        		// convBasic(X, padded_size,
        		//           lay.W, W_size,
        		//           lay.B, B_size,
        		//           lay.Z, lay.output_size);
#ifdef FPGA
				// TODO: Alloc lay.Z of previous layer as shared to elide memcpy here
				PYNQ_SHARED_MEMORY sm_x;
            	int res = PYNQ_allocatedSharedMemory(&sm_x, padded_size[0] * padded_size[1] * padded_size[2] * sizeof(float), 1);
				printf("X res: %d\n", res);

				float * virt_x = (float *) sm_x.pointer;
    			memcpy(virt_x, X, sizeof(float) * padded_size[0]*padded_size[1]*padded_size[2]);

            	// Copy addr over axilite, these can be found in the HLS Summary
            	memcpy(hls.buffer + 0x10, &(sm_x.physical_address), sizeof(size_t));
            	memcpy(hls.buffer + 0x1c, &(lay.sm_w.physical_address), sizeof(size_t));
            	memcpy(hls.buffer + 0x28, &(lay.sm_b.physical_address), sizeof(size_t));
            	memcpy(hls.buffer + 0x54, &(lay.sm_z.physical_address), sizeof(size_t));
    			memcpy(hls.buffer + 0x44, &(block), sizeof(int));

    			/* Start HLS by setting bit */
				uint32_t * hls_ctrl = (uint32_t *) hls.buffer;
    			*hls_ctrl = 0b1;
    			printf("start\n");
    			while(!(*hls_ctrl & 0b100)){}; // waiting for the IDLE(the 3rd bit) to 1, then we can print out the result
    			printf("end\n");

            	res = PYNQ_freeSharedMemory(&sm_x);
				printf("Xfree res: %d\n", res);

#else
        		// TODO: Rename in_h param to block and remove the others
    			//        in_sm, w_sm,	b_sm, in_w,      in_h,      in_c,      out_c,     out_sm
            	EntryConv(X,     lay.W, lay.B,0,		 block,			0, 		   0, 		  lay.Z     );
#endif

        		if(lay.pad > 0) { delete X; }
				X = lay.Z;
				runtime[3] += mtock(start);
				block++;
				break;
			}
			case Layer_Type::Linear: {
    			printf("Linear\n");
    			uint32_t prev_size = layers[i-1].output_size[2] * layers[i-1].output_size[1] * layers[i-1].output_size[0];
				uint32_t W_size[3] = {1, lay.output_size[2], prev_size};
				uint32_t B_size[3] = {1, 1, lay.output_size[2]};
				Linear(X, layers[i-1].output_size,
				       lay.W, W_size,
				       lay.B, B_size,
				       lay.Z, lay.output_size);
				X = lay.Z;
				runtime[0] += mtock(start);
				break;
			}
			default:
				printf("Rogue unimplemented layer found during inference !\n");
		}
	}
	return X;
}

#include "inf_utils.h"

void benchNet(std::vector<CNN_layer_struct> &layers,
        const char * images[],
		const int ref[], int N_in
#ifdef FPGA
		, PYNQ_MMIO_WINDOW &hls
#endif
        )
{
	int N = N_in;
	if(N > IMAGES_MAX_TEST){
		printf("%d above amount of available images!\n", N);
		N = IMAGES_MAX_TEST;
	}
	std::vector<float *> X(N);
	std::vector<int> pred(N);
	
	for(int i =0; i < N; i++){
		printf("%s\n",images[i]);
		X[i] = readBMP(images[i]);
	}
	double runtime[5] = {0};
	double total_time = 0;
	for(int i =0; i < N; i++){
		printf("Image: %s\n",images[i]);
		auto start = mtick();
		float * Z = inference(layers, X[i], runtime
#ifdef FPGA
		                      , hls
#endif
		);
		total_time += mtock(start);
		pred[i] = classImage(Z);
		printf("Actual Class: %s\n",classes_img100[ref[i]]);
	}
	printf("Total Time [ms]: %lf\n",total_time);
	printf("Frames/s : %lf\n",(N*1000)/total_time);

	// Per layer type timing
	const char * names[] = {"Linear","Pool","ReLU","Conv","Softmax"};
	printf("Execution Time [ms]:\n");
	printf("--------------------\n");
	for(int i = 0; i < 5; i++){
		printf("%s Layer: %lf \n",names[i],runtime[i]);
	}
	printf("--------------------\n");

	/* Correctness */
	int corr = 0;
	for(int i =0; i < N; i++){
		if(ref[i] == pred[i])
			corr++;
	}
	printf("Accuracy: %d of %d images (%f %% )!\n",corr,N,(corr/(N*1.0f)));
	for(int i = 0; i < N; i++){
		delete X[i];
	}
}

int main(int argc, char* argv[]) {
	if(argc < 2){
		printf("Usage: ./boardnet N\n");
		return 1;
	}
	int N = atoi(argv[1]);
 	printf("Running mediumnet on %d images.\n",N);

	auto f = fopen("/home/xilinx/final/weights/mediumnet_weights.dat", "rb");
	if(!f) f = fopen("weights/mediumnet_weights.dat", "rb");
	if(!f) f = fopen("/home/xilinx/final_project/weights/mediumnet_weights.dat", "rb");
	if(!f) {
		printf("Please adapt the absolute path to the weights in boardnet.cpp:main\n");
    	return 1;
	}

#ifdef FPGA
	/* connect to the leds */
	PYNQ_loadBitstream((char * )"mediumnet.bit");
	PYNQ_MMIO_WINDOW led, hls;  // hls is AXILITE interface
	PYNQ_createMMIOWindow(&led, 0x40010000,8); //in this functino, it uses mmap; the address can be got from vivado; 8 is length
	PYNQ_createMMIOWindow(&hls, 0x40000000,128); //0x00 to 0x2c is 32+12=44, plus 4bytes, 48 is enough. We give 64. It will allocate the whole page anyway, so it doesn't really matter how big you set this, as long as you don't go over 4kB; This is the MMIOwindow for hls_slave, we can write to here.
	uint32_t * hls_ctrl = (uint32_t *) hls.buffer;
	*hls_ctrl = 0b100;
	// The structure of var"led" can be got from doc
	uint32_t * b_led = (uint32_t *) led.buffer;
	b_led[1] = 0; // the TRI register
	b_led[0] = 3; // DATA register
#endif

	allocLayers(mediumNet);
	readMediumNet(f, mediumNet);
 	printf("CCCCC\n");

	benchNet(mediumNet,input_imgs,imgs_class,N
#ifdef FPGA
	         , hls
#endif
	);

	freeLayers(mediumNet);

	/* Now can write the input data to the memory address */
	// One cannot write to the pointer to virtual memory, so cast it to float * first

	fclose(f);

#ifdef FPGA
	PYNQ_closeMMIOWindow( &led);
	PYNQ_closeMMIOWindow( &hls);
#endif
	return 0;
}
