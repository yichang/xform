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
You can place the dependencies under ```third_party```.

## Compile xform

You will compile the Halide code under ```xform_halide``` and C++03 code under ```src/``` separately.

### Compile the Halide library under ```xform_halide/```: 

```
cd xform_halide/ 
make
```

You can check out  ```test_*.cpp``` to see the input and output of the Halide functions.

### Compile the code under ```src/```

We will compile test cases written with Google test framework.

```
cd make/   
make TransformModel_unittest
./TransformModel_unittest
```

Check ```test/TreansformModel_unittest.cpp``` to see how to use this package.

The generated results of the tests are under ```make/```.

The functions under ```src/``` are used by the server App and the client App described in the below.

## Server Apps

To launch the server app, which includes an image processing algorithm and a recipe fitting algorithm, do the following: 

```
cd apps/recipe/
make
```

Try ```test.sh``` to make sure the app works.

The compiled library will be used by a socket program, eg., the one under  ```socket/```
is a good starting point but not finished yet.


## Client Apps

### ```Plasma/```
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
  
