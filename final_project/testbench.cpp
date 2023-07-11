#include <cstdio>
#include "conv.h"
#include <cstdlib>
#include <time.h>
#include <cmath>
#include "stdint.h"
#include <stdio.h>
#include "common.h"

int main(){
    auto f = openTestFile();
    if(!f) return 1;

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
			if(X_size[0] > IN_CHANNEL || X_size[1] > IN_SIZE || X_size[2] > IN_SIZE || R_size[0] > OUT_CHANNEL) {
				printf("Input/Output too big\n"); continue;
			}
			//Use FPGA for Conv2D_3x3:
    		//        in_sm, w_sm,	b_sm, in_w,      in_h,      in_c,      out_c,     out_sm
        	EntryConv(X,     W,     B,    X_size[1], X_size[2], X_size[0], R_size[0], Z     );
			//convBasic(X,X_size, W,W_size, B,B_size, Z,R_size);
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

	return 0;
}

