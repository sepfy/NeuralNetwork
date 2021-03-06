#include <iostream>
#include <vector>
#include "gemm.h"
#include "blas.h"
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include <string.h>
#include <fstream>
#include "binary.h"
#define XNOR_NET
using namespace std;

class Layer {
  public:
    int batch;
    bool train_flag = true;
    float *input;
    float *output;
    float *m_delta;
    virtual void forward() = 0;
    virtual void backward(float* delta) = 0;
    virtual void update(float lr) = 0;
    virtual void init() = 0;
    virtual void save(fstream *file) = 0;
};

class Connected : public Layer {

  public:
    float *weight;
    float *bias;
    float *grad_weight;
    float *grad_bias;
    // W is NxM matrix
    int N;   
    int M;
  
    // Adam optimizer
    float beta1 = 0.9;
    float beta2 = 0.999;
    float *m_weight;
    float *v_weight;
    float *m_bias;
    float *v_bias;
    float iter = 0.0;
    float epsilon = 1.0e-7;
 
    Connected(int _n, int _m);
    ~Connected();
    void init(); 
    void forward();
    void backward(float *delta);
    void update(float lr);
    void save(fstream *file);
    static Connected* load(char *buf);

};

class Sigmoid: public Layer {

  public:
    int N;
    Sigmoid(int _N);
    ~Sigmoid();
    void init();
    void forward();
    void backward(float *delta);
    void update(float lr);
    void save(fstream *file);
};

class SoftmaxWithCrossEntropy : public Layer {

  public:
    int N;

    SoftmaxWithCrossEntropy(int _n);
    ~SoftmaxWithCrossEntropy();
    void init();
    void forward();
    void backward(float *delta);
    void update(float lr);
    void save(fstream *file);
    static SoftmaxWithCrossEntropy* load(char *buf);

};

class Convolution : public Layer {

  public:
    bool xnor = true;
    float *col;
    int FW, FH, FC;
    int stride, pad;

    int W, H, C;
    int out_channel;
    int out_w, out_h;
    int col_size;
    int im_size;
    int weight_size;
    int bias_size;
    int input_size;

    bool runtime = false;

    float *weight, *bias, *out_col, *im;
    float *grad_weight, *grad_bias;
    float *mean;
    // Adam optimizer
    float beta1 = 0.9;
    float beta2 = 0.999;
    float *m_weight;
    float *v_weight;
    float *m_bias;
    float *v_bias;
    float iter = 0.0;
    float epsilon = 1.0e-7;

    Convolution(int _W, int _H, int _C,
	int _FW, int _FH, int _FC, int _stride, bool _pad);
    ~Convolution();
    void init();
    void forward();
    void backward(float *delta);
    void update(float lr);
    void save(fstream *file);
    static Convolution* load(char *buf);

#ifdef XNOR_NET
    float *binary_weight;
    float *binary_input;
    float *avg_filter;
    float *avg_col;
    float *k_filter;
    float *k_output;
    Bitset *bitset_outcol, *bitset_weight;
    void swap_weight();
    float binarize_weight();
    void binarize_input();
#endif
};

class Pooling : public Layer {

  public:

    float *col;
    int FW, FH, FC;
    int stride, pad;
    int W, H, C;
    int out_channel;
    int out_w, out_h;
    float *weight, *bias, *out_col, *im;
    float *grad_weight;
    float *delta_col;
    vector<int> max_seq;
    Pooling(int _W, int _H, int _C,
	int _FW, int _FH, int _FC, int _stride, bool _pad);
    ~Pooling();
    void init();
    void forward();
    void backward(float *delta);
    void update(float lr);
    void save(fstream *file);
    static Pooling* load(char *buf);
};

class Relu : public Layer {
  public:
    int N;
    float *cut;
    vector<int> mask;
    Relu(int _N);
    ~Relu();
    void init();
    void forward();
    void backward(float *delta);
    void update(float lr);
    void save(fstream *file);
    static Relu* load(char *buf);
};

class Batchnorm : public Layer {
  public:
    int N;
    float iter = 0.0;
    float *mean, *var, *running_mean, *running_var;
    float *normal;
    float epsilon = 1.0e-7;
    float *gamma, *beta, *dgamma, *dbeta;
    float *m_gamma, *m_beta, *v_gamma, *v_beta;
    float *dxn;
    float *dxc;
    float *dvar;
    float *dstd;
    float *dmu;
    float momentum = 0.9;
    float beta1 = 0.9;
    float beta2 = 0.999;
    Batchnorm(int _N);
    ~Batchnorm();
    void init();
    void forward();
    void backward(float *delta);
    void update(float lr);
    void save(fstream *file);
    static Batchnorm* load(char *buf);
};

class Dropout : public Layer {
  public:
    int N;
    float *mask;
    float ratio;
    Dropout(int _N, float _ratio);
    ~Dropout();
    void init();
    void forward();
    void backward(float *delta);
    void update(float lr);
    void save(fstream *file);
    static Dropout* load(char *buf);
};


class Shortcut : public Layer {
  public:
    int w, h, c;
    Shortcut(int _w, int _h, int _c, Convolution *_conv, Relu *_activation);
    ~Shortcut();
    float *identity;
    Convolution *conv;
    Relu *activation;
    void init();
    void forward();
    void backward(float *delta);
    void update(float lr);
    void save(fstream *file);
    static Shortcut* load(char *buf);
};


