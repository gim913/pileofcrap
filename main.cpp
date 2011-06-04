#include "search/Search.h"
#include "types.h"

#include <gtest/gtest.h>

namespace {
	TEST(BufferTest, TestLen) {
		ASSERT_EQ(POD::ConstBuffer("foobar", 7).len, 7);
	}
	
	TEST(SearchPattern, TestSelf) {
		POD::ConstBuffer p("Nupogodi", sizeof("Nupogodi")-1);
		POD::ConstBuffer t = Search::pattern(p, p);
		ASSERT_EQ(p.ptr, t.ptr);
	}
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}