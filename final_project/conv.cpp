#include "conv.h"
#include <stdio.h>
#include <cstdlib>

#define IS (OS+KS-1)
//OSX is the half of OS when poolX2 is true, otherwise the same
//OSX is an extra generic param since OUT is dependent on it
template<int KS, int IN_C, int OS, int out_c, bool relu, bool poolX2, int OSX>
void Conv2D(dt IN[IN_C][IS][IS],
	dt W[out_c][IN_C][KS][KS],
	dt B[out_c],
	dt OUT[out_c][OS][OSX])
{
	// Buffer KSxKS part of input image and reuse KS-1 columns in it per output pixel
	// using "modulo indexing", because then we don't need to shift around the KS-1 columns
	// around in the buffer
	dt in[KS][KS];
#pragma HLS array_partition variable=in complete
	dt w[IN_C][KS][KS];
	dt b[out_c];

	loadBias: for(int oc = 0; oc < out_c; oc++) {
#pragma HLS pipeline II=1
		b[oc] = B[oc];
	}

	// Perform convolution
	forEachOC:for(int oc = 0; oc < out_c; oc++) {

		// load w
		int oc_W_idx = oc * IN_C * KS * KS;
		loadKernel: for(int ic = 0; ic < IN_C; ic++) {
#pragma HLS LOOP_FLATTEN
			for(int p = 0; p < KS; p++) {
			for(int q = 0; q < KS; q++) {
				w[ic][p][q] = W[oc][ic][p][q];
			}}
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

				// load all but one column for in
				loadINAtStart1: for(int p = 0; p < KS; p++) {
#pragma HLS unroll
					int p_idx = p * KS;
					int y_plus_p_IN_idx = (y+p) * IS;
					loadInAtStart2: for(int q = 0; q < KS - 1; q++) {
#pragma HLS unroll
						in[p][q] = IN[ic][y+p][q];
					}
				}

				int xmk = 0;
				forEachX: for(int x = 0; x < OS; x++) {
#pragma HLS pipeline II=KS
					dt acc = 0; // create buffer for each KS conv

					// Replace old column with new column for in
					// without shifting the existing columns, by indexing using (x + q) % KS
					loadINSingleColumn: for(int p = 0; p < KS; p++) {
						int p_idx = p * KS;
						int y_plus_p_IN_idx = (y+p) * IS;
						int xmk_plus_KS_minus_1_mod_KS = xmk+KS-1;
						if(xmk_plus_KS_minus_1_mod_KS >= KS) xmk_plus_KS_minus_1_mod_KS -= KS;
						in[p][xmk_plus_KS_minus_1_mod_KS] = IN[ic][y+p][x+KS-1];
					}

					// Conv calculation
					conv1: for(int p = 0; p < KS; p++) {

						conv2: for (int q = 0; q < KS; q++) {
    						int xmk_plus_q_mod_KS = xmk+q;
    						if(xmk_plus_q_mod_KS >= KS) xmk_plus_q_mod_KS -= KS;
							acc += in[p][xmk_plus_q_mod_KS] * w[ic][p][q];
						}
					}

					acc_row[x] += acc;

					xmk++;
					if(xmk >= KS) xmk -= KS;
				} // for x

			} // for ic

			// add bias and send one row to OUT (in DRAM)
			// Only iterate until OS / 2 if pooling on x
			// Without pooling or relu:
			//	OUT[oc][y][x] = acc_row[x] + b[oc];
			if constexpr(poolX2) {
				writeRowToOUTPool: for(int x = 0; x < OSX; x++) {
#pragma HLS pipeline II=1
					// Vitis HLS doesn't support if constexpr(relu)	but for mediumnet there's always a relu and pooling layer following anyways
					dt sum = acc_row[x * 2] + b[oc];
					if(sum < 0) sum = 0;
					dt sum2 = acc_row[x * 2 + 1] + b[oc];
					if(sum2 < 0) sum2 = 0;
					dt max = sum;
					if(sum2 > sum) max = sum2;
					OUT[oc][y][x] = max;
				}
			} else {
				writeRowToOUT: for(int x = 0; x < OS; x++) {
#pragma HLS pipeline II=1
					// Vitis HLS doesn't support if constexpr(relu)	but for mediumnet there's always a relu and pooling layer following anyways
					dt sum = acc_row[x] + b[oc];
					if(sum < 0) sum = 0;
					OUT[oc][y][x] = sum;
				}
			}
		} //for y
	}
}


void EntryConv(dt IN[MAX_IN_CHANNEL*MAX_IN_SIZE*MAX_IN_SIZE],
               dt W[OUT_CHANNEL*MAX_IN_CHANNEL*MAX_KERNEL_SIZE*MAX_KERNEL_SIZE],
               dt B[OUT_CHANNEL],
               dt OUT[OUT_CHANNEL*MAX_OUT_SIZE*MAX_OUT_SIZE],
               int block)
{
    #pragma HLS INTERFACE m_axi port=IN depth = 100000
    #pragma HLS INTERFACE m_axi port=W depth = 100000
    #pragma HLS INTERFACE m_axi port=B depth = 100000
    #pragma HLS INTERFACE m_axi port=OUT depth = 100000

    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS INTERFACE s_axilite port=IN,W
    #pragma HLS INTERFACE s_axilite port=B
    #pragma HLS INTERFACE s_axilite port=OUT

    #pragma HLS INTERFACE s_axilite port=block

	//Medium net:
	// Block 1
	// ConvLayer(3,96,128,128,7,3), Relu, Pool
	// Block 2
	// ConvLayer(96,256,64,64,5,2), Relu, Pool
	// Block 3
	// ConvLayer(256,384,32,32,3,1), Relu, Pool
	// Block 4
	// ConvLayer(384,384,14,14,3,0), Relu
	// Block 5
	// ConvLayer(384,256,12,12,3,0), Relu, Pool
    switch(block) {
        case 1:
            Conv2D<7, 3, 128, 96, true, true, 64>(
                (float (*)[134][134]) IN,
                (float (*)[3][7][7]) W,
                B,
                (float (*)[128][64]) OUT);
        	break;
		case 2:
            Conv2D<5, 96, 64, 256, true, true, 32>(
                (float (*)[68][68]) IN,
                (float (*)[96][5][5]) W,
                B,
                (float (*)[64][32]) OUT);
        	break;
        case 3:
            Conv2D<3, 256, 32, 384, true, true, 16>(
                (float (*)[34][34]) IN,
                (float (*)[256][3][3]) W,
                B,
                (float (*)[32][16]) OUT);
        	break;
		case 4:
            Conv2D<3, 384, 14, 384, true, false, 14>(
                (float (*)[16][16]) IN,
                (float (*)[384][3][3]) W,
                B,
                (float (*)[14][14]) OUT);
            break;
		case 5:
            Conv2D<3, 384, 12, 256, true, true, 6>(
                (float (*)[14][14]) IN,
                (float (*)[384][3][3]) W,
                B,
                (float (*)[12][6]) OUT);
        	break;
    }

}
