#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gemm.h"
#include "blas.h"
#include "utils.h"

using ::testing::ElementsAreArray;

TEST(BLAS, LINF_NORM) {
  int N = 3;
  int M = 2;
  float A[] = {-1, 3, 9, 1, -3, 2};
  float norm;
  norm = Linf_norm(N, M, A);
  EXPECT_EQ(9, norm);
}

TEST(BLAS, L1_NORM) {
  int N = 3;
  int M = 2;
  float A[] = {-1, 3, 9, 1, -3, 2};
  float norm;
  norm = L1_norm(N, M, A);
  EXPECT_EQ(19, norm);
}

TEST(BLAS, L2_NORM) {
  int N = 3;
  int M = 2;
  float A[] = {-1, 3, 9, 1, -3, 2};
  float norm;
  norm = L2_norm(N, M, A);
  EXPECT_EQ(105, norm);
}

 
TEST(BLAS, TRANSPOSE) { 

  int M = 3;
  int N = 2;
  float A[] = {3, 4, 1, 2, 1, 3}; //3x2
  float B[] = {3, 1, 1, 4, 2, 3}; //2x3
  float C[6] = {0}; //3x3
  transpose(M, N, A, C);

  ASSERT_THAT(C, ElementsAreArray(B));
}


 
TEST(BLAS, GEMM) { 

  int M = 3;
  int N = 2;
  int P = 3;
  float A[] = { 3, 4, 1, 2, 1, 3, 2, 2, 1}; //3x3
  float B[] = { 1, 2, 3, 2, 1, 2};  //3x2
  float D[] = {32, 32, 16, 24, 18, 20}; //3x2
  float alpha = 2;
  float C[6] = {0};
  gemm(M, N, P, alpha, A, B, C);

  ASSERT_THAT(C, ElementsAreArray(D));
}

TEST(UTILITY, IM2COL) {

  /*
   *  (N, H, W, C) = (1, 3, 3, 2)  
   *  filter = (2, 2, 2), stride = 1 padding = 1
   *
   *  Input:                   Output:
   *    1 2 1   3 4 2            1 3 2 4 3 1 4 2
   *    3 4 2   1 2 2            2 4 1 2 4 2 2 2
   *    1 6 2   9 8 7            3 1 4 2 1 9 6 8
   *                             4 2 2 2 6 8 2 7
   *
   */

  float im[] = {1, 3, 2, 4, 1, 2, 3, 1, 4, 2, 2, 2, 1, 9, 6, 8, 2, 7};
  float col[] = {1, 3, 2, 4, 3, 1, 4, 2,
                 2, 4, 1, 2, 4, 2, 2, 2,
                 3, 1, 4, 2, 1, 9, 6, 8,
                 4, 2, 2, 2, 6, 8, 2, 7};

  int W = 3, H = 3, C = 2;
  int FW = 2, FH = 2, FC = 2;
  int stride = 1, pad = 0;

  int out_w = (W + 2*pad - FW)/stride + 1;  //3-2+1 = 2
  int out_h = (H + 2*pad - FH)/stride + 1;  //3-2+1 = 2
  int channel_out = FW*FH*C;

  //float *col_out = new float[out_w*out_h*channel_out];
  float col_out[32] = {0};;
  im2col(W, H, C, FW, FH, FC,
             stride, pad, im, col_out);

  ASSERT_THAT(col_out, ElementsAreArray(col));
}


TEST(UTILITY, COL2IM) {

  /*
   *  (N, H, W, C) = (1, 3, 3, 2)  
   *  filter = (2, 2, 2), stride = 1 padding = 1
   *
   *  Output:                  Input:
   *    1 2 1   3 4 2            1 3 2 4 3 1 4 2
   *    3 4 2   1 2 2            2 4 1 2 4 2 2 2
   *    1 6 2   9 8 7            3 1 4 2 1 9 6 8
   *                             4 2 2 2 6 8 2 7
   *
   */

  float im[] = {1, 3, 2, 4, 1, 2, 3, 1, 4, 2, 2, 2, 1, 9, 6, 8, 2, 7};
  float col[] = {1, 3, 2, 4, 3, 1, 4, 2,
                 2, 4, 1, 2, 4, 2, 2, 2,
                 3, 1, 4, 2, 1, 9, 6, 8,
                 4, 2, 2, 2, 6, 8, 2, 7};

  int W = 3, H = 3, C = 2;
  int FW = 2, FH = 2, FC = 2;
  int stride = 1, pad = 0;

  int out_w = (W + 2*pad - FW)/stride + 1;  //3-2+1 = 2
  int out_h = (H + 2*pad - FH)/stride + 1;  //3-2+1 = 2
  int channel_out = FW*FH*C;

  //float *col_out = new float[out_w*out_h*channel_out];
  float im_out[18] = {0};;
  col2im(W, H, C, FW, FH, FC,
             stride, pad, im_out, col);

  ASSERT_THAT(im_out, ElementsAreArray(im));
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
