#include "format/Format.h"
#include "types.h"

#include <gtest/gtest.h>

namespace {
	#define STR(x) x, sizeof(x)
	
	TEST(FormatSimple, Test1) {
		char foo[] = "xxy{} {}zzv";
		Format x;
		
		char *p = x.format(POD::Buffer(foo, sizeof(foo)));
		ASSERT_STREQ(p, "xxy zzv");
	}
	
	TEST(FormatSimple, Test2) {
		char foo[] = "xxy{} {}zz{{v";
		Format x;
		
		char *p = x.format(POD::Buffer(foo, sizeof(foo)));
		ASSERT_STREQ(p, "xxy zz{v");
	}
}

int runFormatTests()
{
	return RUN_ALL_TESTS();
}