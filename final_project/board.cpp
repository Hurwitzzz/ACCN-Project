#include <cstdio>
#include <stdint.h>
#include <cstdlib>
#include <cstring>
#include <cmath>

extern "C"{
#include "pynq_api.h"
        }

#define INPUT_SIZE  100
#define KERNEL_SIZE 3
#define OUTPUT_SIZE (INPUT_SIZE - KERNEL_SIZE + 1)

void loaddat(FILE * f, int ntest,  float * x , float * w, float * ref);


int main(){

        /* connect to the leds */
        PYNQ_loadBitstream((char * )"conv2d.bit");
        PYNQ_MMIO_WINDOW led, hls;  // hls is AXILITE interface
        PYNQ_createMMIOWindow(&led, 0x40010000,8); //in this functino, it uses mmap; the address can be got from vivado; 8 is length
        PYNQ_createMMIOWindow(&hls, 0x40000000,64); //0x00 to 0x2c is 32+12=44, plus 4bytes, 48 is enough. We give 64. It will allocate the whole page anyway, so it doesn't really matter how big you set this, as long as you don't go over 4kB; This is the MMIOwindow for hls_slave, we can write to here.

        /* Alocating shared memory */
        PYNQ_SHARED_MEMORY sm_x, sm_w, sm_z;
        PYNQ_allocatedSharedMemory(&sm_x, INPUT_SIZE * sizeof(float), 0);
        PYNQ_allocatedSharedMemory(&sm_w, KERNEL_SIZE * sizeof(float), 0);
        PYNQ_allocatedSharedMemory(&sm_z, OUTPUT_SIZE * sizeof(float), 0);

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
        memcpy(hls.buffer + 0x10, &(sm_x.physical_address), sizeof(size_t)); // 0x10 can be found in hls' synthsis summary
        memcpy(hls.buffer + 0x1c, &(sm_w.physical_address), sizeof(size_t));
        memcpy(hls.buffer + 0x28, &(sm_z.physical_address), sizeof(size_t));

        /* Load the test data */
        FILE * f = fopen("test.dat", "r");
        int ntests;
        fscanf(f,"%d",&ntests);
    	float *z = new float[OUTPUT_SIZE * ntests];
    	float *ref = new float[OUTPUT_SIZE * ntests];
    	float *x = new float[INPUT_SIZE * ntests];
    	float *w = new float[KERNEL_SIZE * ntests];
    	loaddat(f,ntests,x,w,ref);

        for(int i = 0; i < ntests; i++) {
            memcpy(virt_x, x + i*INPUT_SIZE, sizeof(float) * INPUT_SIZE);
            memcpy(virt_w, w + i*KERNEL_SIZE, sizeof(float) * KERNEL_SIZE);

            /* Start HLS by setting bit */
            uint32_t * hls_ctrl = (uint32_t *) hls.buffer;
            *hls_ctrl = 0b1;
            while((*hls_ctrl & 0b100)){}; // waiting for the IDLE(the 3rd bit) to 1, then we can print out the result
     
            memcpy(z + i * OUTPUT_SIZE, virt_z, sizeof(float) * OUTPUT_SIZE);
        }

		for(int n = 0; n < ntests; n++) {
        	for (int i = 0; i < OUTPUT_SIZE; i++){
            	if(fabs(ref[OUTPUT_SIZE*n + i] - z[OUTPUT_SIZE*n + i]) > 1e-4) {
					printf("Error %d: z:%f != ref:%f \n",i,z[OUTPUT_SIZE*n + i],ref[OUTPUT_SIZE*n + i]);
					return 1;
            	}
        	}
		}

    	printf("All values are equal!\n");
        // don't forget to free the memory
        delete [] ref;
        delete [] x;
        delete [] w;
        delete [] z;
        PYNQ_closeMMIOWindow( &led);
        PYNQ_closeMMIOWindow( &hls);
        return 0;
}


void loaddat(FILE * f, int ntest,  float * x , float * w, float * ref)
{
	for(int j = 0; j < ntest ; j++){
		for(int i = 0 ; i < INPUT_SIZE; i++){
			fscanf(f,"%f",&(x[j*INPUT_SIZE + i]));
		}
		for(int i = 0 ; i < KERNEL_SIZE; i++){
			fscanf(f,"%f",&(w[j*KERNEL_SIZE + i]));
		}
		for(int i = 0 ; i < OUTPUT_SIZE; i++){
			fscanf(f,"%f",&(ref[j*OUTPUT_SIZE + i]));
		}
	}
}
