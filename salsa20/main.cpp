/* The following implementation and tests, are only for educational
 * purposes, and shouldn't be used in software
 *
 * Mar. 2012, Michal 'GiM' Spadlinski
 */
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <memory.h>

#define ROTL32(a,b) (((a)<<(b))|((a)>>(32-(b))))

struct Salsa20
{
	static void qr(uint32_t y[4], uint32_t (&z)[4]) {
		z[1] = y[1] ^ ROTL32(y[0] + y[3],  7);
		z[2] = y[2] ^ ROTL32(z[1] + y[0],  9);
		z[3] = y[3] ^ ROTL32(z[2] + z[1], 13);
		z[0] = y[0] ^ ROTL32(z[3] + z[2], 18);
	}

	static void rowRound(uint32_t y[16], uint32_t (&z)[16]) {
		uint32_t temp[4];
		uint32_t yIn[4];

#define ToIn(a,b,c,d)  yIn[0] = y[a];   yIn[1] = y[b];   yIn[2] = y[c];   yIn[3] = y[d]
#define ToOut(a,b,c,d) z[a] = temp[0];  z[b] = temp[1];  z[c] = temp[2];  z[d] = temp[3]

		ToIn(0,1,2,3);
		qr(yIn, temp);
		ToOut(0,1,2,3);

		ToIn(5,6,7,4);
		qr(yIn, temp);
		ToOut(5,6,7,4);

		ToIn(10,11,8,9);
		qr(yIn, temp);
		ToOut(10,11,8,9);

		ToIn(15,12,13,14);
		qr(yIn, temp);
		ToOut(15,12,13,14);

#undef ToIn
#undef ToOut
	}

	static void colRound(uint32_t y[16], uint32_t (&z)[16]) {
		uint32_t temp[4];
		uint32_t yIn[4];

#define ToIn(a,b,c,d)  yIn[0] = y[a];   yIn[1] = y[b];   yIn[2] = y[c];   yIn[3] = y[d]
#define ToOut(a,b,c,d) z[a] = temp[0];  z[b] = temp[1];  z[c] = temp[2];  z[d] = temp[3]

		ToIn(0,4,8,12);
		qr(yIn, temp);
		ToOut(0,4,8,12);

		ToIn(5,9,13,1);
		qr(yIn, temp);
		ToOut(5,9,13,1);

		ToIn(10,14,2,6);
		qr(yIn, temp);
		ToOut(10,14,2,6);

		ToIn(15,3,7,11);
		qr(yIn, temp);
		ToOut(15,3,7,11);
	}

	static void doubleRound(uint32_t y[16], uint32_t (&z)[16]) {
		uint32_t temp[16];
		colRound(y, temp);
		rowRound(temp, z);
	}

	static void salsa20prg(uint8_t salsaInput[64], uint8_t (&salsaOutput)[64], int rounds = 10) {
		uint32_t *sIn = (uint32_t*)salsaInput;
		uint32_t *sOut = (uint32_t*)&salsaOutput;
		uint32_t rIn[16], rOut[16];

		memcpy(rIn, salsaInput, sizeof(rIn));
		for (int i=0; i<rounds; ++i) {
			doubleRound(rIn, rOut);
			memcpy(rIn, rOut, sizeof(rIn));
		}

		for (int i=0; i<16; ++i) {
			sOut[i] = rIn[i] + sIn[i];
		}
	}

	static void salsa20exp2(uint8_t k0[16], uint8_t k1[16], uint8_t n[16], uint8_t (&salsaExpOutput)[64]) {
#define LENDIAN(a,b,c,d) (((d)<<24)|((c)<<16)|((b)<<8)|(a))
		// from spec: "expand 32-byte k"
		static const uint32_t t0 = LENDIAN(101, 120, 112, 97);
		static const uint32_t t1 = LENDIAN(110, 100,  32, 51);
		static const uint32_t t2 = LENDIAN( 50,  45,  98, 121);
		static const uint32_t t3 = LENDIAN(116, 101,  32, 107);

		uint8_t tempIn[64];
		uint32_t *tempIn32 = (uint32_t*)tempIn;
		
		tempIn32[0] = t0;
		memcpy(tempIn + 4*1, k0, 16);
		tempIn32[5] = t1;
		memcpy(tempIn + 4*2 + 16, n, 16);
		tempIn32[10] = t2;
		memcpy(tempIn + 4*3 + 32, k1, 16);
		tempIn32[15] = t3;

		salsa20prg(tempIn, salsaExpOutput);
#undef LENDIAN
	}

	static void salsa20exp1(uint8_t k[16], uint8_t n[16], uint8_t (&salsaExpOutput)[64]) {
#define LENDIAN(a,b,c,d) (((d)<<24)|((c)<<16)|((b)<<8)|(a))
		// from spec: "expand 16-byte k"
		static const uint32_t t0 = LENDIAN(101, 120, 112, 97);
		static const uint32_t t1 = LENDIAN(110, 100,  32, 49);
		static const uint32_t t2 = LENDIAN( 54,  45,  98, 121);
		static const uint32_t t3 = LENDIAN(116, 101,  32, 107);

		uint8_t tempIn[64];
		uint32_t *tempIn32 = (uint32_t*)tempIn;

		tempIn32[0] = t0;
		memcpy(tempIn + 4*1, k, 16);
		tempIn32[5] = t1;
		memcpy(tempIn + 4*2 + 16, n, 16);
		tempIn32[10] = t2;
		memcpy(tempIn + 4*3 + 32, k, 16);
		tempIn32[15] = t3;

		salsa20prg(tempIn, salsaExpOutput);
#undef LENDIAN
	}

	// for encryption in-place make encoded == message
	static void salsa20(uint8_t *message, size_t messageLen, uint8_t* encoded, uint8_t nonce[8], uint8_t key[32]) {
		uint64_t i = 0;
		uint8_t v[16];
		memcpy(v, nonce, 8);
		
		size_t cnt;
		for (cnt=0; cnt < messageLen / 64; ++cnt) {
			uint8_t currentKey[64];
			*(uint64_t*)(v+8) = i;
			salsa20exp2(key, key+16, v, currentKey);

			for (int j=0; j<64; ++j) {
				encoded[cnt*64 + j] = message[cnt*64 + j] ^ currentKey[j];
			}
			++i;
		}
		if (messageLen % 64) {
			uint8_t currentKey[64];
			*(uint64_t*)(v+8) = i;
			salsa20exp2(key, key+16, v, currentKey);

			int limit = (int)(messageLen % 64);
			for (int j=0; j<limit; ++j) {
				encoded[cnt*64 + j] = message[cnt*64 + j] ^ currentKey[j];
			}
		}
	}
};

void check(int num, bool isSuccess) {
	if (! isSuccess) {
		::abort();
	}
	::printf("test[%d] passed\n", num);
}

void testQr(int& testNo) {
	Salsa20 s;

	uint32_t test1[4] = { 0 };
	uint32_t outData[4];
	s.qr(test1, outData);
	check(testNo++, (0x00000000 == outData[0]) && (0x00000000 == outData[1]) && (0x00000000 == outData[2]) && (0x00000000 == outData[3]));

	test1[0] = 1;
	s.qr(test1, outData);
	check(testNo++, (0x08008145 == outData[0]) && (0x00000080 == outData[1]) && (0x00010200 == outData[2]) && (0x20500000 == outData[3]));

	test1[0] = 0;
	test1[1] = 1;
	s.qr(test1, outData);
	check(testNo++, (0x88000100 == outData[0]) && (0x00000001 == outData[1]) && (0x00000200 == outData[2]) && (0x00402000 == outData[3]));

	test1[1] = 0;
	test1[2] = 1;
	s.qr(test1, outData);
	check(testNo++, (0x80040000 == outData[0]) && (0x00000000 == outData[1]) && (0x00000001 == outData[2]) && (0x00002000 == outData[3]));

	test1[2] = 0;
	test1[3] = 1;
	s.qr(test1, outData);
	check(testNo++, (0x00048044 == outData[0]) && (0x00000080 == outData[1]) && (0x00010000 == outData[2]) && (0x20100001 == outData[3]));

	test1[0] = 0xe7e8c006;
	test1[1] = 0xc4f9417d;
	test1[2] = 0x6479b4b2;
	test1[3] = 0x68c67137;
	s.qr(test1, outData);
	check(testNo++, (0xe876d72b == outData[0]) && (0x9361dfd5 == outData[1]) && (0xf1460244 == outData[2]) && (0x948541a3 == outData[3]));
}

void testRr(int& testNo) {
	uint32_t z[16];
	uint32_t y[16] = {
		0x08521bd6, 0x1fe88837, 0xbb2aa576, 0x3aa26365,
		0xc54c6a5b, 0x2fc74c2f, 0x6dd39cc3, 0xda0a64f6,
		0x90a2f23d, 0x067f95a6, 0x06b35f61, 0x41e4732e,
		0xe859c100, 0xea4d84b7, 0x0f619bff, 0xbc6e965a
	};

	Salsa20::rowRound(y, z);

	static const uint32_t correct[16] = {
		0xa890d39d, 0x65d71596, 0xe9487daa, 0xc8ca6a86,
		0x949d2192, 0x764b7754, 0xe408d9b9, 0x7a41b4d1,
		0x3402e183, 0x3c3af432, 0x50669f96, 0xd89ef0a8,
		0x40ede5, 0xb545fbce, 0xd257ed4f, 0x1818882d
	};

	for (int i = 0; i < 16; ++i) {
		if (z[i] != correct[i]) {
			check(testNo, false);
		}
	}
	check(testNo++, true);
}

void testCr(int& testNo) {
	uint32_t y[16];
	uint32_t x[16] = {
		0x08521bd6, 0x1fe88837, 0xbb2aa576, 0x3aa26365,
		0xc54c6a5b, 0x2fc74c2f, 0x6dd39cc3, 0xda0a64f6,
		0x90a2f23d, 0x067f95a6, 0x06b35f61, 0x41e4732e,
		0xe859c100, 0xea4d84b7, 0x0f619bff, 0xbc6e965a
	};
	Salsa20::colRound(x, y);
	static const uint32_t correct[16] = {
		0x8c9d190a, 0xce8e4c90, 0x1ef8e9d3, 0x1326a71a,
		0x90a20123, 0xead3c4f3, 0x63a091a0, 0xf0708d69,
		0x789b010c, 0xd195a681, 0xeb7d5504, 0xa774135c,
		0x481c2027, 0x53a8e4b5, 0x4c1f89c5, 0x3f78c9c8
	};

	for (int i = 0; i < 16; ++i) {
		if (y[i] != correct[i]) {
			check(testNo, false);
		}
	}
	check(testNo++, true);
}

void testDr(int& testNo) {
	uint32_t y[16];
	uint32_t x[16] = {
		0xde501066, 0x6f9eb8f7, 0xe4fbbd9b, 0x454e3f57,
		0xb75540d3, 0x43e93a4c, 0x3a6f2aa0, 0x726d6b36,
		0x9243f484, 0x9145d1e8, 0x4fa9d247, 0xdc8dee11,
		0x054bf545, 0x254dd653, 0xd9421b6d, 0x67b276c1
	};
	Salsa20::doubleRound(x, y);
	static const uint32_t correct[16] = {
		0xccaaf672, 0x23d960f7, 0x9153e63a, 0xcd9a60d0,
		0x50440492, 0xf07cad19, 0xae344aa0, 0xdf4cfdfc,
		0xca531c29, 0x8e7943db, 0xac1680cd, 0xd503ca00,
		0xa74b2ad6, 0xbc331c5c, 0x1dda24c7, 0xee928277
	};

	for (int i = 0; i < 16; ++i) {
		if (y[i] != correct[i]) {
			check(testNo, false);
		}
	}
	check(testNo++, true);
} 

void testSalsaPrg(int& testNo) {
	uint8_t salsaInput1[64] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	uint8_t salsaOutput[64];

	Salsa20::salsa20prg(salsaInput1, salsaOutput);

	for (int i = 0; i < 64; ++i) {
		if (salsaOutput[i] != salsaInput1[i]) {
			check(testNo, false);
		}
	}
	check(testNo++, true);

	uint8_t salsaInput2[64] = {
		211,159, 13,115, 76, 55, 82,183, 3,117,222, 37,191,187,234,136,
		49,237,179, 48, 1,106,178,219,175,199,166, 48, 86, 16,179,207,
		31,240, 32, 63, 15, 83, 93,161,116,147, 48,113,238, 55,204, 36,
		79,201,235, 79, 3, 81,156, 47,203, 26,244,243, 88,118,104, 54
	};	
	Salsa20::salsa20prg(salsaInput2, salsaOutput);
	static const uint8_t correct2[64] = {
		109, 42,178,168,156,240,248,238,168,196,190,203, 26,110,170,154,
		29, 29,150, 26,150, 30,235,249,190,163,251, 48, 69,144, 51, 57,
		118, 40,152,157,180, 57, 27, 94,107, 42,236, 35, 27,111,114,114,
		219,236,232,135,111,155,110, 18, 24,232, 95,158,179, 19, 48,202
	};
	for (int i = 0; i < 64; ++i) {
		if (salsaOutput[i] != correct2[i]) {
			check(testNo, false);
		}
	}
	check(testNo++, true);


	uint8_t salsaInput3[64] = {
		88,118,104, 54, 79,201,235, 79, 3, 81,156, 47,203, 26,244,243,
		191,187,234,136,211,159, 13,115, 76, 55, 82,183, 3,117,222, 37,
		86, 16,179,207, 49,237,179, 48, 1,106,178,219,175,199,166, 48,
		238, 55,204, 36, 31,240, 32, 63, 15, 83, 93,161,116,147, 48,113
	};
	Salsa20::salsa20prg(salsaInput3, salsaOutput);
	static const uint8_t correct3[64] = {
		179, 19, 48,202,219,236,232,135,111,155,110, 18, 24,232, 95,158,
		26,110,170,154,109, 42,178,168,156,240,248,238,168,196,190,203,
		69,144, 51, 57, 29, 29,150, 26,150, 30,235,249,190,163,251, 48,
		27,111,114,114,118, 40,152,157,180, 57, 27, 94,107, 42,236, 35
	};
	for (int i = 0; i < 64; ++i) {
		if (salsaOutput[i] != correct3[i]) {
			check(testNo, false);
		}
	}
	check(testNo++, true);

	uint8_t salsaInput4[64] = {
		6,124, 83,146, 38,191, 9, 50, 4,161, 47,222,122,182,223,185,
		75, 27, 0,216, 16,122, 7, 89,162,104,101,147,213, 21, 54, 95,
		225,253,139,176,105,132, 23,116, 76, 41,176,207,221, 34,157,108,
		94, 94, 99, 52, 90,117, 91,220,146,190,239,143,196,176,130,186
	};
	for (int j=0; j<1000000; ++j) {
		Salsa20::salsa20prg(salsaInput4, salsaOutput);
		memcpy(salsaInput4, salsaOutput, sizeof(salsaInput4));
	}
	static const uint8_t correct4[64] = {
		8, 18, 38,199,119, 76,215, 67,173,127,144,162,103,212,176,217,
		192, 19,233, 33,159,197,154,160,128,243,219, 65,171,136,135,225,
		123, 11, 68, 86,237, 82, 20,155,133,189, 9, 83,167,116,194, 78,
		122,127,195,185,185,204,188, 90,245, 9,183,248,226, 85,245,104
	};
	for (int i = 0; i < 64; ++i) {
		if (salsaOutput[i] != correct4[i]) {
			check(testNo, false);
		}
	}
	check(testNo++, true);
}

void testSalsaExp2(int& testNo) {
	uint8_t k0[16] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };
	uint8_t k1[16] = { 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216 };
	uint8_t n[16]  = { 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116 };
	uint8_t salsaExp2Output[64];

	{
		Salsa20::salsa20exp2(k0, k1, n, salsaExp2Output);
		static const uint8_t correct[64] = {
			69, 37, 68, 39, 41, 15,107,193,255,139,122, 6,170,233,217, 98,
			89,144,182,106, 21, 51,200, 65,239, 49,222, 34,215,114, 40,126,
			104,197, 7,225,197,153, 31, 2,102, 78, 76,176, 84,245,246,184,
			177,160,133,130, 6, 72,149,119,192,195,132,236,234,103,246, 74
		};
		for (int i = 0; i < 64; ++i) {
			if (salsaExp2Output[i] != correct[i]) {
				check(testNo, false);
			}
		}
		check(testNo++, true);
	}

	{
		Salsa20::salsa20exp1(k0, n, salsaExp2Output);
		static const uint8_t correct[64] = {
			39,173, 46,248, 30,200, 82, 17, 48, 67,254,239, 37, 18, 13,247,
			241,200, 61,144, 10, 55, 50,185, 6, 47,246,253,143, 86,187,225,
			134, 85,110,246,161,163, 43,235,231, 94,171, 51,145,214,112, 29,
			14,232, 5, 16,151,140,183,141,171, 9,122,181,104,182,177,193
		};
		for (int i = 0; i < 64; ++i) {
			if (salsaExp2Output[i] != correct[i]) {
				check(testNo, false);
			}
		}
		check(testNo++, true);
	}
}

void testSalsaEncryption(int& testNo)
{
	char message[] = "What if a cyber brain could generate it's own ghost, create a soul all by itself?\n"
		"And if it did, just what would be the importance of being human then?\n";
	uint8_t encoded[sizeof(message)];
	uint8_t decoded[sizeof(message)];

	uint8_t nonce[8] = {1,2,3,4,5,6,7,8};
	uint8_t key[32] = {
		0xed, 0x12, 0x9e, 0x12, 0xd8, 0xc1, 0x3f, 0xae,
		0x7e, 0xa4, 0x76, 0x32, 0x51, 0xc7, 0xb2, 0x1b,
		0xf1, 0xbf, 0xeb, 0xfa, 0xfa, 0x01, 0x6f, 0x62,
		0x36, 0x1, 0xbc, 0xd0, 0x32, 0x83, 0xe2, 0xa5
	};

	Salsa20::salsa20((uint8_t*)message, sizeof(message), encoded, nonce, key);
	Salsa20::salsa20(encoded, sizeof(message), decoded, nonce, key);

	for (size_t i=0; i<sizeof(message); ++i) {
		if (decoded[i] != message[i]) {
			check(testNo, false);
		}
	}
	::printf("%s", decoded);
	check(testNo++, true);
}

int main()
{
	int testNumber = 0;
	testQr(testNumber);
	testRr(testNumber);
	testCr(testNumber);
	testDr(testNumber);
	testSalsaPrg(testNumber);

	testSalsaExp2(testNumber);

	testSalsaEncryption(testNumber);
	return 0;
}
