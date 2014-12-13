// Compile: g++ test_eigen.cpp -I./../libs/eigen-eigen-1306d75b4a21/; ./a.out

#include <iostream>
#include <Eigen/Dense>
using namespace Eigen; 

int main(){

  MatrixXd m(2,2);
  m(0,0) = 3;
  m(1,0) = 2.5;
  m(0,1) = -1;
  m(1,1) = m(1,0) + m(0,1);

  std::cout << m << std::endl;
  return 0; 
}

