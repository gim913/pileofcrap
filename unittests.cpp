#include "search/Search.h"
#include "types.h"

#include <gtest/gtest.h>

namespace {
	TEST(BufferTest, TestLen) {
		ASSERT_EQ(POD::ConstBuffer("foobar", 7).len, 7);
	}
	
	// ===== ===== ===== Search::Pattern TESTS ===== ===== ===== 
	
	TEST(SearchPattern, TestSelf) {
		POD::ConstBuffer p("Nupogodi", sizeof("Nupogodi")-1);
		POD::ConstBuffer r = Search::pattern(p, p);
		ASSERT_EQ(p.ptr, r.ptr);
	}

	TEST(SearchPattern, TestReset) {
		POD::ConstBuffer a("Nupogodi", sizeof("Nupogodi")-1);
		POD::ConstBuffer b("foobar", sizeof("foobar")-1);
		Search::Pattern<const char> search(a);
		search.reset(b);
		ASSERT_EQ(b.ptr, search.getPattern().ptr);
	}

	TEST(SearchPattern, TestAllocated) {
		char realBuf[0x1000];

		e_uint lfsr = 0x123456;
		for (size_t i = 0; i < sizeof(realBuf); ++i) {
			lfsr = (lfsr >> 1) ^ (unsigned int)(0 - (lfsr & 1u) & 0xd0000001u);
			lfsr = (lfsr >> 1) ^ (unsigned int)(0 - (lfsr & 1u) & 0xd0000001u);
			lfsr = (lfsr >> 1) ^ (unsigned int)(0 - (lfsr & 1u) & 0xd0000001u);
			realBuf[i] = lfsr & 0xff;
		}
		
		POD::ConstBuffer txt(realBuf, sizeof(realBuf));
		POD::ConstBuffer p(realBuf + 0xabc, 0x100);
		POD::ConstBuffer r = Search::pattern(p, txt);
		ASSERT_EQ((txt.ptr + 0xabc), r.ptr);
		
		// 'o' occures few times, but shouldn't cause problem
		POD::ConstBuffer r2 = Search::patternDot(p, 'o', txt);
		ASSERT_EQ((txt.ptr + 0xabc), r2.ptr);
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
	
	// ===== ===== ===== Search::PatternDot TESTS ===== ===== ===== 

	TEST(SearchDotPattern, TestSelf) {
		POD::ConstBuffer p("Nupogodi", sizeof("Nupogodi")-1);
		POD::ConstBuffer r = Search::patternDot(p, 0, p);
		ASSERT_EQ(p.ptr, r.ptr);
	}

	TEST(SearchDotPattern, TestReset) {
		POD::ConstBuffer a("Nupogodi", sizeof("Nupogodi")-1);
		POD::ConstBuffer b("foobar", sizeof("foobar")-1);
		Search::PatternDot<const char> search(a, 0);
		search.reset(b, 0);
		ASSERT_EQ(b.ptr, search.getPattern().ptr);
	}
	
	struct TestMississippiDot : public ::testing::Test {
		TestMississippiDot() : txt("Mississippi", sizeof("Mississippi")-1) {}
		POD::ConstBuffer txt;
	};
	TEST_F(TestMississippiDot, TestBegin) {
		POD::ConstBuffer p("M.s", sizeof("M.s")-1);
		POD::ConstBuffer r = Search::patternDot(p, '.', txt);
		ASSERT_EQ(txt.ptr, r.ptr);
	}
	
	TEST_F(TestMississippiDot, TestMiddle1) {
		POD::ConstBuffer p("s.s", sizeof("s.s")-1);
		POD::ConstBuffer r = Search::patternDot(p, '.', txt);
		ASSERT_EQ( (txt.ptr + 3), r.ptr);
	}
	
	TEST_F(TestMississippiDot, TestMiddle2) {
		POD::ConstBuffer p("s.i", sizeof("s.i")-1);
		POD::ConstBuffer r = Search::patternDot(p, '.', txt);
		ASSERT_EQ( (txt.ptr + 2), r.ptr);
	}
	
	TEST_F(TestMississippiDot, TestMiddle3) {
		POD::ConstBuffer p("s.p", sizeof("s.p")-1);
		POD::ConstBuffer r = Search::patternDot(p, '.', txt);
		ASSERT_EQ( (txt.ptr + 6), r.ptr);
	}
	TEST_F(TestMississippiDot, TestEnd) {
		POD::ConstBuffer p("p.i", sizeof("p.i")-1);
		POD::ConstBuffer r = Search::patternDot(p, '.', txt);
		ASSERT_EQ( (txt.ptr + 8), r.ptr);
	}
	
	TEST_F(TestMississippiDot, TestMiddle4) {
		POD::ConstBuffer p("i..i", sizeof("i..i")-1);
		POD::ConstBuffer r = Search::patternDot(p, '.', txt);
		ASSERT_EQ( (txt.ptr + 1), r.ptr);
	}
	
	TEST_F(TestMississippiDot, TestMiddle5) {
		POD::ConstBuffer p("s...i", sizeof("s...i")-1);
		POD::ConstBuffer r = Search::patternDot(p, '.', txt);
		ASSERT_EQ( (txt.ptr + 3), r.ptr);
	}
	
	TEST_F(TestMississippiDot, TestMiddle6) {
		POD::ConstBuffer p("s.i.s", sizeof("s.i.s")-1);
		POD::ConstBuffer r = Search::patternDot(p, '.', txt);
		ASSERT_EQ( (txt.ptr + 2), r.ptr);
	}
	
	TEST_F(TestMississippiDot, TestMiddle7) {
		POD::ConstBuffer p("si.", sizeof("si.")-1);
		POD::ConstBuffer r = Search::patternDot(p, '.', txt);
		ASSERT_EQ( (txt.ptr + 3), r.ptr);
	}

	TEST_F(TestMississippiDot, TestMiddle8) {
		POD::ConstBuffer p("i.pi", sizeof("i.pi")-1);
		POD::ConstBuffer r = Search::patternDot(p, '.', txt);
		ASSERT_EQ( (txt.ptr + 7), r.ptr);
	}
	
	struct TestMississippiMulti : public ::testing::Test {
		TestMississippiMulti() {}
		static const POD::ConstBuffer pats[3];
	};
	
	const POD::ConstBuffer TestMississippiMulti::pats[] = {
		{ "iss", 3 },
		{ "ipp", 3 },
		{ "ssip", 4 }
	};

#define _countof(x) (sizeof(x)/sizeof(*x))
	TEST_F(TestMississippiMulti, Test1) {
		POD::ConstBuffer txt("Mississippi", sizeof("Mississippi")-1);
		POD::ConstBuffer r = Search::multiPattern(pats, _countof(pats), txt);
		ASSERT_EQ(txt.ptr+1, r.ptr);
	}
	
	TEST_F(TestMississippiMulti, Test2) {
		POD::ConstBuffer txt("Mxssxssippi", sizeof("Mxssxssippi")-1);
		POD::ConstBuffer r = Search::multiPattern(pats, _countof(pats), txt);
		ASSERT_EQ(txt.ptr+5, r.ptr);
	}
	
	TEST_F(TestMississippiMulti, Test3) {
		POD::ConstBuffer txt("Mxssxssippi", sizeof("Mxssxssippi")-1);
		Search::MultiPattern<const char, 10> mp(pats, _countof(pats));
		POD::ConstBuffer r = mp.search(txt);
		ASSERT_EQ(txt.ptr+5, r.ptr);
	}
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}