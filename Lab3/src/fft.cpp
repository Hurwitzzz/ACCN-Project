#include "fft.h"

void C_Tensor::allocate(uint32_t dim_z, uint32_t dim_y, uint32_t dim_x)
{
	size[0] = dim_z;
	size[1] = dim_y;
	size[2] = dim_x;
	size_t dim_zy = size[1]*size[0];
	size_t dim_zyx = dim_zy * size[2]; 
	data = new C_FLOAT **[size[0]];
	C_FLOAT ** tmp_y = new C_FLOAT*[dim_zy];
	C_FLOAT * tmp_x = new C_FLOAT[dim_zyx];
	for(uint32_t i = 0; i < size[0]*size[1]; i++){
		tmp_y[i] = &(tmp_x[i * size[2]]);
	}
	for(uint32_t i = 0; i < size[0]; i++){
		data[i] = &(tmp_y[i * size[1]]);
	}
}

C_Tensor::C_Tensor(uint32_t dim_z, uint32_t dim_y, uint32_t dim_x)
{
	allocate(dim_z,dim_y,dim_x);
}

C_Tensor::~C_Tensor()
{
	if(data != NULL){
		if(data[0] != NULL){
			if(data[0][0] != NULL)
				delete [] data[0][0];
			delete [] data[0];
		}
		delete [] data;
	}
}

void fft(C_FLOAT * x_in, C_FLOAT * X_out, int N)
{
}

void ifft(C_FLOAT * x_in, C_FLOAT * X_out, int N)
{
}

void fft2d(C_Tensor * x_in, C_Tensor * X_f)
{
}

void ifft2d(C_Tensor * X_f , C_Tensor * x_out)
{
}


