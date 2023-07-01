#include "tensor.h"
#include <vector>
#include "time.h"
#include "conv.h"
#include <cstdio>
#include "fft.h"
#include <iostream>


#include "kernels.h"
#include "cnn.h"
#include "nets.h"
#include <string>
#include "imagenet_classes.h"
#include <algorithm>

using namespace ml;


void testConv(const char * infile, Optimization optim);
void timeConv(uint32_t input_channels, uint32_t input_width, uint32_t kernel_size, 
		uint32_t output_channels, int N, Optimization optim);
void test_net(const char * weight_file , const char * data_file, std::vector<CNN_layer_struct> net, Optimization optim);
void predict_image(const char * weight_file , const char * data_file, std::vector<CNN_layer_struct> net,
		const char * imagefile, Optimization optim);
void bench_net(const char * weight_file , std::vector<CNN_layer_struct> net, int N, Optimization optim);


int main(int argc , char * argv[])
{
	// printf("argv[0]: %s\n",argv[0]);
	// printf("argv[1]: %s\n",argv[1]);
	// printf("argv[2]: %s\n",argv[2]);
	if(argc < 3) {
		printf("Usage (test) : ./lab3.bin 0 optim\n");
		printf("Usage (bench): ./lab3.bin 1 optim input_channels input_width kernel_size output_channels N\n");
		printf("Usage (test lab2 nets) : ./lab3.bin 2 optim\n");
		printf("Usage (classify with vgg) : ./lab3.bin 3 optim image_tensor\n");
		printf("Usage (bench lab2 nets) : ./lab3.bin 4 optim\n");
		printf("optim: 0 = naive, 1 = fft, 2 = wino, 3 = auto\n");
		return 1;
	}

	uint32_t argv2 = atoi(argv[2]);
	if(argv2 < 0 || argv2 > 3) {
    	printf("optim needs to be in 0..3");
	}
	Optimization optim = static_cast<Optimization>(argv2);
	int subcmd = atoi(argv[1]);
	if(subcmd == 0) {
    	if(optim == Auto) {
			printf("Running convolution test with automatic optimization\n");
    	}
		testConv("data/conv_test.dat",optim);
	} else if(subcmd == 1) {
		if(argc < 8){
			printf("Usage: ./lab3.bin 1 optim input_channels input_width kernel_size output_channels N\n");
			return 1;
		}
		if(optim == Auto) {
			printf("Running benchmark with automatic optimization\n");
		}
		uint32_t ic = atoi(argv[3]);
		uint32_t iw = atoi(argv[4]);
		uint32_t ks = atoi(argv[5]);
		uint32_t oc = atoi(argv[6]);
		uint32_t N = atoi(argv[7]);
		timeConv(ic,iw,ks,oc,N,optim);
	} else if(subcmd == 2) {
		if(optim == Auto) {
			printf("Running testnets with automatic optimization\n");
		}
		printf("Running test!\n");
		test_net("data/testnet_weights.dat","data/testnet_test.dat",testNet, optim);
		test_net("data/smallnet_weights.dat","data/smallnet_test.dat",smallNet, optim);
		test_net("data/mediumnet_weights.dat","data/mediumnet_test.dat",mediumNet, optim);
		test_net("data/largenet_weights.dat","data/largenet_test.dat",largeNet, optim);
		test_net("data/giantnet_weights.dat","data/giantnet_test.dat",giantNet, optim);
	} else if(subcmd == 3) {
		printf("Image Classification\n");
		predict_image("data/vgg16_weights.dat","data/vgg16_test.dat",VGG16,argv[2], optim);
	} else if(subcmd == 4) {
		printf("Running Benchmarks\n");
		bench_net("data/smallnet_weights.dat",smallNet,4, optim);
		bench_net("data/mediumnet_weights.dat",mediumNet,4, optim);
		bench_net("data/largenet_weights.dat",largeNet,4, optim);
		bench_net("data/giantnet_weights.dat",giantNet,4, optim);
	}
	return 0;
}

Tensor * readConv(Tensor * X, Tensor * Ref, Tensor * B , FILE * f);

void testConv(const char * infile, Optimization optim)
{
	FILE * f = fopen(infile,"rb");
	Tensor X,R,B;
	printf("------------------------------\n");
	printf("Testing Convolutional Layer...\n");
	while(1){
		Tensor * W = readConv(&X,&R,&B,f);
		if(W == NULL)
			break;
		Tensor Z(R.size[0],R.size[1],R.size[2]);
		Z.randomize(-1,1);
		printf("Test X:[%dx%dx%d] W:[%dx%d] Output channels: %d!\n",X.size[0],X.size[1],X.size[2],
				W->size[1],W->size[2],R.size[0]);
		/* Select Optimization for *this* loop iteration only */
		Optimization thisOptim = optim;
		if(optim == Auto)
    		thisOptim = autoDecide(R.size, W->size[1], X.size[0]);

		if(thisOptim == None)
			convBasic(&X,W,&B,&Z);
		else if(thisOptim == FFT) {

			/* test the bit_reverse_copy funtion*/
			// C_FLOAT x_in[] = {C_FLOAT(0), C_FLOAT(1), C_FLOAT(2), C_FLOAT(3), C_FLOAT(4), C_FLOAT(5), C_FLOAT(6), C_FLOAT(7)};
			// C_FLOAT x_out[8];
			// int N = 8;
			// bit_reverse_copy(x_in,x_out,N);
			// for (int i = 0; i < 8; ++i) {
			// 	std::cout << "Element " << i << ": " << x_out[i].real() << " + " << x_out[i].imag() << "i" << std::endl;
			// }

			// // print out the elements of the first channel of the second weight tensor
			// for (int i = 0; i < 2; ++i){
			// 	for (int j = 0; j < W[i].size[1]; ++j) {
			// 	for (int k = 0; k < W[i].size[2]; ++k) {
			// 		std::cout << "Element " << i << ", " << j << ", " << k << ": " << W[i].data[0][j][k]<< std::endl;
			// 	}
			// }
			// }
			
			// C_Tensor * W_out = new C_Tensor[2];
			// flip_Matrix(W,W_out,0);
			// //print out the elements of the first channel of the first weight tensor after flip
			// for (int j = 0; j < W_out->size[1]; ++j) {
			// 	for (int k = 0; k < W_out->size[2]; ++k) {
			// 		std::cout << "W_out Element " << "0" << ", " << j << ", " << k << ": " << W_out->data[0][j][k]<< std::endl;
			// 	}
			// }

			C_Tensor * U = fftWeights(W,Z.size[0]);
			convFFT(&(X),U,&(B),&(Z),W->size[2]);
			delete [] U;
		}
		else if(thisOptim == Wino) {
			Tensor * U = winoWeights(W,R.size[0]);

			convWinograd(&X,U,&B,&Z,W->size[2]);
			delete [] U;
		}
		compareTensors(&Z,&R,1,1e-3);
		delete [] W;
	}
	fclose(f);
}

void timeConv(uint32_t input_channels, uint32_t input_width, uint32_t kernel_size, 
		uint32_t output_channels, int N, Optimization optim)
{
	Tensor * X = new Tensor[N];
	Tensor * W = new Tensor[N * output_channels];
	Tensor * B = new Tensor[N];
	uint32_t out_width = input_width - kernel_size + 1;
	Tensor Z(output_channels, out_width, out_width);
	for(int i =0 ; i  < N ; i++){
		X[i].allocate(input_channels,input_width, input_width);
		X[i].randomize(-1,1);
		B[i].allocate(1,1,output_channels);
		B[i].randomize(-1,1);
		for(int j = 0; j < output_channels; j++){
			W[i*output_channels + j].allocate(input_channels,kernel_size,kernel_size);
			W[i*output_channels + j].randomize(-1,1);
		}
	}

	if(optim == Auto)
    	optim = autoDecide(Z.size, kernel_size, input_channels);

	printf("Benchmark %d, N: %d, input size: %d,%d,%d kernel_size: %d output_channels: %d \n"
			,optim, N,input_channels,input_width,input_width, kernel_size, 
			output_channels);
	double total_time = 0;
	if(optim == None) {
		auto start = mtick();
		for(int i = 0; i < N ; i++){
			convBasic(&(X[i]),&(W[i * output_channels]),&(B[i]),&(Z));
		}
		total_time = mtock(start);
	}
	else if(optim == FFT) {
		C_Tensor ** U = new C_Tensor*[N];
		for(int i =0 ; i < N; i++)
			U[i] = fftWeights(&(W[i*output_channels]),Z.size[0]);
		auto start = mtick();
		for(int i = 0; i < N ; i++){
			convFFT(&(X[i]),U[i],&(B[i]),&(Z),W->size[2]);
		}
		total_time = mtock(start);
		for(int i =0 ; i < N; i++)
			delete [] U[i];
		delete [] U;
	}
	else if(optim == Wino) {
		Tensor ** U = new Tensor*[N];
		for(int i =0; i < N; i++){
			U[i] = winoWeights(&(W[i*output_channels]),Z.size[0]);
		}
		auto start = mtick();
		for(int i =0; i < N ; i++){
			convWinograd(&(X[i]),U[i],&(B[i]),&Z,W->size[2]);
		}
		total_time = mtock(start);
		for(int i =0 ; i < N ; i++)
			delete [] U[i];
		delete [] U;
	}

	delete [] X;
	delete [] W;
	delete [] B;

	printf("Total Time [ms]: %lf \n",total_time);
	printf("Avg. Time [ms]: %lf \n",total_time/N);
}

void test_net(const char * weight_file , const char * data_file, std::vector<CNN_layer_struct> net, Optimization optim)
{
	CNN dut(net);
	if(dut.read(weight_file) == CNN_RETURN_FAILED){
		printf("Reading Weights failed!\n");
		return;
	}
	FILE *f;
	if((f = fopen(data_file,"rb")) == NULL){
		printf("Reading data file failed!\n");
		return;
	}
	uint32_t ntests;
	if(fread(&ntests,sizeof(ntests),1,f) == 0){
		printf("Reading tests failed!\n");
		return;
	}

	dut.prepare(optim);

	Tensor X,R;
	double total_time = 0;
	printf("Weights: %s\n", weight_file);
	printf("Data: %s\n", data_file);
	printf("N: %d\n", ntests);
	for(int i = 0; i < ntests ; i++){
		X.read(f);
		R.read(f);
		auto start = mtick();
		Tensor * Z = dut.inference(&X);
		total_time += mtock(start);
		if(Z->size[2] != R.size[2]){
			printf("Test failed Output Tensor has the wrong Dimensions! \n");
			return; 
		}
		compareTensors(&R,Z,1,0.01);
	}
	printf("Total time[ms]: %lf\n",total_time);
	dut.print_timing();
	fclose(f);
}

void predict_image(const char * weight_file , const char * data_file, std::vector<CNN_layer_struct> net,
		const char * imagefile, Optimization optim)
{
	CNN dut(net);
	if(dut.read(weight_file) == CNN_RETURN_FAILED){
		printf("Reading Weights failed!\n");
		return;
	}
	Tensor X;
	FILE * f;
	if((f = fopen(imagefile,"rb")) == NULL){
			printf("Reading imagefile failed!\n");
			return;
	}
	X.read(f);
	fclose(f);
	printf("Starting preparation !\n");
	dut.prepare(optim);
	printf("Starting inference !\n");
	auto start = mtick();
	Tensor * Z = dut.inference(&X);
	double time = mtock(start);
	printf("Total time[ms]: %lf\n",time);
	dut.print_timing();
	int maxel = std::distance((*Z)[0][0],std::max_element((*Z)[0][0],(*Z)[0][0] + 1000));
	printf("Predicted class with %f : %s \n",(*Z)[0][0][maxel],imagenet_classes[maxel]);
}

void bench_net(const char * weight_file , std::vector<CNN_layer_struct> net, int N, Optimization optim)
{
	CNN dut(net);
	if(dut.read(weight_file) == CNN_RETURN_FAILED){
		printf("Reading Weights failed!\n");
		return;
	}
	Tensor * X = new Tensor[N];
	for(int i =0 ; i  < N ; i++){
		X[i].allocate(3,128,128);
	}
	dut.prepare(optim);
	printf("Benchmark: %s , N: %d\n",weight_file,N);
	auto start = mtick();
	for(int i =0 ; i < N ; i++){
		Tensor * Z = dut.inference(&(X[i]));
	}
	double extime = mtock(start);
	printf("Total Time [ms]: %lf\n",extime);
	dut.print_timing();
	delete [] X;
}


Tensor * readConv(Tensor * X, Tensor * Ref, Tensor * B , FILE * f)
{
	if(X->read(f) == TENSOR_READ_FAILED)
		return NULL;
	if(Ref->read(f) == TENSOR_READ_FAILED)
		return NULL;
	Tensor * W = new Tensor[Ref->size[0]]();
	for(int i = 0; i < Ref->size[0] ; i++){
		if(W[i].read(f) == TENSOR_READ_FAILED){
			delete [] W;
			return NULL;
		}
	}
	if(B->read(f) == TENSOR_READ_FAILED){
		delete [] W;
		return NULL;
	}
	return W;
}
