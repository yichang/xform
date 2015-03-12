include ../support/Makefile.inc

LDFLAGS += -ljpeg

all: process

local_laplacian: ../../ local_laplacian.cpp
	$(MAKE) -C ../../ $(LIB_HALIDE)
	$(CXX) local_laplacian.cpp -g -I../../include ../../$(LIB_HALIDE) -o local_laplacian -lpthread -ldl -lz $(LDFLAGS) $(LLVM_SHARED_LIBS)

local_laplacian.o: local_laplacian
	./local_laplacian

process: process.cpp local_laplacian.o
	$(CXX) -I../support -Wall -O3 process.cpp local_laplacian.o -o process -I../../include -lpthread -ldl -ljpeg $(PNGFLAGS) $(CUDA_LDFLAGS) $(OPENCL_LDFLAGS) $(OPENGL_LDFLAGS) $(LLVM_SHARED_LIBS)

# process_jpeg: process_jpeg.cpp jo_jpeg.cpp local_laplacian.o 
# 	$(CXX) -I../support -Wall -O3 process_jpeg.cpp local_laplacian.o -o process_jpeg -lpthread -ldl $(PNGFLAGS) $(CUDA_LDFLAGS) $(OPENCL_LDFLAGS) $(OPENGL_LDFLAGS) $(LLVM_SHARED_LIBS)



out.png: process
	./process ../images/rgb.png 8 1 1 out.png

clean:
	rm -f process process_jpeg local_laplacian.o local_laplacian
