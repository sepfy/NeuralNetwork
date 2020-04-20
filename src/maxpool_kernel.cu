#include "layers.h"

__global__ void maxpool_forward_gpu_kernel(float *output, float *input, float *indexes, int H, int W, int C, int FH, int FW, int FC, int out_h, int out_w, int stride, int size) {


  //int b = blockIdx.x;
  //int i = threadIdx.x;
  //int j = threadIdx.y;

  int index = blockIdx.x*blockDim.x + threadIdx.x;
  if(index > size) return;

  int b = index / (out_w*out_h);
  int i = index / out_w % out_h;
  int j = index % out_w;

       for(int k = 0; k < FC; k++) {

          float max = -1.0e+6;
          int max_idx = -1;
          for(int n = 0; n < FH; n++) {
            for(int m = 0; m < FW; m++) {
              int im_row = i*stride + n;
              int im_col = j*stride + m;
              int idx = b*H*W*C + im_row*W*C + im_col*C + k;
              max_idx = (input[idx] > max) ? idx : max_idx;
              max = (input[idx] > max) ? input[idx] : max;
            }
          }

          int out_idx = b*out_h*out_w*FC + i*out_w*FC + j*FC + k;
          output[out_idx] = max;
          indexes[out_idx] = max_idx;
        }
}



void MaxPool::forward_gpu() {

  int out_w = (W + 2*pad - FW)/stride + 1;
  int out_h = (H + 2*pad - FH)/stride + 1;
  //dim3 d = {(unsigned int)out_w, (unsigned int)out_h, 1};
  int size = out_w*out_h*batch;
  maxpool_forward_gpu_kernel<<<default_grid(size), BLOCK>>>(output, input, indexes, H, W, C, FH, FW, FC, out_w, out_h, stride, size);
  check_error(cudaGetLastError());
}

__global__ void maxpool_backward_gpu_kernel(float *m_delta, float *delta, float *indexes, int size) {
    
    int i = blockIdx.x*blockDim.x + threadIdx.x;
    if(i > size) return;
    int j = indexes[i];
    m_delta[j] = delta[i];
}

void MaxPool::backward_gpu(float *delta) {

  int size = out_w*out_h*FC*batch;
  maxpool_backward_gpu_kernel<<<default_grid(size), BLOCK>>>(m_delta, delta, indexes, size);
  check_error(cudaGetLastError());
}