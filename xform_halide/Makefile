
blur: blur.cpp
	g++ blur.cpp -o blur.out -I../third_party/halide/include/ -L ../third_party/halide/bin/ -lHalide -ldl -lpthread -lz
	DYLD_LIBRARY_PATH=../third_party/halide/bin/  ./blur.out

resize: resize.cpp
	g++ resize.cpp -o resize.out -I../third_party/halide/include/ -L ../third_party/halide/bin/ -lHalide -ldl -lpthread -lz
	DYLD_LIBRARY_PATH=../third_party/halide/bin/  ./resize.out

recon: recon.cpp
	g++ recon.cpp -o recon.out -I../third_party/halide/include/ -L ../third_party/halide/bin/ -lHalide -ldl -lpthread -lz
	DYLD_LIBRARY_PATH=../third_party/halide/bin/  ./recon.out

recon_separate: recon_separate.cpp
	g++ recon_separate.cpp -o recon_separate.out -I../third_party/halide/include/ -L ../third_party/halide/bin/ -lHalide -ldl -lpthread -lz
	DYLD_LIBRARY_PATH=../third_party/halide/bin/  ./recon_separate.out

dequant: dequant.cpp
	g++ dequant.cpp -o dequant.out -I../third_party/halide/include/ -L ../third_party/halide/bin/ -lHalide -ldl -lpthread -lz
	DYLD_LIBRARY_PATH=../third_party/halide/bin/  ./dequant.out

downsample: downsample.cpp
	g++ downsample.cpp -o downsample.out -I../third_party/halide/include/ -L ../third_party/halide/bin/ -lHalide -ldl -lpthread -lz
	DYLD_LIBRARY_PATH=../third_party/halide/bin/  ./downsample.out

highpass: highpass.cpp
	g++ highpass.cpp -o highpass.out -I../third_party/halide/include/ -L ../third_party/halide/bin/ -lHalide -ldl -lpthread -lz
	DYLD_LIBRARY_PATH=../third_party/halide/bin/  ./highpass.out

test: test.cpp blur resize
	g++ test.cpp halide_blur.o halide_resize.o  -o test -I ../third_party/halide/apps/support/ -lpng15
	./test ../images/tiger.png ./out.png

test_recon: test_recon.cpp recon resize
	g++ test_recon.cpp halide_recon.o halide_resize.o -o test_recon -I ../third_party/halide/apps/support/ -lpng15
	./test_recon ../images/yichang.png ../make/recipe_ac.png ../make/recipe_dc.png ./recon_out.png

test_recon_separate: test_recon_separate.cpp recon_separate
	g++ test_recon_separate.cpp halide_recon_separate.o  -o test_recon_separate -I ../third_party/halide/apps/support/ -lpng15 
	./test_recon_separate ./../images/yichang.png  ./../make/recipe_ac_lumin.png ./../make/recipe_ac_chrom.png ./../make/recipe_dc.png ./../make/quant.meta ./recon_separate_out.png

test_downsample: test_downsample.cpp downsample
	g++ test_downsample.cpp halide_downsample.o  -o test_downsample -I ../third_party/halide/apps/support/ -lpng15 
	./test_downsample ./../images/yichang.png  ./downsample_out.png

test_highpass: test_highpass.cpp highpass downsample
	g++ test_highpass.cpp halide_highpass.o halide_downsample.o -o test_highpass -I ../third_party/halide/apps/support/ -lpng15 
	./test_highpass ./../images/yichang.png ./highpass_out.png

test_dequant: test_dequant.cpp dequant
	g++ test_dequant.cpp halide_dequant.o  -o test_dequant -I ../third_party/halide/apps/support/ -lpng15
	./test_dequant ../make/recipe_ac.png


clean: 
	rm -rf blur resize test recon test_recon dequant test_dequant recon_separate test_recon_separate downsample test_downsample
