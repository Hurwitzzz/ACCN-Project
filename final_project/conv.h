#define X_SIZE 50 
#define X_CHANNEL 3
#define W_SIZE 30 
#define W_CHANNEL (X_CHANNEL)
#define Z_SIZE 50 
#define Z_CHANNEL 3
#define B_SIZE 20 
#define B_CHANNEL Z_CHANNEL
#define KERNEL_SIZE 3

// Declare the function
float get_x(float tensor[KERNEL_SIZE*KERNEL_SIZE],int row, int column);
float get_X(float tensor[X_CHANNEL*X_SIZE*X_SIZE], int ch, int row, int column);
float get_w(float tensor[KERNEL_SIZE*KERNEL_SIZE], int row, int column);
float get_W(float tensor[], int z_ch, int ch, int row, int column);
float get_z(float tensor[], int ch, int row, int column);


// void EntryConv( const float X[X] ,const float W[M],  float Z[ZM]);