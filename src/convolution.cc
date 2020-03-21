#include "layers.h"


using namespace std;

Convolution::Convolution(int _W, int _H, int _C,
  int _FW, int _FH, int _FC, int _stride, bool _pad) {

  W = _W;
  H = _H;
  C = _C;
  FW = _FW;
  FH = _FH;
  FC = _FC;
  stride = _stride;


  if(_pad == true) {
    //pad = 0.5*((stride - 1)*W - stride + FW);
    pad = 0.5*(FW - stride);
    //out_w = W/stride;
    //out_h = H/stride;
  }
  else 
    pad = 0;

  out_w = (W + 2*pad - FW)/stride + 1;
  out_h = (H + 2*pad - FH)/stride + 1;


  out_channel = FW*FH*C;
  col_size = out_w*out_h*out_channel;
  im_size = H*W*C;
  weight_size = out_channel*FC;
  bias_size = FC;
  input_size = batch*im_size;

}

Convolution::~Convolution() {

#ifdef GPU

#else
  delete []output;
  delete []weight;
  delete []grad_weight;
  delete []grad_bias;
  delete []m_delta;

  //delete []im;
  //delete []out_col;
  /* Adam optimizer */
  delete []m_weight;
  delete []v_weight;
  delete []m_bias;
  delete []v_bias;

#endif

}

void Convolution::init() {

#ifdef GPU

  output = malloc_gpu(batch*out_w*out_h*FC);

  weight = malloc_gpu(out_channel*FC);
  grad_weight = malloc_gpu(out_channel*FC);
  bias = malloc_gpu(FC);
  grad_bias = malloc_gpu(FC);

  m_delta = malloc_gpu(batch*W*H*C); 

  //out_col = malloc_gpu(out_w*out_h*out_channel*batch);
  //delta_col = malloc_gpu(out_channel*out_w*out_h);

  // Adam optimizer
  m_weight = malloc_gpu(out_channel*FC);
  v_weight = malloc_gpu(out_channel*FC);
  m_bias = malloc_gpu(FC);
  v_bias = malloc_gpu(FC);


  random_normal_gpu(out_channel*FC, weight);
  random_normal_gpu(FC, bias);


#else

  col = new float[out_w*out_h*out_channel];
  output = new float[batch*out_w*out_h*FC];
  out_col = new float[out_w*out_h*out_channel*batch];
  weight = new float[out_channel*FC];
  grad_weight = new float[out_channel*FC];
  bias = new float[FC];
  grad_bias = new float[FC];
  im = new float[H*W*C];
  m_delta = new float[batch*W*H*C]; 
  delta_col = new float[batch*out_channel*out_w*out_h];

  /* Adam optimizer */
  m_weight = new float[out_channel*FC];
  v_weight = new float[out_channel*FC];
  m_bias = new float[FC];
  v_bias = new float[FC];

  random_normal(out_channel*FC, weight);
  random_normal(FC, bias);
  memset(m_weight, 0, out_channel*FC*sizeof(float));
  memset(v_weight, 0, out_channel*FC*sizeof(float));
  memset(m_bias, 0 , FC*sizeof(float));  
  memset(v_bias, 0 , FC*sizeof(float));  


#endif



#ifdef XNOR_NET
  binary_weight = new float[out_channel*FC];
  avg_filter = new float[batch*im_size];
  avg_col = new float[out_w*out_h*FW*FC*batch];
  k_filter = new float[FW*FH];
  k_output = new float[out_w*out_h*batch];
  for(int i = 0; i < FW*FH; i++)
    k_filter[i] = 1.0/(float)(FW*FH);
  mean = new float[FC];

  bitset_outcol = new Bitset[batch*out_h*out_w];
  bitset_weight = new Bitset[FC];

  for(int i = 0; i < batch*out_h*out_w; i++)
    bitset_outcol[i].init(out_channel);

  for(int i = 0; i < FC; i++)
    bitset_weight[i].init(out_channel);

#endif



  shared_size = out_w*out_h*out_channel*batch;
}


void Convolution::print() {

  float umem = (float)(batch*out_w*out_h*FC
	      + 4*out_channel*FC
              + 4*FC
	      + batch*W*H*C)/(1024*1024);
  if(xnor)
    printf("Conv \t %.2f \t %d x %d x %d \t %d x %d x %d \n",  
		  umem, H, W, C, out_h, out_w, FC);
  else 
    printf("Conv \t %.2f \t %d x %d x %d \t %d x %d x %d \n",  
		  umem, H, W, C, out_h, out_w, FC);

}

#ifdef XNOR_NET
void Convolution::swap_weight() {
    float *swap = weight;
    weight = binary_weight;
    binary_weight = swap;
}

float Convolution::binarize_weight() {

  for(int i = 0; i < FC; i++) {
    mean[i] = 0.0;
    for(int j = 0; j < out_channel; j++) 
      mean[i] += fabs(weight[i*out_channel+j]);

    mean[i] /= (float)(out_channel);
    for(int j = 0; j < out_channel; j++) {
      int widx = i*out_channel+j;
       binary_weight[widx] = (weight[widx] > 0) ? 1.0 : -1.0;
    }
  }

}

void Convolution::binarize_input() {

  for(int b = 0; b < batch; b++) {
    for(int i = 0; i < H; i++) {
      for(int j = 0; j < W; j++) {
        int avg_idx = b*H*W + i*W + j;
        int in_idx = b*im_size + i*W*C + j*C;
        avg_filter[avg_idx] = 0.0;
        for(int k = 0; k < C; k++)
          avg_filter[avg_idx] += fabs(input[in_idx + k]);
        avg_filter[avg_idx] /= (float)C;
      }
    }
  }

  for(int i = 0; i < batch*im_size; i++)
    input[i] > 0 ? input[i] = 1 : input[i] = -1;
 
}

#endif

void Convolution::forward_xnor() {
  binarize_input();
  for(int i = 0; i < batch; i++)
    im2col(W, H, C, FW, FH, FC, stride, pad, 
      input + i*im_size, out_col+i*col_size);

  if(!runtime) {

    binarize_weight();
    swap_weight();
    gemm_cpu(TRS_N, TRS_N,
             batch*out_h*out_w, FC, out_channel, 1.0,
              out_col, weight, output);  
  }
  else {

    for(int i = 0; i < batch*out_h*out_w; i++) {
      bitset_outcol[i].clean();
      bitset_outcol[i].set(out_col+i*out_channel);
    }

    bin_gemm(batch*out_h*out_w, FC, out_channel, 1.0, 
      bitset_outcol, bitset_weight, output);
  }

  // Do K = A (*) k
  for(int i = 0; i < batch; i++) 
    im2col(W, H, 1, FW, FH, 1, stride, pad, 
      avg_filter + i*W*H, avg_col + i*out_w*out_h*FW*FH);
  gemm_cpu(TRS_N, TRS_N, batch*out_h*out_w, 1, FW*FH, 1.0, avg_col, k_filter, k_output);

  for(int b = 0; b < batch; b++)
    for(int i = 0; i < out_h; i++)
      for(int j = 0; j < out_w; j++) {
        int idx = b*out_h*out_w+i*out_w+j;
        scalar(FC, k_output[idx],
         output+idx*FC, output+idx*FC);
        for(int k = 0; k < FC; k++)
          output[idx*FC + k] *= mean[k];
      }

  if(xnor && !runtime) {
    swap_weight();
  }
}

void Convolution::forward_full() {

  for(int i = 0; i < batch; i++)
    im2col(W, H, C, FW, FH, FC, stride, pad, 
      input + i*im_size, shared+i*col_size);

  gemm_cpu(TRS_N, TRS_N, batch*out_h*out_w, FC, out_channel, 1, shared, weight, output);
  bias_add();

}

void Convolution::forward() {

  if(xnor)
    forward_xnor();
  else 
    forward_full();

}

void Convolution::bias_add() {
  for(int b = 0; b < batch; b++)
    for(int i = 0; i < out_w*out_h; i++)
        for(int j = 0; j < FC; j++)
          output[b*out_w*out_h*FC + i*FC + j] += bias[j];
}


void Convolution::backward(float *delta) {

  for(int i = 0; i < batch; i++)
    im2col(W, H, C, FW, FH, FC, stride, pad,
      input + i*im_size, shared+i*col_size);

  gemm_cpu(TRS_T, TRS_N, 
           out_channel, FC, out_h*out_w*batch, 1.0,
           shared, delta, grad_weight);

  if(xnor) {

    for(int i = 0; i < out_channel; i++)
      for(int j = 0; j < FC; j++) {
        int idx = i*FC+j;
        grad_weight[idx] = (grad_weight[idx]*(1.0/(float)(out_channel) 
                         + mean[j]*(fabs(weight[idx]) <= 1 ? weight[idx] : 0)))*(1.0 - 1.0/(float)C)*out_channel;
      }

    gemm_cpu(TRS_N, TRS_T,
           batch*out_w*out_h, out_channel, FC, 1.0,
           delta, weight, delta_col);

  }
  else {
    gemm_cpu(TRS_N, TRS_T,
           batch*out_w*out_h, out_channel, FC, 1.0,
           delta, weight, shared);
  }

  row_sum(batch*out_w*out_h, FC, delta, grad_bias);

  for(int i = 0; i < batch; i++)
    col2im(W,H, C, FW, FH, FC, stride, pad, 
      m_delta + i*im_size, shared + i*col_size);

}

void Convolution::update(update_args a) {

#ifdef GPU
  axpy_gpu(out_channel*FC, a.decay, weight, grad_weight);
  //axpy_gpu(FC, a.decay, bias, grad_bias);

  if(a.adam) {
    adam_gpu(out_channel*FC, weight, grad_weight, m_weight, v_weight, a);
    adam_gpu(FC, bias, grad_bias, m_bias, v_bias, a);
  }
  else {
    momentum_gpu(out_channel*FC, weight, grad_weight, v_weight, a);
    momentum_gpu(FC, bias, grad_bias, v_bias, a);
  }

#else
  adam_cpu(out_channel*FC, weight, grad_weight, m_weight, v_weight, a);
  adam_cpu(FC, bias, grad_bias, m_bias, v_bias, a);
#endif


#if 0
  mat_scalar(out_channel, FC, grad_weight, lr, grad_weight);
  mat_minus(out_channel, FC, weight, grad_weight, weight);
  mat_scalar(1, out_w*out_h*FC, grad_bias, lr, grad_bias);
  mat_minus(1, out_w*out_h*FC, bias, grad_bias, bias);
#endif

}

void Convolution::save(fstream *file) {

  char buf[64] = {0};
  sprintf(buf, "Convolution,%d,%d,%d,%d,%d,%d,%d,%d,%d", 
    W, H, C, FW, FH, FC, stride, pad, xnor);
  //cout << weight[0] << endl;
  //cout << bias[0] << endl;
  file->write(buf, sizeof(buf));

  if(xnor) {
    float *BB = new float[FC*out_channel];
    for(int i = 0; i < FC; i++)
      for(int j = 0; j < out_channel; j++)
        BB[i*out_channel+j] = weight[j*FC+i];

    for(int i = 0; i < FC; i++) {
      bitset_weight[i].set(BB+i*out_channel);
    }
    delete[] BB;

    for(int i = 0; i < FC; i++) {
      file->write((char*)bitset_weight[i].bits,
                         bitset_weight[i].N*sizeof(BIT_BLK));
    } 

    file->write((char*)mean, FC*sizeof(float));
  } 
  else {

#ifdef GPU
    float *weight_tmp = new float[weight_size];
    gpu_pull_array(weight, weight_tmp, weight_size);
    file->write((char*)weight_tmp, weight_size*sizeof(float));
    delete []weight_tmp;
#else
    file->write((char*)weight, weight_size*sizeof(float));
#endif

  }



#ifdef GPU
  float *bias_tmp = new float[bias_size];
  gpu_pull_array(bias, bias_tmp, bias_size);
  file->write((char*)bias_tmp, bias_size*sizeof(float));
  delete []bias_tmp;
#else  
  file->write((char*)bias, bias_size*sizeof(float));
#endif

}

Convolution* Convolution::load(char *buf) {

  int para[9] = {0};
  int idx = 0;

  char *token;
  while (buf) {
    token = strtok(NULL, ",");
    para[idx] = atoi(token);
    idx++;
    if(idx > 8)
      break;
  }

  Convolution *conv = new Convolution(para[0], para[1], 
  para[2], para[3], para[4], para[5], para[6], para[7]);
  conv->xnor = para[8];
  return conv;
}
