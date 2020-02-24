#include "layers.h"

__global__ void im2col_gpu_kernel(int W, int H, int C, int FW, int FH, int FC,
            int stride, int pad, float *im, float *col) {

  //int k = blockIdx.x * blockDim.x + threadIdx.x;
  int k = blockIdx.x;
  int i = threadIdx.x;
  int j = threadIdx.y;
  int out_w = (W + 2*pad - FW)/stride + 1;
  //int out_h = (H + 2*pad - FH)/stride + 1;
  int out_col = FH*FW*C;
  int offset_w, offset_h, c_im;
  int im_row, im_col;


//  for(int k = 0; k < out_col; k++) {
    c_im = k % C;
    offset_w = k / C % FW;
    offset_h = k / C / FW;
  
  //  for(int i = 0; i < out_h; i++) {
  //    for(int j = 0; j < out_w; j++) {

        im_row = offset_h + i*stride;
        im_col = offset_w + j*stride;


        int col_idx = (i*out_w + j)*out_col + k;
        int im_pad_row = im_row - pad;
        int im_pad_col = im_col - pad;

        if(im_pad_row < 0 || im_pad_col < 0 ||
           im_pad_row >= H || im_pad_col >= W)
          col[col_idx] = 0.0;
        else {
          int im_idx = C*(im_row*W + im_col) + c_im;
          col[col_idx] = im[im_idx];
        }
//      }
//    }
//  }
 



}

void im2col_gpu(int W, int H, int C, int FW, int FH, int FC,
            int stride, int pad, float *im, float *col) {

  int out_col = FH*FW*C;
  int out_w = (W + 2*pad - FW)/stride + 1;
  int out_h = (H + 2*pad - FH)/stride + 1;
  dim3 d = {(unsigned int)out_w, (unsigned int)out_h, 1};
  im2col_gpu_kernel<<<out_col, d>>>(W, H, C, FW, FH, FC, stride, pad, im ,col);
  cudaDeviceSynchronize();
}


__global__ void col2im_gpu_kernel(int W, int H, int C, int FW, int FH, int FC,
            int stride, int pad, float *im, float *col) {

  int out_w = (W + 2*pad - FW)/stride + 1;
  //int out_h = (H + 2*pad - FH)/stride + 1;
  int out_col = FH*FW*C;
  int offset_w, offset_h, c_im;
  int im_row, im_col;

  int k = blockIdx.x;
  int i = threadIdx.x;
  int j = threadIdx.y;

//  for(int k = 0; k < out_col; k++) {

    c_im = k % C;
    offset_w = k / C % FW;
    offset_h = k / C / FW;

//    for(int i = 0; i < out_h; i++) {
//      for(int j = 0; j < out_w; j++) {
        im_row = offset_h + i*stride;
        im_col = offset_w + j*stride;
        int im_idx = C*(im_row*W + im_col) + c_im;
        int col_idx = (i*out_w + j)*out_col + k;
        im[im_idx] = col[col_idx];
//      }
//    }
//  }

}

void col2im_gpu(int W, int H, int C, int FW, int FH, int FC,
            int stride, int pad, float *im, float *col) {

  int out_col = FH*FW*C;
  int out_w = (W + 2*pad - FW)/stride + 1;
  int out_h = (H + 2*pad - FH)/stride + 1;
  dim3 d = {(unsigned int)out_w, (unsigned int)out_h, 1};
  im2col_gpu_kernel<<<out_col, d>>>(W, H, C, FW, FH, FC, stride, pad, im ,col);
  check_error(cudaGetLastError());

  //cudaDeviceSynchronize();
}
