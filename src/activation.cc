#include "layers.h"

#if 0
Sigmoid::Sigmoid(int _N) {
  N = _N;
}

Sigmoid::~Sigmoid() {

}

void Sigmoid::init() {
  output = new float[batch*N];
  m_delta = new float[batch*N];
}

void Sigmoid::forward() {
  for(int i = 0; i < batch; i++) 
    for(int j = 0; j < N; j++) 
      output[i*N+j] = 1.0/(1.0 + exp(-1.0*(input[i*N+j])));
}

void Sigmoid::backward(float *delta) {
  for(int i = 0; i < batch; i++) 
    for(int j = 0; j < N; j++) 
      m_delta[i*N+j] = delta[i*N+j]*(1.0 - output[i*N+j])*output[i*N+j];
}

void Sigmoid::update(update_args a) {

}

void Sigmoid::save(fstream *file) {

}
#endif

SoftmaxWithCrossEntropy::SoftmaxWithCrossEntropy(int _n) {
  N = _n;
}

void SoftmaxWithCrossEntropy::init() {

#ifdef GPU
  output = malloc_gpu(batch*N);
  m_delta = malloc_gpu(batch*N);
#else
  output = new float[batch*N];
  m_delta = new float[batch*N];
#endif

}

SoftmaxWithCrossEntropy::~SoftmaxWithCrossEntropy() {

}    

void SoftmaxWithCrossEntropy::print() {

  float umem = (float)(2*batch*N)/(1024*1024);
  printf("Softmax\t %.2f\n", umem);
}

void SoftmaxWithCrossEntropy::forward() {

  for(int i = 0; i < batch; i++) {
    float tmp = 0;
    float max = 0;
    for(int j = 0; j < N; j++) 
      if(input[i*N+j] > max)
	max = input[i*N+j];

    for(int j = 0; j < N; j++) {
      output[i*N+j] = exp(input[i*N+j] - max);
      tmp += output[i*N+j];
    }
    for(int j = 0; j < N; j++) 
      output[i*N+j] /= tmp;
  }

}

void SoftmaxWithCrossEntropy::backward(float *delta) {

  mat_minus(batch, N, output, delta, m_delta);  
  mat_scalar(batch, N, m_delta, 1.0/(float)batch, m_delta);
}

void SoftmaxWithCrossEntropy::update(update_args a) {
}

void SoftmaxWithCrossEntropy::save(fstream *file) {
  char buf[64] = {0};
  sprintf(buf, "Softmax,%d", N);
  //cout << buf << endl;
  file->write(buf, sizeof(buf));
}

SoftmaxWithCrossEntropy* SoftmaxWithCrossEntropy::load(char *buf) {
  int para = 0;
  char *token;
  token = strtok(NULL, ",");
  para = atoi(token);
  SoftmaxWithCrossEntropy *softmax= new SoftmaxWithCrossEntropy(para);
  return softmax;

}



Activation::Activation(int _N, ACT act) {
  N = _N;
  activation = act;
}

Activation::~Activation() {

}

void Activation::init() {

#ifdef GPU
	
  output = malloc_gpu(batch*N);
  m_delta = malloc_gpu(batch*N);
  cut = malloc_gpu(batch*N);
   
#else
  output = new float[batch*N];
  m_delta = new float[batch*N];
  cut = new float[batch*N];
  memset(cut, 0, sizeof(float)*batch*N);
#endif

}

string act_table[NUM_TYPE] = {"Relu", "Leaky", "SIGM"};

void Activation::print() {
  float umem = (float)(3*batch*N)/(1024*1024);
  printf("%s \t %.2f\n", act_table[activation].c_str(), umem);
}


void Activation::forward() {

  switch(activation) {
    case RELU:
      relu_activate();
    case LEAKY:
      leaky_activate();
  }

}

void Activation::relu_activate() {

  for(int i = 0; i < batch; i++) 
    for(int j = 0; j < N; j++) 
      output[i*N+j] = (input[i*N+j] >= 0 ? input[i*N+j] : 0);
}

void Activation::leaky_activate() {

  for(int i = 0; i < batch; i++) 
    for(int j = 0; j < N; j++) 
      output[i*N+j] = (input[i*N+j] >= 0 ? input[i*N+j] : 0.1*input[i*N+j]);
}

void Activation::backward(float *delta) {

  switch(activation) {
    case RELU:
      relu_backward(delta);
    case LEAKY:
      leaky_backward(delta);
  }

}


void Activation::relu_backward(float *delta) {
  for(int i = 0; i < batch; i++) 
    for(int j = 0; j < N; j++) 
      m_delta[i*N+j] = (cut[i*N+j] + delta[i*N+j])*(input[i*N+j] >= 0);
}

void Activation::leaky_backward(float *delta) {
  for(int i = 0; i < batch; i++) 
    for(int j = 0; j < N; j++) 
      m_delta[i*N+j] = (cut[i*N+j] + delta[i*N+j])*(input[i*N+j] >= 0 ? 1.0 : 0.1);
}

void Activation::update(update_args a) {
}

void Activation::save(fstream *file) {
  char buf[64] = {0};
  sprintf(buf, "Activation,%d,%d", N, activation);
  //cout << buf << endl;
  file->write(buf, sizeof(buf));
}

Activation* Activation::load(char *buf) {

  int para[2] = {0};
  char *token;
  int idx = 0;
  while (buf) {
    token = strtok(NULL, ",");
    para[idx] = atoi(token);
    idx++;
    if(idx > 2)
      break;
  }

  Activation *actv = new Activation(para[0], (ACT)para[1]);
  return actv;
}
