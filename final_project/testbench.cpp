#include <cstdio>
#include "conv.h"
#include <cstdlib>
#include <time.h>
#include <cmath>
#include "stdint.h"
#include <stdio.h>
#include <stdexcept>

#define TENSOR_READ_SUCCSEFULL	0
#define TENSOR_READ_RESIZED		1
#define TENSOR_READ_FAILED 		2

class Tensor{
	private:
		int allocated;

	public:
		float *** data;
		void allocate(uint32_t dim_z, uint32_t dim_y, uint32_t dim_x);
		uint32_t size[3];
		// Constructor and Destructor
		Tensor(uint32_t dim_z, uint32_t dim_y, uint32_t dim_x);
		// Creates an empty un- allocated Tensor
		Tensor();
		// Destructor
		~Tensor();
		// Read and write Tensor to file
		int read(FILE * f);
		void write(FILE * f);
		void resize(uint32_t dim_z, uint32_t dim_y, uint32_t dim_x);
		// Easy access to the data
		float ** operator[](uint32_t i){
			return data[i];
		}
};

void Tensor::resize(uint32_t dim_z, uint32_t dim_y, uint32_t dim_x)
{
	if(data != NULL){
		delete [] data[0][0];
		delete [] data[0];
		delete [] data;
	}
	allocate(dim_z,dim_y,dim_x);
}

void Tensor::allocate(uint32_t dim_z, uint32_t dim_y, uint32_t dim_x)
{
	allocated = 1;
	data = new float ** [dim_z];
	float ** tmp_y = new float * [(uint64_t) dim_y *
		(uint64_t) dim_z];
	float * tmp_x = new float[(uint64_t) dim_z * 
		(uint64_t) dim_y * (uint64_t) dim_x]();
	for(int i = 0; i < dim_y*dim_z ; i++){
		tmp_y[i] = &(tmp_x[i * dim_x]);
	}
	for(int i = 0; i < dim_z ; i++){
		data[i] = &(tmp_y[i * dim_y]);
	}
	size[0] = dim_z;
	size[1] = dim_y;
	size[2] = dim_x;
}

Tensor::Tensor(uint32_t dim_z, uint32_t dim_y, uint32_t dim_x)
{
	allocate(dim_z,dim_y,dim_x);
}

Tensor::Tensor()
{
	allocated = 0;
	data = NULL;
	size[0] = 0;
	size[1] = 0;
	size[2] = 0;
}




Tensor::~Tensor()
{
	if(data != NULL){
		if(data[0] != NULL){
			if((data[0][0] != NULL) && (allocated))
				delete [] data[0][0];
			delete [] data[0];
		}
		delete [] data;
	}
}


void Tensor::write(FILE *f)
{
	uint64_t num_params = size[0]*size[1]*size[2];
	fwrite(&(size[0]),sizeof(size[0]),1,f);
	fwrite(&(size[1]),sizeof(size[1]),1,f);
	fwrite(&(size[2]),sizeof(size[2]),1,f);
	fwrite(data[0][0],sizeof(data[0][0][0]),num_params,f);
}


static float Fabs(float a)
{
	return (a > 0) ? a : -a;
}


// Compare two Tensor arrays of length N 
int compareTensors(Tensor * y, Tensor * ref, int N, float limit){
	int n,i,j,k;
	int ret = 0;
	double abs_diff = 0;
	for(n = 0; n < N; n++){
		for(i = 0; i < y[n].size[0]; i++){
			if(y[n].size[0] != ref[n].size[0] || (y[n].size[1] != ref[n].size[1]) 
					|| (y[n].size[2] != ref[n].size[2])){
				throw std::runtime_error("Tensor dimensions don't match ! \n");
			}
			float ** my = y[n][i];
			float ** mr = ref[n][i];
			// printf("my values\n");
			for(int j =0; j < y[n].size[1]; j++){
				for(int k =0; k < y[n].size[2];k++){
					float diff = Fabs( my[j][k] -  mr[j][k]);
					if((diff > limit) && (ret == 0)){
						printf("Tensors differ at: [%d][%d][%d] by %f \n",
								i,j,k,diff);
						ret = 1;
					}
					abs_diff += diff;
					// // print out my and mr values
					// printf("%f",my[j][k]);
				}
				// printf("\n");
			}
			// // print out ref values
			// printf("ref values\n");
			// for (int j = 0; j < ref[n].size[1]; j++){
			// 	for(int k = 0; k < ref[n].size[2]; k++){
			// 		printf("%f",mr[j][k]);
			// 	}
			// 	printf("\n");
			// }

		}
	}
	printf("Total avg diff: %lf\n",abs_diff/(1.f * y->size[0] * y->size[1] * y->size[2]));
	if(ret == 0){
		printf("Tensors are equal!\n");
	}
	return  ret;
}

// Compare two Tensor arrays of length N 
int compareTensorsRaw(float* a, uint32_t a_size[3], float* ref, uint32_t ref_size[3], float limit){
	if(a_size[0] != ref_size[0] || (a_size[1] != ref_size[1]) 
			|| (a_size[2] != ref_size[2])){
		throw std::runtime_error("Tensor dimensions don't match ! \n");
	}
	int i,j,k;
	int ret = 0;
	double abs_diff = 0;
	for(int z = 0; z < a_size[0]; z++){
	for(int y = 0; y < a_size[1]; y++){
	for(int x = 0; x < a_size[2]; x++){
    	int index = z*(a_size[2]*a_size[3]) + y*(a_size[3]) + x;
		float diff = Fabs( a[index] -  ref[index] );
		if((diff > limit) && (ret == 0)){
			printf("Tensors differ at: [%d][%d][%d] by %f \n",
					i,j,k,diff);
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

Tensor * padTensor(Tensor * X , uint32_t pad)
{
	int N = X->size[1] +  pad *2;
	int K = X->size[2] +  pad *2;
	Tensor * Xpad = new Tensor(X->size[0], N,K);
	for(int z = 0; z < X->size[0]; z++){
		float ** xpad = (*Xpad)[z];
		float ** x = (*X)[z];
		for(int i = 0; i < N ;i++){
			for(int j = 0; j < K; j++){
				if((i >= pad) && (i < (N-pad)) &&
							(j >=  pad) && (j <  (K- pad)))
					xpad[i][j] = x[i-pad][j-pad];
			}
		}
	}
	return Xpad;
}

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
        		float sum = 0;
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

int Tensor::read(FILE * f)
{
	uint32_t dims[3];
	for(int i = 0; i < 3 ;i++){
		if( 1 != fread(&(dims[i]),sizeof(dims[0]),1,f))
			return TENSOR_READ_FAILED;
	}

	int retval = TENSOR_READ_SUCCSEFULL;
	if(dims[0] != size[0] || (dims[1] != size[1]) || (dims[2] != size[2])){
		retval = TENSOR_READ_RESIZED;
		resize(dims[0],dims[1],dims[2]);
	}
	uint32_t num_params = dims[0] * dims[1] * dims[2];
	if(num_params != fread(&(data[0][0][0]),sizeof(data[0][0][0]),num_params,f))
		return TENSOR_READ_FAILED;
	return retval;
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

//Returns 1 if realloc was necessary
void reallocIfNecessary(float ** out, uint32_t size[], uint32_t new_size[], int dimensions)
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
int readTensorRaw(FILE * f, float ** out, uint32_t size[3])
{
	uint32_t num_params = size[0] * size[1] * size[2];
	if(num_params != fread(&(*out[0]),sizeof(float),num_params,f))
		return 0;
	return 1;
}

//Returns 0 on failure
int readConvRaw(float ** X, uint32_t X_size[3], float ** Ref, uint32_t Ref_size[3], float ** W, uint32_t W_size[4], float ** B, uint32_t B_size[3], FILE * f)
{
    uint32_t new_X_size[3];
    if(!readTensorSize(f, new_X_size)) return 0;
    reallocIfNecessary(X, X_size, new_X_size, 3); 
    if(!readTensorRaw(f, X, X_size)) return 0;

    uint32_t new_Ref_size[3];
    if(!readTensorSize(f, new_Ref_size)) return 0;
    reallocIfNecessary(Ref, Ref_size, new_Ref_size, 3); 
	if(!readTensorRaw(f, Ref, Ref_size)) return 0;

    uint32_t new_W_size[3] = {Ref_size[0]};
    if(!readTensorSize(f, &new_W_size[1])) return 0;
    reallocIfNecessary(W, W_size, new_W_size, 4);
	for(int i = 0; i < Ref_size[0] ; i++){
    	if(i != 0) {
        	if(!readTensorSize(f, &new_W_size[1])) return 0;
    	}
		if(!readTensorRaw(f, &W[i*W_size[1]*W_size[2]*W_size[3]], &W_size[1])) return 0;
	}

    uint32_t new_B_size[3];
    if(!readTensorSize(f, new_B_size)) return 0;
    reallocIfNecessary(B, B_size, new_B_size, 3); 
    if(!readTensorRaw(f, B, B_size)) return 0;

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
			//Use FPGA for Conv2D_3x3:
    		//        in_sm, w_sm,	b_sm, in_w,      in_h,      in_c,      out_c,     out_sm
        	EntryConv(X,     W,     B,    X_size[1], X_size[2], X_size[0], R_size[0], Z     );
		}
		compareTensorsRaw(Z,R_size,R,R_size,1e-3);
	}
	delete [] X;
	delete [] R;
	delete [] W;
	delete [] B;
	fclose(f);
}

int main(){
	testConv("/home/clyybber/projects/uni/cnnhw/final/ACCN-Homework/final_project/data/conv_test.dat");
	/*
	for(int i = 0; i < ntests; i++){
		EntryConv(x + (i*INPUT_SIZE),w + (i*KERNEL_SIZE), z + (i*OUTPUT_SIZE));
	}*/
	printf("All values are equal!\n");
	return 0;
}

