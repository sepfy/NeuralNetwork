#include <iostream>
#include "layers.h"
#include "gemm.h"
#include "blas.h"
#include "utils.h"
using namespace std;

bool gemm_test() {
  cout << "Test gemm" << endl;
  int M = 3;
  int N = 2;
  int P = 3;
  float A[] = { 3, 4, 1, 2, 1, 3, 2, 2, 1}; //3x3
  float B[] = { 1, 2, 3, 2, 1, 2};  //3x2
  float D[] = {32, 32, 16, 24, 18, 20}; //3x2
  float alpha = 2;
  float *C = new float[N*P];
  memset(C, 0, N*P*sizeof(float));
  gemm(M, N, P, alpha, A, B, C);
/* 
  for(int i = 0; i < N; i++) {
    for(int j = 0; j < P; j++) {
      cout << C[i*P+j] << " ";
    }
    cout << endl;
  }
*/
  cout << compare(M, N, C, D) << endl;
  
}

bool gemm_ta_test() {
  cout << "Test gemm_ta" << endl;

  int M = 2;
  int N = 2;
  int P = 3;
/*
 A = 3 4    A = 3 1 1
     1 2        4 2 3
     1 3

 B = 1 2
     3 2
     1 2

 D = 14 20
     26 36
*/
  float A[] = { 3, 4, 1, 2, 1, 3}; //3x2
  float B[] = { 1, 2, 3, 2, 1, 2}; //3x2
  float D[] = {14, 20, 26, 36}; // 2x2
  float alpha = 2;
  float *C = new float[N*P];
  memset(C, 0, N*P*sizeof(float));

  gemm_ta(M, N, P, alpha, A, B, C);
 
  for(int i = 0; i < M; i++) {
    for(int j = 0; j < N; j++) {
      cout << C[i*N+j] << " ";
    }
    cout << endl;
  }
  cout << compare(M, N, C, D) << endl;
  
}

bool gemm_tb_test() {
  cout << "Test gemm_tb" << endl;

  int M = 3;
  int N = 3;
  int P = 2;
/*
 A = 3 4    
     1 2      
     1 3

 B = 1 2    B = 1 3 1
     3 2        2 2 2
     1 2

 D = 22 34 22
     10 14 10
     14 18 14
*/
  float A[] = { 3, 4, 1, 2, 1, 3}; //3x2
  float B[] = { 1, 2, 3, 2, 1, 2}; //3x2
  float D[] = {22, 34, 22, 10, 14, 10, 14, 18, 14}; // 3x3
  float alpha = 2;
  float *C = new float[N*P];
  memset(C, 0, N*P*sizeof(float));

  gemm_tb(M, N, P, alpha, A, B, C);
 
  for(int i = 0; i < M; i++) {
    for(int j = 0; j < N; j++) {
      cout << C[i*N+j] << " ";
    }
    cout << endl;
  }
  cout << compare(M, N, C, D) << endl;
}  

void im2col_test() {

  /*
  Input: 
    1 2 1   3 4 2 
    3 4 2   1 2 2
    1 6 2   9 8 7
  Output:
    1 3 2 4 3 1 4 2
    2 4 1 2 4 2 2 2
    ...
  */
  // N H W C
  // N C H W
  cout << "Test im2col" << endl;
  float im[] = {1, 3, 2, 4, 1, 2, 3, 1, 4, 2, 2, 2, 1, 9, 6, 8, 2, 7}; 

  int W = 3, H = 3, C = 2;
  int FW = 2, FH = 2, FC = 2;
  int stride = 1, pad = 0;
  
  int out_w = (W + 2*pad - FW)/stride + 1;  //3-2+1 = 2
  int out_h = (H + 2*pad - FH)/stride + 1;  //3-2+1 = 2
  int channel_out = FW*FH*C;

  float *col = new float[out_w*out_h*channel_out];

  im2col(W, H, C, FW, FH, FC,
             stride, pad, im, col);

  for(int i = 0; i < out_h; i++)
    for(int j = 0; j < out_w; j++) {
      for(int k = 0; k < channel_out; k++) {
        cout << col[(i*out_w+j)*channel_out+k] << " ";
      }  
      cout << endl;

    }

}

void norm_test() {

  float A[] = {1, 3, 2, 2, 2, 2};
  float l1 = 12;
  float l2 = 26;
  float l1_func = L1_norm(3, 2, A);
  float l2_func = L2_norm(3, 2, A);
  if(l1 != l1_func) 
    cout << "L1 norm = " << l1_func << " not equal " << l1 << endl;
  else
    cout << "L1 norm pass" << endl;

  if(l2 != l2_func)
    cout << "L2 norm = " << l2_func << " not equal " << l2 << endl;
  else
    cout << "L2 norm pass" << endl;


}

void pooling_test() {

  float im[] = {2, 3, 4, 2, 
                1, 3, 2, 2, 
                2, 9, 4, 3,
                1, 6, 1, 2};
  // 4 -2 / 2 + 1

  
  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < 4; j++) 
      cout << im[i*4+j] << " ";
    
    cout << endl;
  }
  Pooling pool1(1, 4, 4, 1, 2, 2, 1, 2, 0, im);
  pool1.forward();
  for(int i = 0; i < 4; i++)
    cout << pool1.output[i] << "";
  cout << endl;
}

int main() {

  /*
    A = 3 4 1
        2 1 3
        2 2 1

    B = 1 2
        3 2
        1 2 

    C = 32 32
        16 24
        18 20
  */

  //gemm_test();
  //gemm_ta_test();
  //gemm_tb_test();
  im2col_test();
  //norm_test();
  cout << endl;
  pooling_test();
 
/*
  float b[] = { 1, 3, 2};
//  sgemm(N, M, P, alpha, A, B, C);
  bias_add(N, M, A, b);
//  for(int i = 0; i < N; i++) {
//    memcpy(C+i*P, b, sizeof(b));
//  }
  for(int i = 0; i < N; i++) {
    for(int j = 0; j < M; j++) {
      cout << A[i*M+j] << " ";
    }
    cout << endl;
  }
*/
  return 0;
}