#include "format/Format.h"
#include "types.h"

#include <gtest/gtest.h>

namespace {
	#define STR(x) x, sizeof(x)
	
	struct FormatSimple : public ::testing::Test {
		FormatSimple() {}
		Format x;
	};
	
	TEST_F(FormatSimple, Test1) {
		char *p = x.parse(POD::ConstBuffer("xxy{} {}zzv"));
		ASSERT_STREQ(p, "xxy{noarg} {noarg}zzv");
	}
	
	TEST_F(FormatSimple, Test2) {
		char *p = x.parse(POD::ConstBuffer("xxy{} {}zz{{v"));
		ASSERT_STREQ(p, "xxy{noarg} {noarg}zz{v");
	}
	
	TEST_F(FormatSimple, Test3) {
		char *p = x.parse(POD::ConstBuffer("xxy{123"));
		ASSERT_STREQ(p, "xxy{badformat}");
	}
	
	TEST_F(FormatSimple, Test4) {
		char *p = x.parse(POD::ConstBuffer("xxy{,-123"));
		ASSERT_STREQ(p, "xxy{badformat}");
	}
}

int runFormatTests()
{
	return RUN_ALL_TESTS();
}