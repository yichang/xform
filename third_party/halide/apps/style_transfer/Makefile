include ../support/Makefile.inc

all: process style_transfer_wrapper.o

gradient_norm: ../../ gradient_norm.cpp
	$(MAKE)  -C ../../ $(LIB_HALIDE)
	$(CXX) gradient_norm.cpp -g -I../../include ../../$(LIB_HALIDE) -o gradient_norm -lpthread -ldl -lz $(LDFLAGS) $(LLVM_SHARED_LIBS)

style_transfer_wrapper.o: style_transfer_wrapper.cpp 
	$(CXX) -c -I../support -O3 style_transfer_wrapper.cpp  -o style_transfer_wrapper.o -I../../include 

gradient_norm.o: gradient_norm
	./gradient_norm

style_transfer: ../../ style_transfer.cpp
	$(MAKE)  -C ../../ $(LIB_HALIDE)
	$(CXX) style_transfer.cpp -g -I../../include ../../$(LIB_HALIDE) -o style_transfer -lpthread -ldl -lz $(LDFLAGS) $(LLVM_SHARED_LIBS)

hist.o : hist.cpp
	$(CXX) -c hist.cpp -o hist.o

style_transfer.o: style_transfer
	./style_transfer

process: process.cpp style_transfer.o gradient_norm.o hist.o
	$(CXX) -I../support -Wall -O3 process.cpp hist.o style_transfer.o gradient_norm.o -o process -I../../include -lpthread -ljpeg -ldl $(PNGFLAGS) $(CUDA_LDFLAGS) $(OPENCL_LDFLAGS) $(OPENGL_LDFLAGS) $(LLVM_SHARED_LIBS)



clean:
	rm -f process process_jpeg style_transfer.o style_transfer gradient_norm gradient_norm.o
