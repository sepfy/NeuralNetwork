#include "layers.h"

Pooling::Pooling(int _W, int _H, int _C,
  int _FW, int _FH, int _FC, int _stride, bool _pad) {

  W = _W;
  H = _H;
  C = _C;
  FW = _FW;
  FH = _FH;
  FC = _FC;
  stride = _stride;

  if(_pad == true) {
    pad = 0.5*((stride - 1)*W - stride + FW);
    out_w = W;
    out_h = H;
  }
  else {
    pad = 0;
    out_w = (W - FW)/stride + 1;
    out_h = (H - FH)/stride + 1;
  }



  out_channel = FW*FH*C;
}

Pooling::~Pooling() {

}

void Pooling::init() {

#ifdef GPU

  output = malloc_gpu(batch*out_w*out_h*FC);
  m_delta = malloc_gpu(batch*H*W*C);
  indexes = malloc_gpu(batch*out_w*out_h*FC);

#else
  col = new float[out_w*out_h*out_channel];
  output = new float[batch*out_w*out_h*FC];
  out_col = new float[out_w*out_h*out_channel*batch];
  im = new float[H*W*C];
  m_delta = new float[batch*H*W*C];
  delta_col = new float[batch*out_channel*out_w*out_h];
  indexes = new float[batch*out_w*out_h*FC];
#endif

  cout << "Max Pooling layer: memory = " << (batch*out_w*out_h*FC*3)/(1024*1024) << endl;
}


void Pooling::forward() {

//ms_t s = getms();

#ifdef GPU
  forward_gpu();
#else
  int out_w = (W + 2*pad - FW)/stride + 1;
  int out_h = (H + 2*pad - FH)/stride + 1;
  int offset_w = 0, offset_h = 0;

  for(int b = 0; b < batch; b++) {
    for(int i = 0; i < out_h; i++) {
      for(int j = 0; j < out_w; j++) {

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
    }
  }
/*
  int col_size = out_w*out_h*out_channel;
  int im_size = H*W*C;

  for(int i = 0; i < batch; i++)
    im2col(W, H, C, FW, FH, C, stride, pad,
           input + i*im_size, out_col + i*col_size);
  
  int out_size = out_h*out_w*C;
  int filter_size = FH*FW;
  int idx;
  float max;
  float max_idx = 0;
  max_seq.clear();

  for(int i = 0; i < batch; i++) {
    for(int p = 0; p < out_h; p++)
      for(int q = 0; q < out_w; q++)
        for(int o = 0; o < C; o++) {
          max = -1.0e+6;
          for(int j = 0; j < filter_size; j++) {
            idx = i*col_size + p*(out_w*out_channel) + q*out_channel + j*C + o;
            //cout << out_col[idx] << " ";
            if(out_col[idx] > max) {
              //out_w*out_h*out_channel*batch
              max = out_col[idx];
              max_idx = idx;
            }
          }
          output[i*out_size+p*(out_w*C)+q*C+o] = max;
          max_seq.push_back(max_idx);
        }
  }  
*/

#endif
//cout << getms() - s << endl;
}

void Pooling::backward(float *delta) {
/*
  int im_size = W*H*C;
  int col_size = out_w*out_h*out_channel;

#ifdef GPU
  float *im = malloc_gpu(W*H*C);
  float *col = malloc_gpu(out_channel*out_w*out_h);
#else
  float *im = new float[W*H*C];
  float *col = new float[out_channel*out_w*out_h];
#endif
*/
#ifdef GPU
  backward_gpu(delta);
#else
  for(int i = 0; i < out_w*out_h*FC*batch; i++) {
    int j = indexes[i];
    m_delta[j] = delta[i];
  }
#endif
  /*
  memset(delta_col, 0.0, batch*out_channel*out_w*out_h); 
  for(int i = 0; i < out_w*out_h*C*batch; i++)
    delta_col[max_seq[i]] = delta[i];
    //m_delta[max_seq[i]] = delta[i];
  //float
  for(int i = 0; i < batch; i++) {
    memcpy(col, delta_col + i*col_size, col_size*sizeof(float)); 
    col2im(W, H, C, FW, FH, C, stride, pad, im, col);
    memcpy(m_delta + i*im_size, im, im_size*sizeof(float));
  }
 */

/*
#ifdef GPU
  cudaFree(im);
  cudaFree(col);
#else
  delete[] im;
  delete[] col; 
#endif
  */  
}

void Pooling::update(update_args a) {
}

void Pooling::save(fstream *file) {
  char buf[64] = {0};
  sprintf(buf, "Pooling,%d,%d,%d,%d,%d,%d,%d,%d",
    W, H, C, FW, FH, FC, stride, pad);
  //cout << buf << endl;
  file->write(buf, sizeof(buf));
}


Pooling* Pooling::load(char* buf) {

  int para[8] = {0};
  int idx = 0;

  char *token;
  while (buf) {
    token = strtok(NULL, ",");
    para[idx] = atoi(token);
    idx++;
    if(idx > 7)
      break;
  }

  Pooling *pool = new Pooling(para[0], para[1],
  para[2], para[3], para[4], para[5], para[6], para[7]);
  return pool;
}


