#include "fft.h"
#include <vector>

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


void bit_reverse_copy(C_FLOAT * x_in, C_FLOAT * X_out, int N) {
    int logN = log2(N);
    for (int i = 0; i < N; i++) {
        unsigned int reversed = 0;
        unsigned int temp = i;

        // Calculate the bit-reversed index
        for (int j = 0; j < logN; j++) {
            reversed = (reversed << 1) | (temp % 2);
            temp = temp >> 1;
        }

        // Copy the element to the bit-reversed position
        X_out[reversed] = x_in[i];
    }
}



void myfft(C_FLOAT *x_in, C_FLOAT *X_out, int N) {
    bit_reverse_copy(x_in, X_out, N);
    
    for (int s = 1; (1 << s) <= N; s++) {
        int m = 1 << s;
        C_FLOAT wm = std::exp(C_FLOAT(0, -2.0f * M_PI / m));
        for (int k = 0; k < N; k += m) {
            C_FLOAT w = 1;
            for (int j = 0; j < m / 2; j++) {
                C_FLOAT t = w * X_out[k + j + m / 2];
                C_FLOAT u = X_out[k + j];
                X_out[k + j] = u + t;
                X_out[k + j + m / 2] = u - t;
                w *= wm;
            }
        }
    }
}

void ifft(C_FLOAT *x_in, C_FLOAT *X_out, int N) {
    bit_reverse_copy(x_in, X_out, N);

    for (int s = 1; (1 << s) <= N; s++) {
        int m = 1 << s;
        C_FLOAT wm = std::exp(C_FLOAT(0, 2.0f * M_PI / m));
        for (int k = 0; k < N; k += m) {
            C_FLOAT w = 1;
            for (int j = 0; j < m / 2; j++) {
                C_FLOAT t = w * X_out[k + j + m / 2];
                C_FLOAT u = X_out[k + j];
                X_out[k + j] = u + t;
                X_out[k + j + m / 2] = u - t;
                w *= wm;
            }
        }
    }
    
    for (int i = 0; i < N; ++i) {
        X_out[i] /= N;
    }
}

void fft2d(C_Tensor *x_in, C_Tensor *X_out) {
    C_FLOAT *temp_in = new C_FLOAT[x_in->size[1]];
    C_FLOAT *temp_out = new C_FLOAT[x_in->size[1]];
    
    // FFT along rows
    for (int i = 0; i < x_in->size[0]; ++i) {
        for (int j = 0; j < x_in->size[1]; ++j) {
            myfft(x_in->data[i][j], X_out->data[i][j], x_in->size[2]);
        }
    }

    // FFT along columns
    for (int i = 0; i < x_in->size[0]; ++i) {
        for (int j = 0; j < x_in->size[2]; ++j) {
            for (int k = 0; k < x_in->size[1]; ++k) {
                temp_in[k] = X_out->data[i][k][j];
            }
            
            myfft(temp_in, temp_out, x_in->size[1]);
            
            for (int k = 0; k < x_in->size[1]; ++k) {
                X_out->data[i][k][j] = temp_out[k];
            }
        }
    }

    delete [] temp_in;
    delete [] temp_out;
}

void ifft2d(C_Tensor *x_in, C_Tensor *X_out) {
    C_FLOAT *temp_in = new C_FLOAT[x_in->size[1]];
    C_FLOAT *temp_out = new C_FLOAT[x_in->size[1]];

    for (int i = 0; i < x_in->size[0]; ++i) {
        for (int j = 0; j < x_in->size[1]; ++j) {
            ifft(x_in->data[i][j], X_out->data[i][j], x_in->size[2]);
        }
    }

    for (int i = 0; i < x_in->size[0]; ++i) {
        for (int j = 0; j < x_in->size[2]; ++j) {
            for (int k = 0; k < x_in->size[1]; ++k) {
                temp_in[k] = X_out->data[i][k][j];
            }
            
            ifft(temp_in, temp_out, x_in->size[1]);
            
            for (int k = 0; k < x_in->size[1]; ++k) {
                X_out->data[i][k][j] = temp_out[k];
            }
        }
    }

    delete [] temp_in;
    delete [] temp_out;
}


// void fft2d(C_Tensor * x_in, C_Tensor * X_f)
// {
	
// }

// void ifft2d(C_Tensor * X_f , C_Tensor * x_out)
// {
	
// }


