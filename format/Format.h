/** Format.h
 * String format routines
 * 
 * PileOfCrap (http://code.google.com/p/pileofcrap/)
 * 2011 Michal 'GiM' Spadlinski
 */
#ifndef GIM_FORMAT_FORMAT_H
#define GIM_FORMAT_FORMAT_H

#include <iostream>

#include <cstring>

#include "../types.h"

// so the goal is to have (very limited) implementation of String.Format in C++
// here are some constraints though:
//   + I want it to be C++ (which kinda sux cause using C++0x features, would make it easier)
//   + no exceptions
//   + I don't care about resulting size (of executable ;p)
//   + there won't be support for time/date formatting
//   + similarily there won't be support for 'currency', cause I don't want
//      and don't need to play with locale
// I'm not quite sure yet if I will actually finish this
template <size_t Buf_Size = 1024>
class FormatB {
	char dataBuf[Buf_Size];
	int pos; // pos in dataBuf buffer
	typedef char Ch;
	const Ch* p;
	const Ch* end;
	const Ch* last;
	
	// those variables will be needed across different funcs
	int index;
	bool indexPresent;
	int alignment;
	bool alignmentSign;
	bool alignmentPresent; // I could probably remove this and just check if alignment != 0 instead
	POD::ConstBuffer currentFormat;
	bool needsProcessing;
	
	float fltPowerz[254+23];
	
	public:
	FormatB() : currentFormat(0, 0) {
		union { float flt; e_uint bin; } bin32;
		bin32.flt = 1.0;
		for (size_t i = 0; i < 127; ++i) bin32.flt *= 2.f;
		
		for (size_t i = 0; i < 254 + 23; ++i) {
			fltPowerz[i] = bin32.flt;
			bin32.flt /= 2.f;
		}
	}
	
	size_t eat(const POD::ConstBuffer& buf) {
		// space left
		size_t toWrite = Buf_Size-1 - pos;
		if (buf.len < toWrite) {
			toWrite = buf.len;
		}
		//std::cout << "eating... " << buf.len << " " << buf.ptr << std::endl;
		memcpy(dataBuf+pos, buf.ptr, toWrite);
		pos += toWrite;
	}
	
	size_t fill(size_t count, char filler = ' ') {
		size_t toWrite = Buf_Size-1 - pos;
		if (count < toWrite) {
			toWrite = count;
		}
		
		memset(dataBuf+pos, filler, toWrite);
		pos += toWrite;
	}
	
	static size_t fill(char *buf, size_t toWrite, size_t count, char filler = ' ') {
		if (count < toWrite) {
			toWrite = count;
		}
		
		memset(buf, filler, toWrite);
		return toWrite;
	}

	struct FormatSpecifier {
		int mode;
		bool hexUpper;
		size_t maxPrecision;
	};
	
	int parseIntFormat(FormatSpecifier& ret, int defaultMode = 10) {
		const char *f = currentFormat.ptr;
		const char *fEnd = currentFormat.ptr + currentFormat.len;
		
		// default mode is decimal
		ret.mode = defaultMode;
		ret.hexUpper = false;
		ret.maxPrecision = 0;
		if (f && f != fEnd) {
			switch (*f) {
				case 'b': case 'B': ret.mode=2; break;
				case 'o': case 'O': ret.mode=8; break;
				case 'd': case 'D': ret.mode=10; break;
				case 'X':
					ret.hexUpper = true;
				case 'x': ret.mode=16; break;
				
				case 'g':
					if (defaultMode != 10) {
						ret.mode = 0;
						break;
					}
				default:
					eat(POD::ConstBuffer("{badspec}",9));
					return 1;
			}
			f++;
			if (f != fEnd) {
				while ((*f) >= '0' && (*f) <= '9') {
					ret.maxPrecision *= 10;
					ret.maxPrecision += (*f++) - '0';
				}
				if (f != fEnd) {
					eat(POD::ConstBuffer("{badspec}",9));
					return 1;
				}
			}
		}
		return 0;
	}
	
	int parseFloatFormat(FormatSpecifier& ret) {
		const char *f = currentFormat.ptr;
		const char *fEnd = currentFormat.ptr + currentFormat.len;
		
		// default mode is decimal
		ret.mode = 10;
		ret.hexUpper = false;
		ret.maxPrecision = 0;
		if (f && f != fEnd) {
			switch (*f) {
				case 'f': case 'F': break;
				default:
					eat(POD::ConstBuffer("{badspec}",9));
					return 1;
			}
			f++;
			if (f != fEnd) {
				while ((*f) >= '0' && (*f) <= '9') {
					ret.maxPrecision *= 10;
					ret.maxPrecision += (*f++) - '0';
				}
				if (f != fEnd) {
					eat(POD::ConstBuffer("{badspec}",9));
					return 1;
				}
			}
		}
		return 0;
	}

	void print(char x)     {
		//std::cout<< "char   " << std::endl;
		FormatSpecifier fs;
		if (parseIntFormat(fs, 0))
			return;
		realIntPrint<CharPrinter>(static_cast<e_ulong>(x), fs, x < 0 ? 1 : 0);
	}
	
	void print(bool x)     {
		//std::cout<< "bool   " << std::endl;
		FormatSpecifier fs;
		if (parseIntFormat(fs, 0))
			return;
		realIntPrint<BoolPrinter>(static_cast<e_ulong>(x), fs, x < 0 ? 1 : 0);
	}
	
	template <class T>
	e_ulong ptrToUlong(T* ptr) {
		// the code below assumes, that sizeof(T*) >= 4
		if (sizeof(T*) <= sizeof(e_uint)) {
			return reinterpret_cast<e_uint>(ptr);
		
		} else {
			return reinterpret_cast<e_ulong>(ptr);
		}
	}
	
	template <class T>
	void print(T* x) {
		FormatSpecifier fs;
		if (parseIntFormat(fs, 16))
			return;
		
		realIntPrint<IntPrinter>(ptrToUlong(x), fs, 0);
	}
	
	// overload instead of specialization
	void print(const char* x) {
		FormatSpecifier fs;
		if (parseIntFormat(fs, 0))
			return;
		
		realIntPrint<CharPtrPrinter>(ptrToUlong(x), fs, 0);
	}
	
	void print(char* x) {
		print(const_cast<const char*>(x));
	}
	
	void print(e_byte x)   { print(static_cast<e_ulong>(x), x < 0 ? 1 : 0); }
	void print(e_short x)  { print(static_cast<e_ulong>(x), x < 0 ? 1 : 0); }
	void print(e_int x)    { print(static_cast<e_ulong>(x), x < 0 ? 1 : 0); }
	void print(e_long x)   { print(static_cast<e_ulong>(x), x < 0 ? 1 : 0); }

	void print(e_ubyte x)  { print(static_cast<e_ulong>(x)); }
	void print(e_ushort x) { print(static_cast<e_ulong>(x)); }
	void print(e_uint x)   { print(static_cast<e_ulong>(x)); }
	
	void print(e_ulong x, int hasSign = 0) {
		FormatSpecifier fs;
		if (parseIntFormat(fs))
			return;
		realIntPrint<IntPrinter>(x, fs, hasSign);
	}
	
	struct IntPrinter {
		static size_t getLen(e_long x, int mode) {
			size_t l = 0;
			// calc len, log could be used, but I want to avoid it
			e_ulong t=x;
			while (t) {
				l++;
				t /= mode;
			}
			if (!x) l++;
			return l;
		}
		
		static void print(char* buf, size_t bufLen, e_ulong x, const FormatSpecifier& fs, int hasSign) {
			size_t maxPrecision = fs.maxPrecision;
			bool hexUpper = fs.hexUpper;
			int mode = fs.mode;
			
			// skip thelast digits if there's no place to print them
			for (; maxPrecision > hasSign && maxPrecision > bufLen; maxPrecision--) {
				x /= mode;
			}
			
			// first check if there's any sense to do the loop over the digits
			if (hasSign < bufLen) {
				// loops continues down to 'hasSign' in order
				// to leave a space for a '-' if there's such a need
				if (mode != 16) {
					for (; maxPrecision > hasSign; maxPrecision--) {
						buf[maxPrecision-1] = '0' + (x % mode);
						x /= mode;
					}
					
				} else {
					for (; maxPrecision > hasSign; maxPrecision--) {
						int t = (x % mode);
						if (t>9) {
							buf[maxPrecision-1] = (hexUpper?'A':'a')+(t-10);
							
						} else {
							buf[maxPrecision-1] = '0' + t;
						}
						x /= mode;
					}
				}
			}
			// add sign (if there's a place for that)
			if (hasSign && bufLen) { buf[maxPrecision-1] = '-'; }
		}
	};
	
	struct CharPrinter {
		static size_t getLen(e_long x, int mode) {
			if (0 == mode) {
				return 1;
		
			} else {
				return IntPrinter::getLen(x, mode);
			}
		}
		
		static void print(char* buf, size_t bufLen, e_ulong x, const FormatSpecifier& fs, int hasSign) {
			if (0 == fs.mode) {
				size_t maxPrecision = fs.maxPrecision;
				size_t noOfZeroes = maxPrecision - 1;
				size_t written = fill(buf, bufLen, noOfZeroes, '_');
				bufLen -= written;
				buf += written;
				if (bufLen)
					buf[0] = static_cast<char>(x);
				
			} else {
				IntPrinter::print(buf, bufLen, x, fs, hasSign);
			}
		}
	};
	
	struct BoolPrinter {
		static size_t getLen(e_long x, int mode) {
			if (0 == mode) {
				return x ? 4 : 5;
		
			} else {
				return IntPrinter::getLen(x, mode);
			}
		}
		
		static void print(char* buf, size_t bufLen, e_ulong x, const FormatSpecifier& fs, int hasSign) {
			if (0 == fs.mode) {
				size_t maxPrecision = fs.maxPrecision;
				size_t noOfZeroes = maxPrecision - (x ? 4 : 5);
				size_t written = fill(buf, bufLen, noOfZeroes, '_');
				bufLen -= written;
				buf += written;
				// 'Q'-s are currently added for test only purposes
				if (x) {
					static const char strTrue[] = "trueQQQQQQ";
					memcpy(buf, strTrue, bufLen);
					
				} else {
					static const char strFalse[] = "falseQQQQQ";
					memcpy(buf, strFalse, bufLen);
				}
				
			} else {
				IntPrinter::print(buf, bufLen, x, fs, hasSign);
			}
		}
	};
	
	struct CharPtrPrinter {
		static size_t getLen(e_long x, int mode) {
			if (0 == mode) {
				return x ? strlen(reinterpret_cast<char*>(x)) : 6;
		
			} else {
				return IntPrinter::getLen(x, mode);
			}
		}
		
		static void print(char* buf, size_t bufLen, e_ulong x, const FormatSpecifier& fs, int hasSign) {
			if (0 == fs.mode) {
				size_t maxPrecision = fs.maxPrecision;
				size_t strLen = x ? strlen(reinterpret_cast<char*>(x)) : 6;
				size_t noOfZeroes = maxPrecision - strLen;
				size_t written = fill(buf, bufLen, noOfZeroes, '_');
				bufLen -= written;
				buf += written;
				// 'Q'-s are currently added for test only purposes
				if (x) {
					memcpy(buf, reinterpret_cast<char*>(x), bufLen);
					
				} else {
					static const char strFalse[] = "{null}QQQQQ";
					memcpy(buf, strFalse, bufLen);
				}
				
			} else {
				IntPrinter::print(buf, bufLen, x, fs, hasSign);
			}
		}
	};
	
	template <class TypePrinter>
	void realIntPrint(e_ulong x, const FormatSpecifier& readOnlyFs, int hasSign) {
		FormatSpecifier fs = readOnlyFs;
		// sign is only sensible in decimal mode
		if (fs.mode != 10) { hasSign = 0; }
		
		// fix the value, this should be ok for extreme values
		if (hasSign && fs.mode == 10) {
			x = -static_cast<e_long>(x);
		}
		
		size_t l = TypePrinter::getLen(x, fs.mode);
		if (fs.maxPrecision < l) {
			fs.maxPrecision = l;
		}
		if (hasSign) { fs.maxPrecision++; }
		
		if (alignmentPresent && alignment) {
			// in this case alignment doesn't apply
			if (alignment < fs.maxPrecision) {
				alignment = 0;
			}
		}
		
		alignment && !alignmentSign && fill(alignment - fs.maxPrecision);
		
		size_t toWrite = Buf_Size-1 - pos;
		if (fs.maxPrecision < toWrite) {
			toWrite = fs.maxPrecision;
		}
		TypePrinter::print(dataBuf + pos, toWrite, x, fs, hasSign);
		pos += toWrite;
		
		alignment && alignmentSign && fill(alignment - fs.maxPrecision);
	}
	
	void print(float x) {
		FormatSpecifier fs;
		if (parseFloatFormat(fs))
			return;
		
		realFltPrint<FloatPrinter>(x, fs, x < 0.0f);
	}
	
	struct FloatPrinter {
		static const size_t Number_Decimal_Digits = 8;
		static const size_t Size = 32;
		static const size_t Precision = 24;
		static const size_t Precision_1 = 23;
		static const size_t Exponent_Mask = 0xff;
		
		
		// return maximal value
		static size_t getLen(float x, int mode) {
			return 66;
		}
	};

	// ATM will work only with Bit_Count == 256
	template <size_t Bit_Count>
	class SimpleUint {
	public:
		typedef SimpleUint<Bit_Count> _Myself;
		static const size_t Bits_Per_Ulong = (sizeof(e_ulong) * 8);
		static const size_t Ulong_Count = Bit_Count / Bits_Per_Ulong;
	private:
		e_ulong d[Ulong_Count + 1];
	public:
		static const size_t Byte_Count = sizeof(d);
		
		explicit SimpleUint(e_ulong val) {
			memset(d, 0, Byte_Count);
			d[Ulong_Count - 1] = val;
		}
		
		SimpleUint(const _Myself& cp) {
			memcpy(d, cp.d, Byte_Count);
		}
		
		SimpleUint& operator=(const _Myself& other) {
			if (this != &other) {
				memcpy(d, other.d, Byte_Count);
			}
		}
		
		SimpleUint& operator <<=(int arg) {
			arg &= (Bit_Count - 1);
			
			if (arg < 64) {
				size_t i = 0;
				for (; i < Ulong_Count; ++i) {
					d[i] <<= arg;
					d[i] |= (d[i+1] >> (Bits_Per_Ulong - arg));
				}
				d[Ulong_Count] <<= arg;
				
			}
			return *this;
		}
		
		SimpleUint& operator >>=(int arg) {
			arg &= (Bit_Count -1);
			if (arg < 64) {
				size_t i = Ulong_Count;
				for (; i > 0; --i) {
					d[i] >>= arg;
					d[i] |= (d[i-1] << (Bits_Per_Ulong - arg));
				}
				d[0] >>= arg;
				
			} else if (arg && (0 == (arg % Bits_Per_Ulong))) {
				arg /= Bits_Per_Ulong;
				if (arg <= Ulong_Count) {
					for (size_t i = Ulong_Count; i >= arg; --i) {
						d[i] = d[i-arg];
					}
					for (size_t i=0; i<arg; ++i) {
						d[i] = 0;
					}
				}
			}
		}
		
		SimpleUint& operator+=(const _Myself& other) {
			bool carry = false;
			for (size_t i = Ulong_Count; i>0; --i) {
				bool newCarry = false;
				
				if (d[i] + other.d[i] < d[i]) newCarry = true;
				d[i] += other.d[i];
				
				if (d[i] + 1 < d[i]) newCarry = true;
				
				if (carry)
					d[i]++;
				
				carry = newCarry;
			}
			d[0] += other.d[0];
			d[0] += carry;
			
			return *this;
		}
		
		SimpleUint& operator-() {
			bool isZero = true;
			for (size_t i = 0; i <= Ulong_Count && isZero; ++i) {
				if (d[i])
					isZero = false;
			}
			if (!isZero) {
				for (size_t i = 0; i <= Ulong_Count; ++i) {
					d[i] ^= -1;
				}
				
				if (d[0] >> 63) {
					size_t i;
					for (i = Ulong_Count; i > 0; --i) {
						if (d[i] + 1 < d[i]) {
							d[i] = 0;
							
						} else {
							d[i]++;
							break;
						}
					}
					if (i == 0) {
						d[0]++;
					}
				}
			}
			return *this;
		}
		
		void print() {
			std::cout << std::hex;
			for (size_t i = 0; i <= Ulong_Count; ++i) {
				std::cout << d[i] << " ";
			}
			std::cout << std::dec << std::endl;
		}
		
		bool aboveZero() {
			bool nonZero = false;
			// < and not <= !!!
			for (size_t i = 0; i < Ulong_Count && !nonZero; ++i) {
				if (d[i])
					nonZero = true;
			}
			return nonZero;
		}
		
		e_uint modDiv10() {
			_Myself temp(*this);
			_Myself orig(*this);

			/*
			temp >>= 1;
			(*this) += temp;
			(*this) >>= 1;
			
			temp = (*this); temp >>= 4; (*this) += temp;
			
			temp = (*this); temp >>= 8; (*this) += temp;
			
			temp = (*this); temp >>= 16; (*this) += temp;
			
			temp = (*this); temp >>= 32; (*this) += temp;
			
			temp = (*this); temp >>= 64; (*this) += temp;
			
			temp = (*this); temp >>= 128; (*this) += temp;
			
			(*this) >>= 3;
			*/

			//   XXXXXXXX XXYYYYYY YYYYZZZZ 00000000 00000000 00000000 00000000 00000000
			// ========
			// + ...XXXXX XXXXXYYY YYYYYYYZ ZZZ00000 00000000 00000000 00000000 00000000
			// - .....XXX XXXXXXXY YYYYYYYY YZZZZ000 00000000 00000000 00000000 00000000
			// + .......X XXXXXXXX XYYYYYYY YYYZZZZ0 00000000 00000000 00000000 00000000
			// : : : : :
			// + ........ ........ ........ ........ ........ ...XXXXX XXXYYYYY YYYZZZZ0
			// - ........ ........ ........ ........ ........ .....XXX XXXXXYYY YYYYYZZZ
			// : : : : :
			// + ........ ........ ........ ........ ........ ........ ........ .......X

			temp >>= 2;
			(*this) += temp;
			(*this) >>= 1;
			
			for (size_t i=0; i < (256 / 4)-1; ++i) {
				(*this) += orig; (*this) >>= 3;
				(*this) += orig; (*this) >>= 1;
			}
			
			(*this) += orig;
			(*this) >>= 4;
			
			temp = (*this);
			temp.d[Ulong_Count] = 0;
			_Myself temp2(temp);
			temp <<= 3;
			temp2 <<= 1;
			
			//std::cout << "temp: "; temp.print();			
			//std::cout << "orig: "; orig.print();
			orig += -temp;
			orig += -temp2;
			//std::cout << "rslt: "; orig.print();
			
			return orig.d[Ulong_Count - 1] % 10;
		}
	};
	
	template <class TypePrinter>
	void realFltPrint(float x, const FormatSpecifier& readOnlyFs, int hasSign) {
		FormatSpecifier fs = readOnlyFs;
		union { float flt; e_uint bin; } bin32;
		
		bin32.flt = x;
		
		// needs special treatment
		e_int exponent = ((bin32.bin >> TypePrinter::Precision_1) & TypePrinter::Exponent_Mask);
		if (exponent == TypePrinter::Exponent_Mask) {
			hasSign = 0;
			std::cout << ((bin32.bin >> (TypePrinter::Size - 1))?'-':'+');
			if (bin32.bin & ((1ull << TypePrinter::Precision_1) - 1)) {
				std::cout << "nan" << std::endl;
			} else {
				std::cout << "inf" << std::endl;
			}
			
		} else {
			if ((bin32.bin >> (TypePrinter::Size - 1))) {
				hasSign = 1;
			}
			// not sure if this is ok :]
			if (hasSign) {
				x = -x;
			}
			
			size_t l = TypePrinter::getLen(x, fs.mode);
			
			bin32.flt = x;
			exponent -= 127;
			e_ulong intPart = bin32.bin;
			intPart &= ((1ull << TypePrinter::Precision_1) - 1);
			intPart |= (1ull << TypePrinter::Precision_1);
			
			if (exponent >= 0) {
				if (exponent > TypePrinter::Precision_1 && exponent < 64) {
					intPart <<= (exponent - TypePrinter::Precision_1);
					
				} else if (exponent <= TypePrinter::Precision_1) {
					intPart >>= (TypePrinter::Precision_1 - exponent);
				
				} else if (exponent == 64 && exponent) {
					char buf[100];
					//std::cout << "exp:" << exponent << " " << std::hex << intPart << std::dec << std::endl;
					
					SimpleUint<256> suint(intPart);
					suint <<= (exponent - TypePrinter::Precision_1);
					
					//suint.print();
					
					size_t i = 0;
					while (suint.aboveZero()) {
						buf[i++] = '0' + suint.modDiv10();
					}
					buf[i] = 0;
					
					size_t j = 0;
					size_t count = i--;
					while (j < i) {
						char t = buf[i];
						buf[i--] = buf[j];
						buf[j++] = t;
					}
					
					memcpy(dataBuf + pos, buf, count);
					pos += count;
					return;
					
				} else {
					std::cout << "BaaaD ";
				}
			}
			std::cout << "exp:" << exponent << " " << intPart << std::endl;
		}
	}
	
	template <class T>
	char* parse(const POD::ConstBuffer& format, T x) {
		parseInit(format);
		
		parseItem(false);
		print(x);
		parseItem(true);
	}
	
	char* parse(const POD::ConstBuffer& format) {
		parseInit(format);
		return parseItem(true);
	}
	
	void parseInit(const POD::ConstBuffer& format) {
		p = format.ptr;
		last = p;
		end = p + format.len;
		
		dataBuf[0] = dataBuf[Buf_Size-1] = 0;
		pos = 0;
	}
	
	char* parseItem(bool lastPass = false) {
		#define CHECK_END ({if (p == end) { break; }})
		int len = 0;
		
		//std::cout << ">> parseItem("<<lastPass<<") " << p << std::endl;
		do {
			index = 0;
			indexPresent = false;
			alignment = 0;
			alignmentSign = false;
			alignmentPresent = false;
			currentFormat = POD::ConstBuffer(NULL, 0);
			needsProcessing = false;
		
			while (p < end && *p != '{') {
				++p;
			}
			
			// emit fragment
			len += eat(POD::ConstBuffer(last, p-last));
			
			CHECK_END;
			
			// skip '{'
			p++;
			CHECK_END;
			
			// check 'escaped' brace
			if (*p == '{') {
				last = p;
				p++; // skip the '{{'
				continue;
			}
			
			// calculate index
			if (*p >= '0' && *p <= '9') {
				indexPresent = true;
				while (p < end && *p >= '0' && *p <= '9') {
					index = 10*index + (*p++ - '0');
				}
				CHECK_END;
			}
			
			// ok check the alignment
			if (*p == ',') {
				alignmentPresent = true;
				p++;
				CHECK_END;
				
				if (*p == '-' || *p == '+') {
					if (*p=='-') alignmentSign = true;
					p++;
					CHECK_END;
				}
				if (p < end && *p >= '0' && *p <= '9') {
					while (*p >= '0' && *p <= '9') {
						alignment = 10*alignment + (*p++ - '0');
					}
				}
				CHECK_END;
			}
			
			// ok check format string
			const char* formatStart = NULL;
			if (*p == ':') {
				p++;
				CHECK_END;
				
				formatStart = p;
				while (p < end && *p != '}') {
					p++;
				}
			}
			
			if (*p != '}') {
				len += eat(POD::ConstBuffer("{badformat}", 11));
				
			} else {
				if (formatStart) {
					currentFormat = POD::ConstBuffer(formatStart, p-formatStart);
				}
				needsProcessing = true;
				
				if (lastPass) {
					len += eat(POD::ConstBuffer("{noarg}", 7));
				}
				p++;
				CHECK_END;
			}
			
			last = p;
		} while (lastPass);
		return dataBuf;
	}
};

typedef FormatB<> Format;

#endif // GIM_FORMAT_FORMAT_H
