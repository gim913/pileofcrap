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
	
	struct FormatOne : public ::testing::Test {
		FormatOne() {}
		Format x;
	};
	TEST_F(FormatOne, Test1) {
		char *p = x.parse(POD::ConstBuffer("xxy {} {}zzv"), 123);
		ASSERT_STREQ(p, "xxy 123 {noarg}zzv");
		
		char *q = x.parse(POD::ConstBuffer("xxy {:d}"), 123);
		ASSERT_STREQ(q, "xxy 123");
	}
	
	TEST_F(FormatOne, Test2) {
		char *p = x.parse(POD::ConstBuffer("xxy {:z}"), 123);
		ASSERT_STREQ(p, "xxy {badspec}");
		
		char *q = x.parse(POD::ConstBuffer("xxy {:d }"), 123);
		ASSERT_STREQ(p, "xxy {badspec}");
	}
}

int runFormatTests()
{
	return RUN_ALL_TESTS();
}