#include "conv.h"
#include <stdio.h>
#include <cstdlib>


#define IS (OS+KS-1)
template<int KS, int IN_C, int OS, int out_c>
void Conv2D(dt IN[IN_C][IS][IS],
	dt W[out_c][IN_C][KS][KS],
	dt B[out_c],
	dt OUT[out_c][OS][OS])
{
    //int IS = OS+KS-1;
	/* Store in BRAM */
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
						in[p][q] = IN[ic][y+p][q];	  // in[p][q] = IN[ic][y + p][q];
					}
				}

				int xmk = 0;
				forEachX: for(int x = 0; x < OS; x++) {
#pragma HLS pipeline II=KS
					dt acc = 0; // create buffer for each kernel_size conv

					// Replace old column with new column for in
					// without shifting the existing columns, by indexing using (x + q) % KS
					loadINSingleColumn: for(int p = 0; p < KS; p++) {
//#pragma HLS pipeline II=1
						int p_idx = p * KS;
						int y_plus_p_IN_idx = (y+p) * IS;
						int xmk_plus_KS_minus_1_mod_KS = xmk+KS-1;
						if(xmk_plus_KS_minus_1_mod_KS >= KS) xmk_plus_KS_minus_1_mod_KS -= KS;
						in[p][xmk_plus_KS_minus_1_mod_KS] = IN[ic][y+p][x+KS-1];
					}

					// Conv calculation
					conv1: for(int p = 0; p < KS; p++) {

						conv2: for (int q = 0; q < KS; q++) {
//#pragma HLS unroll
    						int xmk_plus_q_mod_KS = xmk+q;
    						if(xmk_plus_q_mod_KS >= KS) xmk_plus_q_mod_KS -= KS;
							acc += in[p][xmk_plus_q_mod_KS] * w[ic][p][q]; // in[p][q] * w[p][q];
						}
					}

					acc_row[x] += acc;

					xmk++;
					if(xmk >= KS) xmk -= KS;
				} // for x

			} // for ic

			// add bias and send one row to OUT (in DRAM)
			writeRowToOUT: for(int x = 0; x < OS; x++) {
#pragma HLS pipeline II=1
				OUT[oc][y][x] = acc_row[x] + b[oc]; // OUT[oc][y][x] = acc_row[x] + B[oc];
			}
		} //for y
	}
}


void EntryConv(dt IN[MAX_IN_CHANNEL*MAX_IN_SIZE*MAX_IN_SIZE],
	dt W[OUT_CHANNEL*MAX_IN_CHANNEL*MAX_KERNEL_SIZE*MAX_KERNEL_SIZE],
	dt B[OUT_CHANNEL],
	int layer_choose,
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

    #pragma HLS INTERFACE s_axilite port=layer_choose


	//Medium net:
	// ConvLayer(3,96,128,128,7,3),
	// Block 2
	// ConvLayer(96,256,64,64,5,2),
	// Block 3
	// ConvLayer(256,384,32,32,3,1),
	// Block 4
	// ConvLayer(384,384,14,14,3,0),
	// Block 5
	// ConvLayer(384,256,12,12,3,0),
    switch(layer_choose) {
        case 1:
            Conv2D<7, 3, 128, 96>(
                (float (*)[134][134]) IN,
                (float (*)[3][7][7]) W,
                B,
                (float (*)[128][128]) OUT);
        	break;
		case 2:
            Conv2D<5, 96, 64, 256>(
                (float (*)[68][68]) IN,
                (float (*)[96][5][5]) W,
                B,
                (float (*)[64][64]) OUT);
        	break;
        case 3:
            Conv2D<3, 256, 32, 384>(
                (float (*)[34][34]) IN,
                (float (*)[256][3][3]) W,
                B,
                (float (*)[32][32]) OUT);
        	break;
		case 4:
            Conv2D<3, 384, 14, 384>(
                (float (*)[16][16]) IN,
                (float (*)[384][3][3]) W,
                B,
                (float (*)[14][14]) OUT);
            break;
		case 5:
            Conv2D<3, 384, 12, 256>(
                (float (*)[14][14]) IN,
                (float (*)[384][3][3]) W,
                B,
                (float (*)[12][12]) OUT);
        	break;
    }

}
