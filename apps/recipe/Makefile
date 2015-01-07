
EIGEN_LIB = eigen-eigen-1306d75b4a21
PNG_LIB = png++-0.2.5

CPPFLAGS += -I./../../src/\
	    -I./../../third_party/$(EIGEN_LIB)/\
	    -I./../../third_party/$(PNG_LIB)/\
	    -lpng15\
	    -ljpeg

CXXFLAGS += -g -Wall -Wextra -pthread -std=c++11 -Ofast -msse2

TARGETS=extract_recipe

all : $(TARGETS)
clean :
	rm -f $(TARGETS)  *.o *.png *.meta

util.o : ../../src/util.cpp \
        $(GTEST_HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ../../src/util.cpp

XImage.o : ../../src/XImage.cpp \
        $(GTEST_HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ../../src/XImage.cpp

Warp.o : ../../src/Warp.cpp \
        $(GTEST_HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ../../src/Warp.cpp

Recipe.o : ../../src/Recipe.cpp \
        $(GTEST_HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ../../src/Recipe.cpp

TransformModel.o : ../../src/TransformModel.cpp \
        $(GTEST_HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ../../src/TransformModel.cpp


extract_recipe: extract_recipe.o Recipe.o TransformModel.o Warp.o XImage.o util.o 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lpthread $^ -o $@

