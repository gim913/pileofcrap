#include "types.h"

#include <gtest/gtest.h>

namespace {
	TEST(BufferTest, TestLen) {
		ASSERT_EQ(POD::ConstBuffer("foobar", 7).len, 7);
	}
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}