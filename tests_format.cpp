#include "format/Format.h"
#include "types.h"

#include <gtest/gtest.h>

#include <float.h>

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
		char *p = x.parse(POD::ConstBuffer("xxy {:g}"), 123);
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
	
	TEST_F(FormatExt, Test4) {
		char *p = x.parse(POD::ConstBuffer("foo{:g7}"), true);
		ASSERT_STREQ(p, "foo___tru");
		
		char *q = x.parse(POD::ConstBuffer("foo{,7:g6}"), true);
		ASSERT_STREQ(q, "foo __tru");
		
		char *r = x.parse(POD::ConstBuffer("foo{,10:g7}"), true);
		ASSERT_STREQ(r, "foo   ___");
		
		char *s = x.parse(POD::ConstBuffer("foo{,-10:g7}"), true);
		ASSERT_STREQ(s, "foo___tru");
		
		char *t = x.parse(POD::ConstBuffer("foo{,-10:g7}"), false);
		ASSERT_STREQ(t, "foo__fals");
		
		char *u = x.parse(POD::ConstBuffer("foo{,-6}"), false);
		ASSERT_STREQ(u, "foofalse ");
	}
	
	
	struct FormatTwo : public ::testing::Test {
		FormatTwo() {}
		Format x;
	};
	
	TEST_F(FormatTwo, TestBool) {
		char *p = x.parse(POD::ConstBuffer("foo{:b}bar"), true);
		ASSERT_STREQ(p, "foo1bar");
		
		char *q = x.parse(POD::ConstBuffer("foo{:o2}bar"), true);
		ASSERT_STREQ(q, "foo01bar");
		
		char *r = x.parse(POD::ConstBuffer("foo{:d3}bar"), true);
		ASSERT_STREQ(r, "foo001bar");
		
		char *s = x.parse(POD::ConstBuffer("foo{:x4}bar"), true);
		ASSERT_STREQ(s, "foo0001bar");
		
		// this will fail ATM
		char *t = x.parse(POD::ConstBuffer("foo {} bar"), true);
		ASSERT_STREQ(t, "foo true bar");
		
		char *u = x.parse(POD::ConstBuffer("foo {:g6} bar"), true);
		ASSERT_STREQ(u, "foo __true bar");
		
		char *v = x.parse(POD::ConstBuffer("foo {,10:g8} bar"), false);
		ASSERT_STREQ(v, "foo   ___false bar");
		
		char *w = x.parse(POD::ConstBuffer("foo {:z} bar"), false);
		ASSERT_STREQ(w, "foo {badspec} bar");
	}
	
	TEST_F(FormatTwo, TestChar) {
		char *p = x.parse(POD::ConstBuffer("foo {:d} bar"), 'a');
		ASSERT_STREQ(p, "foo 97 bar");
		
		char *q = x.parse(POD::ConstBuffer("foo {:x} bar"), 'a');
		ASSERT_STREQ(q, "foo 61 bar");
		
		// this will fail ATM
		char *r = x.parse(POD::ConstBuffer("foo {} bar"), 'a');
		ASSERT_STREQ(r, "foo a bar");
		
		char *s = x.parse(POD::ConstBuffer("foo {:g3} bar"), 'a');
		ASSERT_STREQ(s, "foo __a bar");
		
		char *t = x.parse(POD::ConstBuffer("foo {,-5:g3} bar"), 'a');
		ASSERT_STREQ(t, "foo __a   bar");
		
		char *u = x.parse(POD::ConstBuffer("foo {:z} bar"), 'a');
		ASSERT_STREQ(u, "foo {badspec} bar");
	}
	
	TEST_F(FormatTwo, TestUbyte) {
		char *p = x.parse(POD::ConstBuffer("foo {:d} bar"), static_cast<e_ubyte>(123));
		ASSERT_STREQ(p, "foo 123 bar");
		
		char *q = x.parse(POD::ConstBuffer("foo {} bar"), static_cast<e_ubyte>(123));
		ASSERT_STREQ(q, "foo 123 bar");
	}
	
	
	static void l_itoa(char *dest, void *ptr, size_t base = 16) {
		size_t l = 0;
		e_ulong i = (sizeof(void*) <= sizeof(e_uint)) ? reinterpret_cast<e_uint>(ptr) : reinterpret_cast<e_ulong>(ptr);
		e_ulong t = i;
		if (t) { while (t) { t /= base; l++; } }
		else l++;
		
		dest[l] = 0;
		do {
			char t = i % base;
			dest[--l] = t + (t < 10 ? '0' : ('a' - 10));
			i /= base;
		} while (i);
	}
	
	TEST_F(FormatTwo, TestCharPtr) {
		char realHello[] = "Hello, world!";
		char *hello = realHello;

		char *p = x.parse(POD::ConstBuffer("{}"), hello);
		ASSERT_STREQ(p, "Hello, world!");
		
		hello = 0;
		char *q = x.parse(POD::ConstBuffer("foo{}bar"), hello);
		ASSERT_STREQ(q, "foo{null}bar");
		
		char *r = x.parse(POD::ConstBuffer("{}"), realHello);
		ASSERT_STREQ(r, "Hello, world!");
		
		char *s = x.parse(POD::ConstBuffer("{:x}"), realHello);
		char foo[10];
		l_itoa(foo, realHello);
		ASSERT_STREQ(s, foo);
		
		const char* world = realHello;
		char *t = x.parse(POD::ConstBuffer("{}"), world);
		ASSERT_STREQ(t, "Hello, world!");
		
	}
	
	TEST_F(FormatTwo, TestOthPtr) {
		e_uint realUint[] = { 8,7,6,5,4,4,40 };
		e_ushort realUshort[] = { 8,7,6,5,4,4,40 };
		char foo[20];
		
		l_itoa(foo, realUint);
		char *p = x.parse(POD::ConstBuffer("{:x}"), realUint);
		ASSERT_STREQ(p, foo);
		
		l_itoa(foo, realUshort);
		char *q = x.parse(POD::ConstBuffer("{:x}"), realUshort);
		ASSERT_STREQ(q, foo);
		
		l_itoa(foo, realUint, 10);
		char *r = x.parse(POD::ConstBuffer("{:d}"), realUint);
		ASSERT_STREQ(r, foo);
		
		l_itoa(foo, realUshort, 8);
		char *s = x.parse(POD::ConstBuffer("{:o}"), realUshort);
		ASSERT_STREQ(s, foo);
		
		const e_ushort* tmpUshort = realUshort;
		char *t = x.parse(POD::ConstBuffer("{:o}"), tmpUshort);
		ASSERT_STREQ(t, foo);
	}
	
	struct FpTests : public ::testing::Test {
		FpTests() {}
		Format x;
	};
	
	TEST_F(FpTests, TestRadix) {
		// todo
		ASSERT_EQ(1, 0);
	}
	
	// These tests assume binary32/binary64 representation
	TEST_F(FpTests, TestPrecisionMalcolms) {
		union { float flt; e_uint bin; } bin32;
		bin32.flt = 1.f;
		
		//0 | 01111111 | 0000000 00000000 00000000
		ASSERT_EQ(bin32.bin, 0x3f800000);
		
		// assume exponent range is 127, and skip it
		for (size_t i=0; i<126; ++i) bin32.flt /= 2.f;
		
		//0 | 00000001 | 0000000 00000000 00000000
		ASSERT_EQ(bin32.bin, 0x00800000);
		
		size_t floatPrecision = 0;
		while (bin32.flt > 0.f) {
			bin32.flt /= 2.f;
			floatPrecision++;
		}
		ASSERT_EQ(floatPrecision, 24);
		
		union { double dbl; e_ulong bin; } bin64;
		bin64.dbl = 1.0;
		
		// 0 | 01111111111 | 0000000000000000000000000000000000000000000000000000
		ASSERT_EQ(bin64.bin, 0x3ff0000000000000ull);
		
		for (size_t i=0; i<1022; ++i) bin64.dbl /= 2.0;
		
		// 0 | 00000000001 | 0000000000000000000000000000000000000000000000000000
		ASSERT_EQ(bin64.bin, 0x0010000000000000ull);
		
		size_t doublePrecision = 0;
		while (bin64.dbl > 0.f) {
			bin64.dbl /= 2.f;
			doublePrecision++;
		}
		ASSERT_EQ(doublePrecision, 53);
	}
	
	TEST_F(FpTests, TestPr1) {
		float f = static_cast<float>(4294967295.0f * 0xfffffff0.fp0 + 10.f);
		char *p = x.parse(POD::ConstBuffer("piapprox {}"), f);
		char buf[100];
		
		sprintf(buf, "piapprox %10.0f", f);
		
		ASSERT_STREQ(buf, p);
	}
}

int runFormatTests()
{
	return RUN_ALL_TESTS();
}