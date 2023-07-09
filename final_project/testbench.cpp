#include <cstdio>
#include "conv.h"
#include <cstdlib>
#include <time.h>
#include <cmath>


#include "tensor.h"

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
        		FLOAT sum = 0;
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

void testConv(const char * infile)
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

		convBasic(&X,W,&B,&Z);
		compareTensors(&Z,&R,1,1e-3);
		delete [] W;
	}
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

