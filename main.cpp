#include "search/Search.h"
#include "types.h"

#include <gtest/gtest.h>

namespace {
	TEST(BufferTest, TestLen) {
		ASSERT_EQ(POD::ConstBuffer("foobar", 7).len, 7);
	}
	
	TEST(SearchPattern, TestSelf) {
		POD::ConstBuffer p("Nupogodi", sizeof("Nupogodi")-1);
		POD::ConstBuffer r = Search::pattern(p, p);
		ASSERT_EQ(p.ptr, r.ptr);
	}

	TEST(SearchPattern, TestInit) {
		POD::ConstBuffer a("Nupogodi", sizeof("Nupogodi")-1);
		POD::ConstBuffer b("foobar", sizeof("foobar")-1);
		Search::Pattern<const char> search(a);
		search.init(b);
		ASSERT_EQ(b.ptr, search.getPattern().ptr);
	}
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}