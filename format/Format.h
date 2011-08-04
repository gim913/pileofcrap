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
	
	public:
	FormatB() : currentFormat(0, 0) {}
	
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
	
	int parseFormat(FormatSpecifier& ret, int defaultMode = 10) {
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
				
				// nasty, but seems to work like a charm
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

	void print(char x)     {
		//std::cout<< "char   " << std::endl;
		FormatSpecifier fs;
		if (parseFormat(fs, 0))
			return;
		realPrint<CharPrinter>(static_cast<e_ulong>(x), fs, x < 0 ? 1 : 0);
	}
	
	void print(bool x)     {
		//std::cout<< "bool   " << std::endl;
		FormatSpecifier fs;
		if (parseFormat(fs, 0))
			return;
		realPrint<BoolPrinter>(static_cast<e_ulong>(x), fs, x < 0 ? 1 : 0);
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
		if (parseFormat(fs))
			return;
		realPrint<IntPrinter>(x, fs, hasSign);
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
	
	template <class TypePrinter>
	void realPrint(e_ulong x, const FormatSpecifier& readOnlyFs, int hasSign) {
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
