#include "format/Format.h"

#include <gtest/gtest.h>
#include <iostream>

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
	
  char foo[] = "xxy{} {}zz{{v";
  Format x;
  
  // using sizeof() will include trailing 0
  std::cout << x.format(POD::Buffer(foo, sizeof(foo))) << std::endl;
  
  return 1;
}