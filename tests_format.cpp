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
		
		char *t = x.parse(POD::ConstBuffer("xxy {:d}"), static_cast<e_long>(0x8000000000000000ull));
		ASSERT_STREQ(t, "xxy -9223372036854775808");
		
		char *s = x.parse(POD::ConstBuffer("xxy {:d}"), -123);
		ASSERT_STREQ(s, "xxy -123");
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
	
	TEST_F(FormatOne, Test4) {
		char *p = x.parse(POD::ConstBuffer("foo{,6}"), 345);
		ASSERT_STREQ(p, "foo   345");
		
		char *q = x.parse(POD::ConstBuffer("foo{,6}"), -345);
		ASSERT_STREQ(q, "foo  -345");
		
		char *r = x.parse(POD::ConstBuffer("foo {,3}"), 12345);
		ASSERT_STREQ(r, "foo 12345");
		
		char *s = x.parse(POD::ConstBuffer("foo {,3}"), -12345);
		ASSERT_STREQ(s, "foo -12345");
	}
	
	TEST_F(FormatOne, Test4b) {
		char *p = x.parse(POD::ConstBuffer("foo{,-6}"), 345);
		ASSERT_STREQ(p, "foo345   ");
		
		char *q = x.parse(POD::ConstBuffer("foo{,-6}"), -345);
		ASSERT_STREQ(q, "foo-345  ");
		
		char *r = x.parse(POD::ConstBuffer("foo {,-3}"), 12345);
		ASSERT_STREQ(r, "foo 12345");
		
		char *s = x.parse(POD::ConstBuffer("foo {,-3}"), -12345);
		ASSERT_STREQ(s, "foo -12345");
	}
	
	TEST_F(FormatOne, Test5) {
		char* p = x.parse(POD::ConstBuffer("foo{,6:d5}"), 345);
		ASSERT_STREQ(p, "foo 00345");
		
		char* q = x.parse(POD::ConstBuffer("foo{,6:d5}"), -345);
		ASSERT_STREQ(q, "foo-00345");
		
		char *r = x.parse(POD::ConstBuffer("foo{,8:d6}"), 12345);
		ASSERT_STREQ(r, "foo  012345");
		
		char *s = x.parse(POD::ConstBuffer("foo{,8:d6}"), -12345);
		ASSERT_STREQ(s, "foo -012345");
	}
	
	TEST_F(FormatOne, Test5b) {
		char* p = x.parse(POD::ConstBuffer("foo{,-6:d4}"), 345);
		ASSERT_STREQ(p, "foo0345  ");
		
		char* q = x.parse(POD::ConstBuffer("foo{,-6:d4}"), -345);
		ASSERT_STREQ(q, "foo-0345 ");
		
		char *r = x.parse(POD::ConstBuffer("foo{,-8:d6}"), 12345);
		ASSERT_STREQ(r, "foo012345  ");
		
		char *s = x.parse(POD::ConstBuffer("foo{,-8:d6}"), -12345);
		ASSERT_STREQ(s, "foo-012345 ");
	}
	
	// these, are some extreme checks to make sure that
	// "at the end of buffer", the string will be properly formatted
	struct FormatExt : public ::testing::Test {
		FormatExt() {}
		FormatB<10> x;
	};
	
	TEST_F(FormatExt, Test1) {
		char *p = x.parse(POD::ConstBuffer("foobar{}"), 12345);
		ASSERT_STREQ(p, "foobar123");
		
		char *q = x.parse(POD::ConstBuffer("foobar{}"), -12345);
		ASSERT_STREQ(q, "foobar-12");
		
		char *r = x.parse(POD::ConstBuffer("foobar{:d7}"), 12345);
		ASSERT_STREQ(r, "foobar001");
		
		char *s = x.parse(POD::ConstBuffer("foobar{:d7}"), -12345);
		ASSERT_STREQ(s, "foobar-00");
	}
	
	TEST_F(FormatExt, Test2) {
		char *p = x.parse(POD::ConstBuffer("foobar{,8}"), 12345);
		ASSERT_STREQ(p, "foobar   ");
		
		char *q = x.parse(POD::ConstBuffer("foobar{,8}"), -12345);
		ASSERT_STREQ(q, "foobar  -");
		
		char *r = x.parse(POD::ConstBuffer("foobar{,9}"), 12345);
		ASSERT_STREQ(r, "foobar   ");
		
		char *s = x.parse(POD::ConstBuffer("foobar{,9}"), -12345);
		ASSERT_STREQ(s, "foobar   ");
	}

	TEST_F(FormatExt, Test3) {
		char *p = x.parse(POD::ConstBuffer("foo{,9:d7}"), 12345);
		ASSERT_STREQ(p, "foo  0012");
		
		char *q = x.parse(POD::ConstBuffer("foo{,9:d7}"), -12345);
		ASSERT_STREQ(q, "foo -0012");
		
		char *r = x.parse(POD::ConstBuffer("foo{,-10}"), 12345);
		ASSERT_STREQ(r, "foo12345 ");
		
		char *s = x.parse(POD::ConstBuffer("foo{,-10}"), -12345);
		ASSERT_STREQ(s, "foo-12345");
	}
	
	
	struct FormatTwo : public ::testing::Test {
		FormatTwo() {}
		Format x;
	};
	
	TEST_F(FormatTwo, TestBool) {
		char *p = x.parse(POD::ConstBuffer("foo{:b}bar"), true);
		ASSERT_STREQ(p, "foo1bar");
		
		char *q = x.parse(POD::ConstBuffer("foo{:o}bar"), true);
		ASSERT_STREQ(q, "foo1bar");
		
		char *r = x.parse(POD::ConstBuffer("foo{:d}bar"), true);
		ASSERT_STREQ(r, "foo1bar");
		
		char *s = x.parse(POD::ConstBuffer("foo{:x}bar"), true);
		ASSERT_STREQ(s, "foo1bar");
		
		// this will fail ATM
		char *t = x.parse(POD::ConstBuffer("foo {} bar"), true);
		ASSERT_STREQ(t, "foo true bar");
	}
	
	TEST_F(FormatTwo, TestChar) {
		char *p = x.parse(POD::ConstBuffer("foo {:d} bar"), 'a');
		ASSERT_STREQ(p, "foo 97 bar");
		
		char *q = x.parse(POD::ConstBuffer("foo {:x} bar"), 'a');
		ASSERT_STREQ(q, "foo 61 bar");
		
		// this will fail ATM
		char *r = x.parse(POD::ConstBuffer("foo {} bar"), 'a');
		ASSERT_STREQ(r, "foo a bar");
	}
	
	TEST_F(FormatTwo, TestUbyte) {
		char *p = x.parse(POD::ConstBuffer("foo {:d} bar"), static_cast<e_ubyte>(123));
		ASSERT_STREQ(p, "foo 123 bar");
		
		char *q = x.parse(POD::ConstBuffer("foo {} bar"), static_cast<e_ubyte>(123));
		ASSERT_STREQ(q, "foo 123 bar");
	}
}

int runFormatTests()
{
	return RUN_ALL_TESTS();
}