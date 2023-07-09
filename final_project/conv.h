#define X_SIZE 140 // if possible, set to 128 
#define X_CHANNEL 512
#define W_CHANNEL (X_CHANNEL)
#define Z_SIZE 128 
#define Z_CHANNEL 512
#define B_CHANNEL (Z_CHANNEL)
#define KERNEL_SIZE 3

// Declare the function
float get_x(float tensor[KERNEL_SIZE*KERNEL_SIZE],int row, int column);
float get_X(float tensor[X_CHANNEL*X_SIZE*X_SIZE], int ch, int row, int column);
float get_w(float tensor[KERNEL_SIZE*KERNEL_SIZE], int row, int column);
float get_W(float tensor[], int z_ch, int ch, int row, int column);
float get_z(float tensor[], int ch, int row, int column);

void EntryConv(float x_sm[X_CHANNEL][X_SIZE][X_SIZE],
	float w_sm[W_CHANNEL][KERNEL_SIZE][KERNEL_SIZE][KERNEL_SIZE],
	float b_sm[B_CHANNEL],
	int x_w, int x_h, int x_c, int z_c,
	float z_sm[Z_CHANNEL][Z_SIZE][Z_SIZE]);
