// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2023.1 (64-bit)
// Tool Version Limit: 2023.05
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
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
#ifndef HLS_FASTSIM
#ifdef __cplusplus
extern "C"
#endif
void apatb_EntryConv_ir(const float *, const float *, float *);
#ifdef __cplusplus
extern "C"
#endif
void EntryConv_hw_stub(const float *X, const float *W, float *Z){
EntryConv(X, W, Z);
return ;
}
#ifdef __cplusplus
extern "C"
#endif
void refine_signal_handler();
#ifdef __cplusplus
extern "C"
#endif
void apatb_EntryConv_sw(const float *X, const float *W, float *Z){
refine_signal_handler();
apatb_EntryConv_ir(X, W, Z);
return ;
}
#endif
# 34 "/home/clyybber/projects/uni/cnnhw/final/ACCN-Homework/workflow/conv.cpp"

