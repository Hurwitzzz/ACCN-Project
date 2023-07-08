#include <cstdio>
#include "conv.h"
#include <cstdlib>
#include <time.h>
#include <cmath>




void convSW(float * x , float * w, float * z)
{
	for(int i = 0 ; i < ZM; i++){
		z[i] = 0;
		for(int j = 0; j< M; j++){
			z[i] += x[i + j] * w[j];
		}
	}
}


void loaddat(FILE * f, int ntest,  float * x , float * w, float * ref);
int main(){
	FILE * f = fopen("/media/hewei/Ubuntu/workflow/test.dat","r");
	int ntests;
	fscanf(f,"%d",&ntests);
	float *z = new float[ZM * ntests];
	float *ref = new float[ZM * ntests];
	float *x = new float[N * ntests];
	float *w = new float[M * ntests];
	loaddat(f,ntests,x,w,ref);
	for(int i = 0; i < ntests; i++){
		EntryConv(x + (i*N),w + (i*M), z + (i*ZM));
	}
	for(int n =0; n < ntests ; n++){
		for(int i = 0 ; i < ZM; i++){
			if(fabs(ref[ZM*n + i] - z[ZM*n + i]) > 1e-4){
				printf("Error %d: z:%f != ref:%f \n",i,z[ZM*n + i],ref[ZM*n + i]);
				return 1;
			}
		}
	}
	printf("All values are equal!\n");
	delete [] ref;
	delete [] x;
	delete [] w;
	delete [] z;
	return 0;
}







void loaddat(FILE * f, int ntest,  float * x , float * w, float * ref)
{
	for(int j = 0; j < ntest ; j++){
		for(int i = 0 ; i < N ; i++){
			fscanf(f,"%f",&(x[j*N + i]));
		}
		for(int i = 0 ; i < M ; i++){
			fscanf(f,"%f",&(w[j*M + i]));
		}
		for(int i = 0 ; i < ZM ; i++){
			fscanf(f,"%f",&(ref[j*ZM + i]));
		}
	}
}






