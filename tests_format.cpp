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
		
		char *r = x.parse(POD::ConstBuffer("xxy {:d5}"), 123);
		ASSERT_STREQ(r, "xxy 00123");
	}
	
	TEST_F(FormatOne, Test2) {
		char *p = x.parse(POD::ConstBuffer("xxy {:z}"), 123);
		ASSERT_STREQ(p, "xxy {badspec}");
		
		char *q = x.parse(POD::ConstBuffer("xxy {:d }"), 123);
		ASSERT_STREQ(p, "xxy {badspec}");
	}
	
	TEST_F(FormatOne, Test3) {
		char *p = x.parse(POD::ConstBuffer("xxy {:x}"), 0x123abcde);
		ASSERT_STREQ(p, "xxy 123abcde");
		
		char *q = x.parse(POD::ConstBuffer("xxy {:X}"), 0x123abcde);
		ASSERT_STREQ(p, "xxy 123ABCDE");
		
		char *r = x.parse(POD::ConstBuffer("xxy {:o}"), 01234567);
		ASSERT_STREQ(p, "xxy 1234567");
		
		char *s = x.parse(POD::ConstBuffer("xxy {:b}"), 0xac);
		ASSERT_STREQ(p, "xxy 10101100");
	}
	
	
}

int runFormatTests()
{
	return RUN_ALL_TESTS();
}