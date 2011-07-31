/** Format.h
 * String format routines
 * 
 * PileOfCrap (http://code.google.com/p/pileofcrap/)
 * 2011 Michal 'GiM' Spadlinski
 */
#ifndef GIM_FORMAT_FORMAT_H
#define GIM_FORMAT_FORMAT_H

#include <iostream>

#include "../types.h"

template <size_t Buf_Size = 1024>
struct FormatB {
	char dataBuf[Buf_Size];
	int pos;
	typedef char Ch;
	
	FormatB() {
		dataBuf[0] = dataBuf[Buf_Size-1] = 0;
		pos = 0;
	}
	
	size_t eat(const POD::ConstBuffer& buf) {
		// space left
		size_t toWrite = Buf_Size-1 - pos;
		if (buf.len < toWrite) {
			toWrite = buf.len;
		}
		std::cout << "eating... " << buf.len << " " << buf.ptr << std::endl;
		memcpy(dataBuf+pos, buf.ptr, toWrite);
		pos += toWrite;
	}
	
	char* format(const POD::ConstBuffer& format) {
		#define CHECK_END ({if (p == end) { break; }})
		const Ch* p = format.ptr;
		const Ch* end = p + format.len;
		const Ch* last = p;
		int len = 0;
		
		while (true) {
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
			int index = 0;
			bool indexPresent = false;
			if (*p >= '0' && *p <= '9') {
				indexPresent = true;
				while (p < end && *p >= '0' && *p <= '9') {
					index = 10*index + (*p++ - '0');
				}
				CHECK_END;
			}
			
			// ok check the alignment
			int alignment = 0;
			bool alignmentSign = false;
			bool alignmentPresent = false;
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
				if (alignmentSign) alignment *= -1;
				CHECK_END;
			}
			
			// ok check format string
			if (*p == ':') {
				p++;
				CHECK_END;
				
				while (p < end && *p != '}') {
					p++;
				}
			}
			
			if (*p != '}') {
				len += eat(POD::ConstBuffer("{badformat}", 11));
				
			} else {
				p++;
				CHECK_END;
			}
			
			last = p;
		}
		return dataBuf;
	}
};

typedef FormatB<> Format;

#endif // GIM_FORMAT_FORMAT_H
