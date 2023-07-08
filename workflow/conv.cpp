#include "conv.h"


void conv1D(const float X[N] ,const  float W[M], float Z[ZM])
{
	float x[M];
	float w[M];
	for(int i = 0; i < M; i++){
		w[i] = W[i];
		if(i > 0)
			x[i] = X[i - 1];
	}
OL:	for(int i = 0 ; i < ZM; i++){
#pragma HLS PIPELINE II=1
		float acc[3];
IL:		for(int j = 0; j< (M); j++){
//#pragma HLS UNROLL
			x[j]  = (j == M - 1) ? X[i + j] : x[ j + 1];
			acc[j] = x[j] * w[j];
		}
		Z[i] = acc[0] + acc[1] + acc[2];
	}
}


void EntryConv(const float X[N] ,const float W[M], float Z[ZM])
{
	#pragma HLS interface m_axi port=X,W depth=10
	#pragma HLS interface m_axi port=Z depth=10
	#pragma HLS interface s_axilite port=return
	#pragma HLS interface s_axilite port=X,W
	#pragma HLS interface s_axilite port=Z
	conv1D(X,W,Z);
}
