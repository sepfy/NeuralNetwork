#include <iostream>
#include <random>
#include <sys/time.h>
#include <math.h>

using namespace std;

unsigned long long getms() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec*1.0e+6 + tv.tv_usec;
}

int bitcount(uint32_t n){
  return  __builtin_popcountl(n);
}

class Bitset {

  public:
    uint32_t *bits;
    int size;
    int bitnum;
    uint32_t complement;
    int row, col;
    int offset;
    float l1;
    Bitset(int _size) {
      bitnum = _size;
      size = ceil((float)_size/(8*sizeof(uint32_t)));
      bits = new uint32_t[size];
      offset = _size%(8*sizeof(uint32_t));
      complement = (UINT32_MAX << (32 - offset)) >> (32-offset);
    }

     
    // if tr is true, the compress with column
    // PxN -> bitset: N x \hat{P}
    // else
    // MxP -> bitset: M x \hat{P}
    Bitset(int N, int M, float *input, bool tr) {

      if(tr) {
        bitnum = N;
        offset = bitnum%(8*sizeof(uint32_t));
        size = ceil((float)bitnum/(8*sizeof(uint32_t)));
        bits = new uint32_t[M*size];
        for(int j = 0; j < M; j++) {
          for(int i = 0; i < N; i++) {
            int idx = j*size*(8*sizeof(uint32_t)) + N-1-i;
            input[i*M+j] >= 0 ? set(idx, 1) : set(idx, 0);
          }
        }
      }
      else {
        bitnum = M;
        offset = bitnum%(8*sizeof(uint32_t));
        size = ceil((float)bitnum/(8*sizeof(uint32_t)));
        bits = new uint32_t[N*size];
        for(int i = 0; i < N; i++) {
          for(int j = 0; j < M; j++) {
            int idx = i*size*(8*sizeof(uint32_t)) + M-1-j;
            input[i*M+j] >= 0 ? set(idx, 1) : set(idx, 0);
          }
        }
      }
   }


    void set(int idx, uint32_t value) {
      int bidx = idx/(8*sizeof(uint32_t));
      int offset = idx%(8*sizeof(uint32_t));
      //cout << (value << offset)<<endl;
      bits[bidx] |= (value << offset);
    }

    void sign_to_bin(int size, float *input) {
      for(int i = 0; i < size; i++) {
        input[i] >= 0 ? set(size-1-i, 1) : set(size-1-i, 0);
      }
    }


};


int count(int N, uint32_t *A) {
  int count = 0;
  for(int i = 0; i < N; i++)
    count += bitcount(A[i]);
  return count;
}


int count2value(int bitnum, int N, uint32_t *A) {
  int c = count(N, A);
  return 2*c - bitnum;
}

void xnor_cpu(uint32_t complement, int N, uint32_t *A, uint32_t *B, uint32_t *C) {
  for(int i = 0; i < N; i++) 
    C[i] = ~(A[i]^B[i]);
  C[N-1] &= complement;
}

void xnor(Bitset *b1, Bitset *b2, Bitset *b3) {

  if(b1->bitnum != b2->bitnum || b1->bitnum != b3->bitnum) {
    printf("Bit num not equal");
  }
  int i = 0;

  xnor_cpu(b3->complement, b3->size, b1->bits, b2->bits, b3->bits);
}


int bin_dot(uint32_t offset, int N, uint32_t *A, uint32_t *B) {

  uint32_t *C = new uint32_t[N];
  int complement = (UINT32_MAX << (32 - offset)) >> (32-offset);
  xnor_cpu(complement, N, A, B, C);
  int val = count2value((N-1)*32+offset,N, C);
  delete C;
  return val;
}

//5x7 7x4 5x1 1x4
int bin_mat(uint32_t offset, int M, int N, int P, uint32_t *A, uint32_t *B, float *C) {

  for(int i = 0; i < M; i++) {
    for(int j = 0; j < N; j++) {
      C[i*N+j] = (float)bin_dot(offset, P, A+i*P, B+j*P);
    }
  }
}

void bin_gemm(int M, int N, int P,
  float alpha, float *A, float *B, float *C) {

  Bitset bA(M, P, A, false);
  Bitset bB(P, N, B, true);

   

  bin_mat(bA.offset, M, N, bA.size, bA.bits, bB.bits, C);
}

void test() {

#if 0 
  int i;
  int size = 6;
  default_random_engine generator = default_random_engine(time(NULL));
  normal_distribution<float> distribution(0, 0.5);

  float *mat1 = new float[size];
  float *mat2 = new float[size];
  for(i = 0; i < size; i++) {
    mat1[i] = distribution(generator);
    mat2[i] = distribution(generator);
  }




  unsigned long long start = getms();

  for(int i = 0; i < size; i++) {
    //mat[i] >= 0 ? mat[i] = 0 : mat[i] = 1;
    mat1[i] >= 0 ? cout << 1 : cout << 0;
  }

  cout << endl;
  for(int i = 0; i < size; i++) {
    //mat[i] >= 0 ? mat[i] = 0 : mat[i] = 1;
    mat2[i] >= 0 ? cout << 1 : cout << 0;
  }
  cout << endl;
  Bitset bitset1(size);
  Bitset bitset2(size);
  Bitset bitset3(size);
  bitset1.sign_to_bin(size, mat1);
  bitset2.sign_to_bin(size, mat2);


  xnor(&bitset1, &bitset2, &bitset3);

  cout << bitset1.bits[0] << endl;
  cout << bitset2.bits[0] << endl;
  cout << bitset3.bits[0] << endl;
  cout << "count" << endl;
  //cout << bitset1.count2value() << endl;
  //cout << bitset2.count2value() << endl;
  //cout << bitset3.count2value() << endl;
  cout << count2value(bitset1.bitnum, bitset1.size, bitset1.bits) << endl;
  cout << count2value(bitset2.bitnum, bitset2.size, bitset2.bits) << endl;
  cout << count2value(bitset3.bitnum, bitset3.size, bitset3.bits) << endl;
#endif

  //bin_matmul(A, B, C);

  //return 0;
}


float binarize(int size, float *input) {
  float l1 = 0.0;
  for(int i = 0; i < size; i++) {
    l1 += input[i];
    input[i] >= 0 ? input[i] = 1 : input[i] = 0;
  }

  return l1/(float)size;
}

int main() {

  int M = 5;
  int N = 4;
  int P = 7;
  default_random_engine generator = default_random_engine(time(NULL));
  normal_distribution<float> distribution(0, 0.5);

  float *mat1 = new float[M*P];
  float *mat2 = new float[N*P];
  for(int i = 0; i < M; i++) {
    for(int j = 0; j < P; j++) {
      mat1[i*P+j] = distribution(generator);
      mat1[i*P+j] >= 0 ? cout << "1" : cout << "0";
    }
    cout << endl;
  }
  cout << endl;
  for(int i = 0; i < P; i++) {
    for(int j = 0; j < N; j++) {
      mat2[i*N+j] = distribution(generator);
      //mat2[i*N+j] >= 0 ? cout << "1" : cout << "0";
      cout << mat2[i*N+j] << " ";
    }
    cout << endl;
  }
    cout << endl;

  binarize(P*N, mat2);
  for(int i = 0; i < P; i++) {
    for(int j = 0; j < N; j++) {
      cout << mat2[i*N+j];
    }
    cout << endl;
  }
    cout << endl;

#if 0
  long long start = getms();
  float *C = new float[M*P];
  bin_gemm(M, N, P, 1.0, mat1, mat2, C);
  cout << getms() -start << endl;
  for(int i = 0; i < M; i++) {
    for(int j = 0; j < N; j++) {
      printf("%.2f ", C[i*N+j]);
    }
    cout << endl;
  }
  cout << endl;
#endif
  return 0;
}
