#define KERNEL_SIZE 3
#define IN_SIZE 140 // if possible, set to 128 
#define IN_CHANNEL 512
#define OUT_SIZE IN_SIZE - KERNEL_SIZE + 1
#define OUT_CHANNEL 512

// Declare the function
float get_in(float tensor[KERNEL_SIZE*KERNEL_SIZE],int row, int column)

float get_IN(float tensor[IN_CHANNEL*IN_SIZE*IN_SIZE], int ch, int row, int column, int in_w, int in_h)

float get_w(float tensor[KERNEL_SIZE*KERNEL_SIZE], int row, int column)

float get_W(float tensor[], int z_ch, int ch, int row, int column, int in_c)
float get_z(float tensor[OUT_SIZE * OUT_SIZE], int row, int column, int in_w)


float get_OUT(float tensor[OUT_CHANNEL*OUT_SIZE*OUT_SIZE], int ch, int row, int column, int out_w)


void EntryConv(float in_sm[IN_CHANNEL*IN_SIZE*IN_SIZE],
	float w_sm[OUT_CHANNEL*IN_CHANNEL][KERNEL_SIZE][KERNEL_SIZE],
	float b_sm[OUT_CHANNEL],
	int in_w, int in_h, int in_c, int out_c,
	float out_sm[OUT_CHANNEL*OUT_SIZE*OUT_SIZE]);
