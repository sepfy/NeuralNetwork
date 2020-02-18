#include <cublas_v2.h>
#include <cuda_runtime.h>
#include <curand.h>
cublasHandle_t gpu_handle(void);
float* malloc_gpu(size_t n);
void check_error(cudaError_t status);
void random_normal_gpu(int n, float *gpu_x);
void memset_gpu(size_t n, float *gpu_x);
