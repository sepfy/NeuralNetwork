#include "layers.h"

Shortcut::Shortcut(int _w, int _h, int _c, Convolution *_conv, Activation *_activation) {
  w = _w;
  h = _h;
  c = _c;
  conv = _conv;
  activation = _activation;
}

Shortcut::~Shortcut() {

}

void Shortcut::print() {}

void Shortcut::init() {

#ifdef GPU
  output = malloc_gpu(batch*h*w*c);
  m_delta = malloc_gpu(batch*h*w*c);
  identity = activation->output;

#else
  output = new float[batch*h*w*c];
  m_delta = new float[batch*h*w*c];
  identity = activation->output;
#endif
}



void Shortcut::forward() {


#ifdef GPU
  forward_gpu();
#else
  for(int b = 0; b < batch; b++) {
    for(int i = 0; i < h; i++) {
      for(int j = 0; j < w; j++) {
        for(int k = 0; k < c; k++) {
          int out_idx = b*h*w*c + i*w*c + j*c + k;
          output[out_idx] = input[out_idx] + identity[out_idx];
        }
      }
    }
  }
#endif

}

void Shortcut::backward(float *delta) {

#ifdef GPU
  backward_gpu(delta);
#else
  for(int b = 0; b < batch; b++) {
    for(int i = 0; i < h; i++) {
      for(int j = 0; j < w; j++) {
        for(int k = 0; k < c; k++) {
          int out_idx = b*h*w*c + i*w*c + j*c + k;
          m_delta[out_idx] = delta[out_idx];
          activation->cut[out_idx] = delta[out_idx];
        }
      }
    }
  }
#endif
}

void Shortcut::update(update_args a) {
}

void Shortcut::save(fstream *file) {
}


Shortcut* Shortcut::load(char* buf) {

}


