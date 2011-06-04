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
	
	struct TestMississippi : public ::testing::Test {
		TestMississippi() : txt("Mississippi", sizeof("Mississippi")-1) {}
		POD::ConstBuffer txt;
	};
	TEST_F(TestMississippi, TestBegin) {
		POD::ConstBuffer p("Mis", sizeof("Mis")-1);
		POD::ConstBuffer r = Search::pattern(p, txt);
		ASSERT_EQ(txt.ptr, r.ptr);
	}
	
	TEST_F(TestMississippi, TestMiddle1) {
		POD::ConstBuffer p("sis", sizeof("sis")-1);
		POD::ConstBuffer r = Search::pattern(p, txt);
		ASSERT_EQ( (txt.ptr + 3), r.ptr);
	}
	
	TEST_F(TestMississippi, TestMiddle2) {
		POD::ConstBuffer p("ssi", sizeof("ssi")-1);
		POD::ConstBuffer r = Search::pattern(p, txt);
		ASSERT_EQ( (txt.ptr + 2), r.ptr);
	}
	
	TEST_F(TestMississippi, TestMiddle3) {
		POD::ConstBuffer p("sip", sizeof("sip")-1);
		POD::ConstBuffer r = Search::pattern(p, txt);
		ASSERT_EQ( (txt.ptr + 6), r.ptr);
	}
	TEST_F(TestMississippi, TestEnd) {
		POD::ConstBuffer p("ppi", sizeof("ppi")-1);
		POD::ConstBuffer r = Search::pattern(p, txt);
		ASSERT_EQ( (txt.ptr + 8), r.ptr);
	}
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}