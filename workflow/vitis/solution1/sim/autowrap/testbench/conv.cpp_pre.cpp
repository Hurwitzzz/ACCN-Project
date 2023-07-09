# 1 "/home/clyybber/projects/uni/cnnhw/final/ACCN-Homework/workflow/conv.cpp"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 1 "<command-line>" 2
# 1 "/home/clyybber/projects/uni/cnnhw/final/ACCN-Homework/workflow/conv.cpp"
# 1 "/home/clyybber/projects/uni/cnnhw/final/ACCN-Homework/workflow/conv.h" 1





void EntryConv( const float X[100] ,const float W[3], float Z[(100 - 3 + 1)]);
# 2 "/home/clyybber/projects/uni/cnnhw/final/ACCN-Homework/workflow/conv.cpp" 2


void conv1D(const float X[100] ,const float W[3], float Z[(100 - 3 + 1)])
{
 float x[3];
 float w[3];
 for(int i = 0; i < 3; i++){
  w[i] = W[i];
  if(i > 0)
   x[i] = X[i - 1];
 }
OL: for(int i = 0 ; i < (100 - 3 + 1); i++){
#pragma HLS PIPELINE II=1
  float acc[3];
IL: for(int j = 0; j< (3); j++){
#pragma HLS UNROLL
   x[j] = (j == 3 - 1) ? X[i + j] : x[ j + 1];
   acc[j] = x[j] * w[j];
  }
  Z[i] = acc[0] + acc[1] + acc[2];
 }
}


void EntryConv(const float X[100] ,const float W[3], float Z[(100 - 3 + 1)])
{
#pragma HLS interface m_axi port=X,W
#pragma HLS interface m_axi port=Z
#pragma HLS interface s_axilite port=return
#pragma HLS interface s_axilite port=X,W
#pragma HLS interface s_axilite port=Z
 conv1D(X,W,Z);
}
