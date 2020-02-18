USE_OPENMP = 0
USE_GPU = 1


OUTDIR = objs
SRCDIR = src
SAMPLE = samples

SRC = $(wildcard $(SRCDIR)/*.cc)
OBJS = $(addsuffix .o, $(basename $(patsubst $(SRCDIR)/%,$(OUTDIR)/%,$(SRC))))

ARCH= -gencode arch=compute_30,code=sm_30 \
      -gencode arch=compute_35,code=sm_35 \
      -gencode arch=compute_50,code=[sm_50,compute_50] \
      -gencode arch=compute_52,code=[sm_52,compute_52]


NVCC = /usr/local/cuda/bin/nvcc $(ARCH)

CXXFLAGS = -O3 -std=c++11 -Wno-unused-result
INCLUDE = -I ./include/
LIBS = -lm
LIB = libxnnc.a
OPENCV = `pkg-config opencv --cflags --libs`

ifeq ($(USE_OPENMP), 1) 
  MACRO += -D USE_OPENMP
  LIBS += -fopenmp
endif

ifeq ($(USE_GPU), 1) 
  MACRO += -D GPU
  LIBS += -L /usr/local/cuda/lib64 -lcublas -lcudart -lcurand
  INCLUDE += -I /usr/local/cuda/include/
  CUDA_SRC = $(wildcard $(SRCDIR)/*.cu)
  OBJS += $(addsuffix .o, $(basename $(patsubst $(SRCDIR)/%,$(OUTDIR)/%,$(CUDA_SRC))))
endif

all: $(OUTDIR) $(LIB) samples

samples: mnist cifar

#test: $(LIB)
#	$(CXX) $(CXXFLAGS) $(INCLUDE) $(LIBS) unittest/conn_test.cc $(LIB) -o unittest/conn_test
#	$(CXX) $(CXXFLAGS) $(INCLUDE) $(LIBS) unittest/conv_test.cc $(LIB) -o unittest/conv_test
#	$(CXX) $(CXXFLAGS) $(INCLUDE) $(LIBS) unittest/bn_test.cc $(LIB) -o unittest/bn_test

vgg: $(SAMPLE)/vgg.cc $(LIB)
	$(CXX) $(CXXFLAGS) $(MACRO) $(INCLUDE) $^ $(LIBS) $(OPENCV) -o $(SAMPLE)/$@

mnist: $(SAMPLE)/mnist.cc $(LIB) 
	$(CXX) $(CXXFLAGS) $(MACRO) $(INCLUDE) $^ $(LIBS) -o $(SAMPLE)/$@

cifar: $(SAMPLE)/cifar.cc $(LIB)
	$(CXX) $(CXXFLAGS) $(MACRO) $(INCLUDE) $^ $(LIBS) -o $(SAMPLE)/$@

lenet: $(SAMPLE)/lenet.cc $(LIB)
	$(CXX) $(CXXFLAGS) $(MACRO) $(INCLUDE) $(LIBS) $^ $(OPENCV) -o $(SAMPLE)/$@

$(LIB): $(OBJS) 
	$(AR) rcs $@ $(OBJS)

$(OUTDIR)/%.o: $(SRCDIR)/%.cc 
	$(CXX) $(CXXFLAGS) $(MACRO) $(INCLUDE) $(LIBS) -c $< -o $@ 

$(OUTDIR)/%.o: $(SRCDIR)/%.cu 
	$(NVCC) $(INCLUDE) -c $< -o $@ 

$(OUTDIR):
	mkdir -p $(OUTDIR)

clean:
	rm -rf $(SAMPLE)/mnist $(SAMPLE)/cifar $(SAMPLE)/lenet $(OUTDIR) libxnnc.a


test: $(OUTDIR) $(LIB) 
	$(CXX) $(CXXFLAGS) $(MACRO) $(INCLUDE) gtest/gtest.cc $(LIB) $(LIBS) -lgmock -lgtest -lpthread -o gtest/gtest
	./gtest/gtest	


test_gpu: $(OUTDIR) $(LIB) 
	$(CXX) $(CXXFLAGS) $(MACRO) $(INCLUDE) gtest/gtest_gpu.cc $(LIB) $(LIBS) -lgmock -lgtest -lpthread -o gtest/gtest_gpu
	./gtest/gtest_gpu	



.PHONY: clean $(OUTDIR)

