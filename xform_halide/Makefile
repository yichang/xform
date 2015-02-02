all: resize recon recon_separate compute_features compute_cfeatures dequant downsample highpass 

resize: resize.cpp
	g++ resize.cpp -o resize.out -I../third_party/halide/include/ -L ../third_party/halide/bin/ -lHalide -ldl -lpthread -lz
	DYLD_LIBRARY_PATH=../third_party/halide/bin/  ./resize.out

recon: recon.cpp
	g++ recon.cpp -o recon.out -I../third_party/halide/include/ -L ../third_party/halide/bin/ -lHalide -ldl -lpthread -lz
	DYLD_LIBRARY_PATH=../third_party/halide/bin/  ./recon.out

dequant: dequant.cpp
	g++ dequant.cpp -o dequant.out -I../third_party/halide/include/ -L ../third_party/halide/bin/ -lHalide -ldl -lpthread -lz
	DYLD_LIBRARY_PATH=../third_party/halide/bin/  ./dequant.out

recon_separate: recon_separate.cpp
	g++ recon_separate.cpp -o recon_separate.out -I../third_party/halide/include/ -L ../third_party/halide/bin/ -lHalide -ldl -lpthread -lz
	DYLD_LIBRARY_PATH=../third_party/halide/bin/  ./recon_separate.out 5 4 16

compute_features: compute_features.cpp
	g++ compute_features.cpp -o compute_features.out -I../third_party/halide/include/ -L ../third_party/halide/bin/ -lHalide -ldl -lpthread -lz
	DYLD_LIBRARY_PATH=../third_party/halide/bin/  ./compute_features.out

compute_cfeatures: compute_cfeatures.cpp
	g++ compute_cfeatures.cpp -o compute_cfeatures.out -I../third_party/halide/include/ -L ../third_party/halide/bin/ -lHalide -ldl -lpthread -lz
	DYLD_LIBRARY_PATH=../third_party/halide/bin/  ./compute_cfeatures.out

downsample: downsample.cpp
	g++ downsample.cpp -o downsample.out -I../third_party/halide/include/ -L ../third_party/halide/bin/ -lHalide -ldl -lpthread -lz
	DYLD_LIBRARY_PATH=../third_party/halide/bin/  ./downsample.out

highpass: highpass.cpp
	g++ highpass.cpp -o highpass.out -I../third_party/halide/include/ -L ../third_party/halide/bin/ -lHalide -ldl -lpthread -lz
	DYLD_LIBRARY_PATH=../third_party/halide/bin/  ./highpass.out

test_resize: test_resize.cpp resize
	g++ test_resize.cpp halide_resize.o -o test_resize -I ../third_party/halide/apps/support/ -lpng15
	./test_resize ../images/tiger.png ./out.png

test_recon: test_recon.cpp recon resize
	g++ test_recon.cpp halide_recon.o halide_resize.o -o test_recon -I ../third_party/halide/apps/support/ -lpng15
	./test_recon ../images/tiger.png ../make/recipe_ac.png ../make/recipe_dc.png ./recon_out.png

test_dequant: test_dequant.cpp dequant
	g++ test_dequant.cpp halide_dequant.o  -o test_dequant -I ../third_party/halide/apps/support/ -lpng15
	./test_dequant ../make/recipe_ac.png

test_recon_separate: test_recon_separate.cpp recon_separate
	g++ test_recon_separate.cpp halide_recon_separate.o  -o test_recon_separate -I ../third_party/halide/apps/support/ -lpng15  -Wall -O3 
	./test_recon_separate ./../images/tiger.png  ./../make/recipe_ac_lumin.png ./../make/recipe_ac_chrom.png ./../make/recipe_dc.png ./../make/quant.meta ./recon_separate_out.png

test_compute_features: test_compute_features.cpp compute_features
	g++ test_compute_features.cpp halide_compute_features.o  -o test_compute_features -I ../third_party/halide/apps/support/ -lpng15 
	./test_compute_features ./../images/tiger.png
	
test_compute_cfeatures: test_compute_cfeatures.cpp compute_cfeatures
	g++ test_compute_cfeatures.cpp halide_compute_cfeatures.o  -o test_compute_cfeatures -I ../third_party/halide/apps/support/ -lpng15 
	./test_compute_cfeatures ./../images/tiger.png

test_downsample: test_downsample.cpp downsample
	g++ test_downsample.cpp halide_downsample.o  -o test_downsample -I ../third_party/halide/apps/support/ -lpng15 
	./test_downsample ./../images/tiger.png  ./downsample_out.png

test_highpass: test_highpass.cpp highpass downsample
	g++ test_highpass.cpp halide_highpass.o halide_downsample.o -o test_highpass -I ../third_party/halide/apps/support/ -lpng15 
	./test_highpass ./../images/tiger.png ./highpass_out.png


clean: 
	rm -rf resize.out test_resize recon.out test_recon dequant.out test_dequant recon_separate.out test_recon_separate downsample.out test_downsample test_compute_features compute_features.out test_compute_cfeatures compute_cfeatures.out highpass.out test_highpass *.png
