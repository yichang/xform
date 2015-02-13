xform
=====

Dependencies

- libpng
- libjpeg
- png++
- Eigen
- Halide
- gTest framework

This packages is written by halide code under xform_halide/ 
and c++ under src/
src/ relies on Eigen library and  the library written in Halide.

* Library

To compile the Halide libs in xform_halide/: 

cd xform_halide/
make
There are some test_*.cpp if you're interested.

To test the code under src/, go to make/ and do  
make TransformModel_unittest; ./TransformModel_unittest
Go to test/TreansformModel_unittest to see what's going on.

Test results are under make/

The src/ are used by the server app and the client app in the below.

* Server apps

To launch the server app (processing + fit the recipe), go to apps/recipe/
type make
try test.sh to make sure it works.

The compile library can be used by any socket program, eg., the one under socket/
is a good starting point (not yet finished)


* Client apps

Plasma/
This is the app for on-device local laplacian.
To compile: 
  step 1. run sh build_halide.sh to compile halide objects
  step 2. do ndk-build to  
  NOTE: the accompanying local laplacian takes floating format.
  You need to either change the interface of this app to floating input, 
  or use the original local Laplacian in Halide that uses uint16 format. 
  step3. use the dev to push the java code on the device  

UploadServer/
This is the app for cloud computing
To compile:
  step 1. run sh build_halide.sh to compile halide objects
  step 2. do ndk-build
  step 3. change the url in POST/GET method to your server and port.
  
