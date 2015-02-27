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

You will need to launch a socket program on a server to use this App, eg., the one under  ```socket/```
is a good starting point, but not finished yet.


## Client Apps

### ```Plasma```
Plasma is an app to perform on-device local Laplacian filter.
To compile the app, do the following steps: 


```
cd Plasma/jni
sh build_halide.sh       # compile halide objects
ndk-build                # compile the halide object on Android
```
NOTE: the accompanying local Laplacian filter takes floating format.
You need to either adopt the interface of this Android App to floating input, 
or use the original local Laplacian in Halide, which takes uint16 format. 

 Finally, push the compiled binary to the device using Android SDK.

### UploadToServer

UploadToServer is an App to perform cloud-based image processing.
To compile:

```
cd UploadToServer/
sh build_halide.sh         # compile halide objects
ndk-build             #compile the halide object and native code to Android
```
Note: Before pushing the binary to the Android, you will need to change the url in ```POST``` and ```GET``` method to your server and port.
  
