#include "conv.h"
#include <stdio.h>
#include <cstdlib>


#define IS (OS+KS-1)
template<int KS, int IN_C, int OS>
void Conv2D(dt IN[IN_C*IS*IS],
	dt W[OUT_CHANNEL*IN_C*KS*KS],
	dt B[OUT_CHANNEL],
	int out_c,
	dt OUT[OUT_CHANNEL*OS*OS])
{
	/* Store in BRAM */
	dt in[KS*KS];
//#pragma HLS array_partition variable=in complete
	dt w[IN_C*KS*KS];
	dt b[OUT_CHANNEL];

	loadBias: for(int oc = 0; oc < out_c; oc++) {
#pragma HLS loop_tripcount max=OUT_CHANNEL
#pragma HLS pipeline II=1
		b[oc] = B[oc];
	}

	// Perform convolution
	forEachOC:for(int oc = 0; oc < out_c; oc++) {
#pragma HLS loop_tripcount max=OUT_CHANNEL

		// load w
		int oc_W_idx = oc * IN_C * KS * KS;
		loadKernel: for(int i = 0; i < IN_C * KS * KS; i++) {
//#pragma HLS pipeline II=1
			w[i] = W[oc_W_idx+i]; // w = W[oc];
		}

		int oc_OUT_idx = oc * OS * OS;
		forEachY: for(int y = 0; y < OS; y++) {
//#pragma HLS pipeline II=KS
			dt acc_row[OS]; // One row of output
//#pragma HLS array_partition variable=acc_row complete
			int y_idx = y * OS;
			zeroInitAccRow: for(int x = 0; x < OS; x++) {
//#pragma HLS pipeline II=1
				acc_row[x] = 0;
			}

			forEachIN_C: for(int ic = 0; ic < IN_C; ic++) {
//#pragma HLS pipeline
				int ic_IN_idx = ic * IS * IS;
				int ic_W_idx = ic * KS * KS;

				// load all but one column for in
				loadINAtStart1: for(int p = 0; p < KS; p++) {
#pragma HLS unroll
					int p_idx = p * KS;
					int y_plus_p_IN_idx = (y+p) * IS;
					loadInAtStart2: for(int q = 0; q < KS - 1; q++) {
#pragma HLS unroll
						in[p_idx+q] = IN[ic_IN_idx+y_plus_p_IN_idx+q];	  // in[p][q] = IN[ic][y + p][q];
						//in[p*KS+q] = IN[ic * IS*IS + (y+p) * in_w +q];
					}
				}

				int xmk = 0;
				forEachX: for(int x = 0; x < OS; x++) {
#pragma HLS pipeline II=KS
					dt acc_kernel[KS * KS]; // create buffer for each kernel_size conv
#pragma HLS array_partition variable=acc_kernel complete

					// Replace old column with new column for in
					// without shifting the existing columns, by indexing using (x + q) % KS
					loadINSingleColumn: for(int p = 0; p < KS; p++) {
//#pragma HLS pipeline II=1
						int p_idx = p * KS;
						int y_plus_p_IN_idx = (y+p) * IS;
						int xmk_plus_KS_minus_1_mod_KS = xmk+KS-1;
						if(xmk_plus_KS_minus_1_mod_KS >= KS) xmk_plus_KS_minus_1_mod_KS -= KS;
						in[p_idx+ xmk_plus_KS_minus_1_mod_KS] = IN[ic_IN_idx+y_plus_p_IN_idx+x+KS-1];   // in[p][q] = (q == KS - 1) ? IN[ic][y + p][x + q] : in[p][q + 1];
					}

					// Conv calculation
					conv1: for(int p = 0; p < KS; p++) {

						int p_idx = p * KS;
						int y_plus_p_IN_idx = (y+p) * IS;
						conv2: for (int q = 0; q < KS; q++) {
//#pragma HLS unroll
    						int xmk_plus_q_mod_KS = xmk+q;
    						if(xmk_plus_q_mod_KS >= KS) xmk_plus_q_mod_KS -= KS;
							acc_kernel[p_idx+q] = in[p_idx+ xmk_plus_q_mod_KS] * w[ic_W_idx+p_idx+q]; // in[p][q] * w[p][q];
						}
					}

					dt acc = 0;
					// sum up the results of one kernel
					sumKernel: for(int i = 0; i < KS * KS; i++) {
//#pragma HLS unroll
						acc += acc_kernel[i];
					}
					acc_row[x] += acc;

					xmk++;
					if(xmk >= KS) xmk -= KS;
				} // for x

			} // for ic

			// add bias and send one row to OUT (in DRAM)
			writeRowToOUT: for(int x = 0; x < OS; x++) {
#pragma HLS pipeline II=1
				OUT[oc_OUT_idx+y_idx+x] = acc_row[x] + b[oc]; // OUT[oc][y][x] = acc_row[x] + B[oc];
			}
		} //for y
	}
}

/*
std::vector<CNN_layer_struct> largeNet= {
	ConvLayer(3,64,128,128,3,1),
	ReLULayer(),
	PoolLayer(64,64,64),
	// Block 2
	ConvLayer(64,128,64,64,3,1),
	ReLULayer(),
	PoolLayer(128,32,32),
	// Block 3
	ConvLayer(128,256,32,32,3,1),
	ReLULayer(),
	ConvLayer(256,256,32,32,3,1),
	ReLULayer(),
	PoolLayer(256,16,16),
	// Block 4
	ConvLayer(256,512,16,16,3,1),
	ReLULayer(),
	ConvLayer(512,512,16,16,3,1),
	ReLULayer(),
	PoolLayer(512,8,8),
	// Block 5
	ConvLayer(512,512,8,8,3,1),
	ReLULayer(),
	ConvLayer(512,512,8,8,3,1),
	ReLULayer(),
	PoolLayer(512,4,4),
	// Classifier
};
*/
void EntryConv(dt IN[MAX_IN_CHANNEL*MAX_IN_SIZE*MAX_IN_SIZE],
	dt W[OUT_CHANNEL*MAX_IN_CHANNEL*MAX_KERNEL_SIZE*MAX_KERNEL_SIZE],
	dt B[OUT_CHANNEL],
	int in_w, int in_h, int in_c, int out_c,
	dt OUT[OUT_CHANNEL*MAX_OUT_SIZE*MAX_OUT_SIZE])
{
    #pragma HLS INTERFACE m_axi port=IN depth = 10
    #pragma HLS INTERFACE m_axi port=W depth = 10
    #pragma HLS INTERFACE m_axi port=B depth = 10
    #pragma HLS INTERFACE m_axi port=OUT depth = 10

    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS INTERFACE s_axilite port=IN,W
    #pragma HLS INTERFACE s_axilite port=B
    #pragma HLS INTERFACE s_axilite port=OUT

    #pragma HLS INTERFACE s_axilite port=in_w,in_h,in_c,out_c

	int block = in_w;
	//Medium net:
    switch(block) {
        case 1:
        	// ConvLayer(3,96,128,128,7,3),
        	// ReLULayer(),
        	// PoolLayer(96,64,64),
            Conv2D<7, 3, 128>(IN, W, B, out_c, OUT);
        	break;
		case 2:
        	// ConvLayer(96,256,64,64,5,2),
        	// ReLULayer(),
        	// PoolLayer(256,32,32),
            Conv2D<5, 96, 64>(IN, W, B, out_c, OUT);
        	break;
		case 3:
        	// ConvLayer(256,384,32,32,3,1),
        	// ReLULayer(),
        	// PoolLayer(384,16,16),
            Conv2D<3, 256, 32>(IN, W, B, out_c, OUT);
        	break;
		case 4:
        	// ConvLayer(384,384,14,14,3,0),
        	// ReLULayer(),
            Conv2D<3, 384, 14>(IN, W, B, out_c, OUT);
        	break;
		case 5:
        	// ConvLayer(384,256,12,12,3,0),
        	// ReLULayer(),
        	// PoolLayer(256,6,6),
            Conv2D<3, 384, 12>(IN, W, B, out_c, OUT);
        	break;
    }

}
