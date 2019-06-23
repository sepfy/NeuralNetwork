#include "layers.h"

Connected::Connected(int _n, int _m) {
  N = _n;
  M = _m;
}

Connected::~Connected() {

}

void Connected::init() {

  weight = new float[N*M];
  bias   = new float[M];
  output = new float[batch*M];

  grad_weight = new float[N*M];
  grad_bias = new float[M];
  m_delta = new float[batch*N];

  random_normal(N*M, weight);
  random_normal(M, bias);

  //Adam
  m_weight = new float[N*M];
  v_weight = new float[N*M];
  m_bias = new float[M];
  v_bias = new float[M];
  for(int i = 0; i < N*M; i++) {
    m_weight[i] = 0.0;
    v_weight[i] = 0.0;
  }
  for(int i = 0; i < M; i++) {
    m_bias[i] = 0.0;
    v_bias[i] = 0.0;
  }


}


void Connected::forward() {  
  gemm(batch, M, N, 1, input, weight, output);
  bias_add(batch, M, output, bias);
}

void Connected::backward(float *delta) {

  memset(grad_weight, 0, N*M*sizeof(float));
  memset(m_delta, 0, batch*N*sizeof(float));

  gemm_ta(N, M, batch, 1.0, input, delta, grad_weight);
  gemm_tb(batch, N, M, 1.0, delta, weight, m_delta);
  row_sum(batch, M, delta, grad_bias);
}

void Connected::update(float lr) {

  // Adam optimizer
  
  iter++;
  float m_lr = lr * pow(1.0 - pow(beta2, iter), 0.5) / (1.0 - pow(beta1, iter));
  for(int i = 0; i < N*M; i++) {
    m_weight[i] = (1 - beta1)*grad_weight[i] + beta1*m_weight[i];
    v_weight[i] = (1 - beta2)*pow(grad_weight[i], 2.0) + beta2*v_weight[i];
  }

  for(int i = 0; i < N*M; i++) {
    weight[i] -= m_lr * m_weight[i]/(pow(v_weight[i], 0.5) + eplson);
  }


  for(int i = 0; i < M; i++) {
    m_bias[i] = (1 - beta1)*grad_bias[i] + beta1*m_bias[i];
    v_bias[i] = (1 - beta2)*pow(grad_bias[i], 2.0) + beta1*v_bias[i];
  }

  for(int i = 0; i < M; i++) {
    bias[i] -= m_lr * m_bias[i]/(pow(v_bias[i], 0.5) + eplson);
  }
  
#if 0
  mat_scalar(N, M, grad_weight, lr, grad_weight);
  mat_minus(N, M, weight, grad_weight, weight);
  mat_scalar(1, M, grad_bias, lr, grad_bias);
  mat_minus(1, M, bias, grad_bias, bias);
#endif
}

