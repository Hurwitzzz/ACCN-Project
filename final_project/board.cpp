#include <cstdio>
#include <stdint.h>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "conv.h"
#include "common.h"

extern "C"{
#include "pynq_api.h"
}

int main(){
    auto f = openTestFile();
    if(!f) return 1;

	/* connect to the leds */
	PYNQ_loadBitstream((char * )"conv2d.bit");
	PYNQ_MMIO_WINDOW led, hls;  // hls is AXILITE interface
	PYNQ_createMMIOWindow(&led, 0x40010000,8); //in this functino, it uses mmap; the address can be got from vivado; 8 is length
	PYNQ_createMMIOWindow(&hls, 0x40000000,64); //0x00 to 0x2c is 32+12=44, plus 4bytes, 48 is enough. We give 64. It will allocate the whole page anyway, so it doesn't really matter how big you set this, as long as you don't go over 4kB; This is the MMIOwindow for hls_slave, we can write to here.

	/* Alocating shared memory */
	PYNQ_SHARED_MEMORY sm_x, sm_w, sm_b, sm_z;
	PYNQ_allocatedSharedMemory(&sm_x, IN_CHANNEL * IN_SIZE * IN_SIZE * sizeof(float), 0);
	PYNQ_allocatedSharedMemory(&sm_w, OUT_CHANNEL * IN_CHANNEL * KERNEL_SIZE * KERNEL_SIZE * sizeof(float), 0);
	PYNQ_allocatedSharedMemory(&sm_b, OUT_CHANNEL * sizeof(float), 0);
	PYNQ_allocatedSharedMemory(&sm_z, OUT_CHANNEL * OUT_SIZE * OUT_SIZE * sizeof(float), 0);

	/* write to the address */
	// The structure of var"led" can be got from doc
	uint32_t * b_led = (uint32_t *) led.buffer;
	b_led[1] = 0; // the TRI register
	b_led[0] = 3; // DATA register

	/* Now can write the input data to the memory address */
	// One cannot write to the pointer to virtual memory, so cast it to float * first
	float * virt_x = (float *) sm_x.pointer;
	float * virt_w = (float *) sm_w.pointer;
	float * virt_z = (float *) sm_z.pointer;

	/* Copy addr over axilite */
	// These can be found in the HLS Summary
	memcpy(hls.buffer + 0x10, &(sm_x.physical_address), sizeof(size_t));
	memcpy(hls.buffer + 0x1c, &(sm_w.physical_address), sizeof(size_t));
	memcpy(hls.buffer + 0x28, &(sm_b.physical_address), sizeof(size_t));
	memcpy(hls.buffer + 0x34, &(sm_z.physical_address), sizeof(size_t));

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
    	printf("Test X:[%dx%dx%d] W:[%dx%d] Output channels: %d!\n",X_size[0],X_size[1],X_size[2], W_size[2],W_size[3],R_size[0]);

    		//convBasic(&X,W,&B,&Z);
    	if(W_size[2] == 3 && W_size[3] == 3) {
    		if(X_size[0] > IN_CHANNEL || X_size[1] > IN_SIZE || X_size[2] > IN_SIZE || R_size[0] > OUT_CHANNEL) {
        		printf("Input/Output too big\n"); continue;
    		}
    		//Use FPGA for Conv2D_3x3:
			//        in_sm, w_sm,	b_sm, in_w,      in_h,      in_c,      out_c,     out_sm

    	    memcpy(virt_x, X, sizeof(float) * X_size[0]*X_size[1]*X_size[2]);
    	    memcpy(virt_w, W, sizeof(float) * W_size[0]*W_size[1]*W_size[2]*W_size[3]);

    	    /* Start HLS by setting bit */
    	    uint32_t * hls_ctrl = (uint32_t *) hls.buffer;
    	    *hls_ctrl = 0b1;
    	    while((*hls_ctrl & 0b100)){}; // waiting for the IDLE(the 3rd bit) to 1, then we can print out the result
         
    	    memcpy(Z, virt_z, sizeof(float) * R_size[0]*R_size[1]*R_size[2]);





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

	PYNQ_closeMMIOWindow( &led);
	PYNQ_closeMMIOWindow( &hls);
	return 0;
}
