
blur: blur.cpp
	g++ blur.cpp -o blur -I../third_party/halide/include/ -L ../third_party/halide/bin/ -lHalide -ldl -lpthread -lz
	DYLD_LIBRARY_PATH=../third_party/halide/bin/  ./blur

resize: resize.cpp
	g++ resize.cpp -o resize -I../third_party/halide/include/ -L ../third_party/halide/bin/ -lHalide -ldl -lpthread -lz
	DYLD_LIBRARY_PATH=../third_party/halide/bin/  ./resize

recon: recon.cpp
	g++ recon.cpp -o recon -I../third_party/halide/include/ -L ../third_party/halide/bin/ -lHalide -ldl -lpthread -lz
	DYLD_LIBRARY_PATH=../third_party/halide/bin/  ./recon

test: test.cpp blur resize
	g++ test.cpp halide_blur.o halide_resize.o  -o test -I ../third_party/halide/apps/support/ -lpng15
	./test ../images/tiger.png ./out.png

test_recon: test_recon.cpp recon resize
	g++ test_recon.cpp halide_recon.o halide_resize.o -o test_recon -I ../third_party/halide/apps/support/ -lpng15
	./test_recon ../images/yichang.png ../make/recipe_ac.png ../make/recipe_dc.png ./recon_out.png


clean: 
	rm -rf blur resize test recon test_recon
