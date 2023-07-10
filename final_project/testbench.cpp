#include <cstdio>
#include "conv.h"
#include <cstdlib>
#include <time.h>
#include <cmath>
#include "stdint.h"
#include <stdio.h>
#include <stdexcept>

// void Tensor::write(FILE *f)
// {
// 	uint64_t num_params = size[0]*size[1]*size[2];
// 	fwrite(&(size[0]),sizeof(size[0]),1,f);
// 	fwrite(&(size[1]),sizeof(size[1]),1,f);
// 	fwrite(&(size[2]),sizeof(size[2]),1,f);
// 	fwrite(data[0][0],sizeof(data[0][0][0]),num_params,f);
// }

// Tensor * padTensor(Tensor * X , uint32_t pad)
// {
// 	int N = X->size[1] +  pad *2;
// 	int K = X->size[2] +  pad *2;
// 	Tensor * Xpad = new Tensor(X->size[0], N,K);
// 	for(int z = 0; z < X->size[0]; z++){
// 		float ** xpad = (*Xpad)[z];
// 		float ** x = (*X)[z];
// 		for(int i = 0; i < N ;i++){
// 			for(int j = 0; j < K; j++){
// 				if((i >= pad) && (i < (N-pad)) &&
// 							(j >=  pad) && (j <  (K- pad)))
// 					xpad[i][j] = x[i-pad][j-pad];
// 			}
// 		}
// 	}
// 	return Xpad;
// }

static float Fabs(float a)
{
	return (a > 0) ? a : -a;
}

// Compare two Tensor arrays of length N 
int compareTensorsRaw(float* a, uint32_t a_size[3], float* ref, uint32_t ref_size[3], float limit){
	if(a_size[0] != ref_size[0] || (a_size[1] != ref_size[1]) 
			|| (a_size[2] != ref_size[2])){
		throw std::runtime_error("Tensor dimensions don't match ! \n");
	}
	int ret = 0;
	double abs_diff = 0;
	for(int z = 0; z < a_size[0]; z++){
	for(int y = 0; y < a_size[1]; y++){
	for(int x = 0; x < a_size[2]; x++){
		int index = z*(a_size[1]*a_size[2]) + y*(a_size[2]) + x;
		float diff = Fabs( a[index] - ref[index] );
		if((diff > limit) && (ret == 0)){
			printf("Tensors differ at: [%d][%d][%d] by %f \n",
					z,y,x,diff);
			ret = 1;
		}
		abs_diff += diff;
	}}}
	printf("Total avg diff: %lf\n",abs_diff/(1.f * a_size[0] * a_size[1] * a_size[2]));
	if(ret == 0){
		printf("Tensors are equal!\n");
	}
	return  ret;
}

void convBasic(float* X, uint32_t X_size[3],
               float* W, uint32_t W_size[4],
               float* B, uint32_t B_size[3],
               float* Z, uint32_t Z_size[3])
{
	int in_chan = X_size[0];
	int out_chan = Z_size[0];
	int wx = X_size[1];
	int wy = X_size[2];
	int weight_wx = W_size[2];
	int weight_wy = W_size[3];
	for (int oc = 0; oc < out_chan; oc++) {
    	for (int x = 0; x < wx - weight_wx + 1; x++) {
    		for (int y = 0; y < wy - weight_wy + 1; y++) {
        		float sum = 0;
           		for (int ic = 0; ic < in_chan; ic++) {
        			for (int ix = 0; ix < weight_wx; ix++) {
        				for (int iy = 0; iy < weight_wy; iy++) {
        					sum += X[ic*wx*wy+(x+ix)*wy+y+iy] * 
        					       W[oc*in_chan*weight_wx*weight_wy+ic*weight_wx*weight_wy+ix*weight_wy+iy];
        				}
        			}
           		}
    			Z[oc*Z_size[1]*Z_size[2]+x*Z_size[2]+y] = sum + B[oc];
    		}
    	}
	}
}

//Returns 1 if realloc was necessary
void resizeTensor(float ** out, uint32_t size[], uint32_t new_size[], int dimensions)
{
    uint32_t prev = 1;
    uint32_t next = 1;
    for(int i = 0; i < dimensions; i++) {
		prev *= size[i];
		next *= new_size[i];
		size[i] = new_size[i];
    }
	if(prev != next){
		delete [] *out;
		*out = new float[next];
	}
}

//Returns 0 on failure
int readTensorSize(FILE * f, uint32_t size[3])
{
	for(int i = 0; i < 3 ;i++){
		if(1 != fread(&(size[i]),sizeof(size[0]),1,f))
			return 0;
	}
	return 1;
}

//Returns 0 on failure
int readTensorRaw(FILE * f, float * out, uint32_t size[3])
{
	uint32_t num_params = size[0] * size[1] * size[2];
	if(num_params != fread(out,sizeof(float),num_params,f))
		return 0;
	return 1;
}

//Returns 0 on failure
int readConvRaw(float ** X, uint32_t X_size[3], float ** Ref, uint32_t Ref_size[3], float ** W, uint32_t W_size[4], float ** B, uint32_t B_size[3], FILE * f)
{
    uint32_t new_X_size[3];
    if(!readTensorSize(f, new_X_size)) return 0;
    resizeTensor(X, X_size, new_X_size, 3); 
    if(!readTensorRaw(f, *X, X_size)) return 0;

    uint32_t new_Ref_size[3];
    if(!readTensorSize(f, new_Ref_size)) return 0;
    resizeTensor(Ref, Ref_size, new_Ref_size, 3); 
	if(!readTensorRaw(f, *Ref, Ref_size)) return 0;

    uint32_t new_W_size[4] = {Ref_size[0]};
    if(!readTensorSize(f, &new_W_size[1])) return 0;
    resizeTensor(W, W_size, new_W_size, 4);
	for(int i = 0; i < Ref_size[0] ; i++){
    	if(i != 0) {
        	if(!readTensorSize(f, &new_W_size[1])) return 0;
    	}
		if(!readTensorRaw(f, &(*W)[i*W_size[1]*W_size[2]*W_size[3]], &W_size[1])) return 0;
	}

    uint32_t new_B_size[3];
    if(!readTensorSize(f, new_B_size)) return 0;
    resizeTensor(B, B_size, new_B_size, 3); 
    if(!readTensorRaw(f, *B, B_size)) return 0;

	return 1;
}

void testConv(const char * infile)
{
	FILE * f = fopen(infile,"rb");
	//Tensor X,R,W,B;
	float* X = new float[0];
	uint32_t X_size[3] = {0, 0, 0};
	float* R = new float[0];
	uint32_t R_size[3] = {0, 0, 0};
	float* W = new float[0];
	uint32_t W_size[4] = {0, 0, 0, 0};
	float* B = new float[0];
	uint32_t B_size[3] = {0, 0, 0};
	printf("------------------------------\n");
	printf("Testing Convolutional Layer...\n");
	while(1){
		if(!readConvRaw(&X,X_size, &R,R_size, &W,W_size, &B,B_size, f)) break;

		float* Z = new float[R_size[0]*R_size[1]*R_size[2]];
		//Z.randomize(-1,1);
		printf("Test X:[%dx%dx%d] W:[%dx%d] Output channels: %d!\n",X_size[0],X_size[1],X_size[2],
				W_size[2],W_size[3],R_size[0]);

		//convBasic(&X,W,&B,&Z);
		if(W_size[2] == 3 && W_size[3] == 3) {
			if(X_size[0] > IN_CHANNEL) printf("Too many input channels\n");
			if(X_size[1] > IN_SIZE || X_size[2] > IN_SIZE) printf("Input too big\n");
			if(R_size[0] > OUT_CHANNEL) printf("Too many output channels\n");
			//Use FPGA for Conv2D_3x3:
    		//        in_sm, w_sm,	b_sm, in_w,      in_h,      in_c,      out_c,     out_sm
        	EntryConv(X,     W,     B,    X_size[1], X_size[2], X_size[0], R_size[0], Z     );
			// convBasic(X,X_size, W,W_size, B,B_size, Z,R_size);
    		compareTensorsRaw(Z,R_size,R,R_size,1e-3);
		} else {
    		printf("Skipped because not 3x3 kernel\n");
		}
		delete [] Z;
	}
	delete [] X;
	delete [] R;
	delete [] W;
	delete [] B;
	fclose(f);
}

int main(){
	// testConv("/home/clyybber/projects/uni/cnnhw/final/ACCN-Homework/final_project/data/conv_test.dat");
	testConv("/home/hewei/TUM/ACCN/ACCN-Homework/final_project/data/conv_test.dat");
	/*
	for(int i = 0; i < ntests; i++){
		EntryConv(x + (i*INPUT_SIZE),w + (i*KERNEL_SIZE), z + (i*OUTPUT_SIZE));
	}*/
	// printf("All values are equal!\n");
	return 0;
}

