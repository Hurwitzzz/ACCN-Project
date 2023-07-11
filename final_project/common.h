#ifndef common_h_INCLUDED
#define common_h_INCLUDED


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
    	printf("Tensor dimensions don't match ! \n");
		return -1;
	}
	int ret = 0;
	double abs_diff = 0;
	for(int z = 0; z < a_size[0]; z++){
	for(int y = 0; y < a_size[1]; y++){
	for(int x = 0; x < a_size[2]; x++){
		int index = z*(a_size[1]*a_size[2]) + y*(a_size[2]) + x;
		float diff = Fabs( a[index] - ref[index] );
		if(diff > limit) {
			if(ret == 0) printf("Tensors differ at: [%d][%d][%d] by %f \n",z,y,x,diff);
			ret++;
		}
		abs_diff += diff;
	}}}
	printf("Total avg diff: %lf\n",abs_diff/(1.f * a_size[0] * a_size[1] * a_size[2]));
	if(ret == 0){
		printf("Tensors are equal!\n");
	}
	return ret;
}

// Iterates in oc->y->x->ic->ky->kx order
void convBasic(float* X, uint32_t X_size[3],
               float* W, uint32_t W_size[4],
               float* B, uint32_t B_size[3],
               float* Z, uint32_t Z_size[3])
{
	int in_c = X_size[0];
	int out_c = Z_size[0];
	int in_h = X_size[1];
	int in_w = X_size[2];
	int kernel_h = W_size[2];
	int kernel_w = W_size[3];
	for (int oc = 0; oc < out_c; oc++) {
		for (int y = 0; y < in_h - kernel_h + 1; y++) {
			for (int x = 0; x < in_w - kernel_w + 1; x++) {
				float sum = 0;
				for (int ic = 0; ic < in_c; ic++) {
					for (int ky = 0; ky < kernel_h; ky++) {
						for (int kx = 0; kx < kernel_w; kx++) {
							sum += X[ic*in_h*in_w+(y+ky)*in_w+x+kx] *
							       W[oc*in_c*kernel_h*kernel_w+ic*kernel_h*kernel_w+ky*kernel_w+kx];
						}
					}
				}
				Z[oc*Z_size[1]*Z_size[2]+y*Z_size[2]+x] = sum + B[oc];
			}
		}
	}
}

// Iterates in oc->ic->y->x->ky->kx order
void convBasic2(float* X, uint32_t X_size[3],
               float* W, uint32_t W_size[4],
               float* B, uint32_t B_size[3],
               float* Z, uint32_t Z_size[3])
{
	int in_c = X_size[0];
	int out_c = Z_size[0];
	int in_h = X_size[1];
	int in_w = X_size[2];
	int kernel_h = W_size[2];
	int kernel_w = W_size[3];
	int out_h = in_h - kernel_h + 1;
	int out_w = in_w - kernel_w + 1;
	for (int oc = 0; oc < out_c; oc++) {
		for (int y = 0; y < out_h; y++) {
			for (int x = 0; x < out_w; x++) {
				Z[oc*Z_size[1]*Z_size[2]+y*Z_size[2]+x] = B[oc];
			}
		}
		for (int ic = 0; ic < in_c; ic++) {
			float * sums = new float[out_h * out_w](); // the sums for one input channel, zero-init
			for (int y = 0; y < out_h; y++) {
				for (int x = 0; x < out_w; x++) {
					float sum = 0; // Sum for one input channel for one kernel application/output pixel
					for (int ky = 0; ky < kernel_h; ky++) {
						for (int kx = 0; kx < kernel_w; kx++) {
							sum += X[ic*in_h*in_w+(y+ky)*in_w+x+kx] *
							       W[oc*in_c*kernel_h*kernel_w+ic*kernel_h*kernel_w+ky*kernel_w+kx];
						}
					}
					sums[y*out_w+x] += sum;
				}
			}
			for (int y = 0; y < out_h; y++) {
				for (int x = 0; x < out_w; x++) {
					Z[oc*Z_size[1]*Z_size[2]+y*Z_size[2]+x] += sums[y*out_w+x];
				}
			}
		}
	}
}

// Iterates in oc->y->ic->x->ky->kx order
void convBasic3(float* X, uint32_t X_size[3],
               float* W, uint32_t W_size[4],
               float* B, uint32_t B_size[3],
               float* Z, uint32_t Z_size[3])
{
	int in_c = X_size[0];
	int out_c = Z_size[0];
	int in_h = X_size[1];
	int in_w = X_size[2];
	int kernel_h = W_size[2];
	int kernel_w = W_size[3];
	int out_h = in_h - kernel_h + 1;
	int out_w = in_w - kernel_w + 1;
	for (int oc = 0; oc < out_c; oc++) {
		for (int y = 0; y < out_h; y++) {
			float * sums = new float[out_w](); // One row of output
			for (int ic = 0; ic < in_c; ic++) {
				for (int x = 0; x < out_w; x++) {
					float sum = 0; // Sum for one input channel for one kernel application/output pixel
					for (int ky = 0; ky < kernel_h; ky++) {
						for (int kx = 0; kx < kernel_w; kx++) {
							sum += X[ic*in_h*in_w+(y+ky)*in_w+x+kx] *
							       W[oc*in_c*kernel_h*kernel_w+ic*kernel_h*kernel_w+ky*kernel_w+kx];
						}
					}
					sums[x] += sum;
				}
			}
			for (int x = 0; x < out_w; x++) {
				Z[oc*Z_size[1]*Z_size[2]+y*Z_size[2]+x] = sums[x] + B[oc];
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

FILE * openTestFile() {
    FILE * f = fopen("data/conf_test.dat", "rb");
    // Try to use absolute path for Vitis HLS
    if(!f) f = fopen("/home/clyybber/projects/uni/cnnhw/final/ACCN-Homework/final_project/data/conv_test.dat","rb");
    if(!f) f = fopen("/home/hewei/TUM/ACCN/ACCN-Homework/final_project/data/conv_test.dat", "rb");
    if(!f) printf("test.dat not found\n");
	return f;
}

#endif // common_h_INCLUDED
