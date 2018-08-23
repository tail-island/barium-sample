#include <iostream>

#include "alpha_beta.hpp"
#include "bridge.hpp"

int main(int argc, char** argv) {
  std::cout << "Hello, world!"    << std::endl;
  std::cerr << "Something wrong?" << std::endl;

  (barys::bridge<barys::alpha_beta>(barys::alpha_beta()))();  // 理由は分からないのですけど、Visual C++だと余分な括弧が必要でした……。

  return 0;
}
