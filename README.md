xform
=====

## Dependencies
The xform library relies on the following packages:

- libpng
- libjpeg
- png++ (a wrapper for libpng)
- Eigen
- Halide
- gTest framework

The xform package is written by Halide code and C++03 under 
```xform_halide/``` and ```src/``` 
The code in ```src/``` relies on the Eigen library and  Halide.
You can configure the dependencies to be under ```third_party```.

## Compile xform

You will compile the Halide code under ```xform_halide``` and C++03 code under ```src/``` separately.

### Compile the Halide library under ```xform_halide/```: 

```cd xform_halide/```
```make```

There are some ```test_*.cpp``` if you're interested.

To test the code under src/, go to make/ and do  
make TransformModel_unittest; ./TransformModel_unittest

Go to test/TreansformModel_unittest to see how to use this package.

Test results are under make/

The src/ are used by the server app and the client app in the below.

* Server apps

To launch the server app (processing + fit the recipe), go to apps/recipe/
type make

Try test.sh to make sure it works.

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
  
