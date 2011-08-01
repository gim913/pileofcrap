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
	bool alignmentPresent;
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
		std::cout << "eating... " << buf.len << " " << buf.ptr << std::endl;
		memcpy(dataBuf+pos, buf.ptr, toWrite);
		pos += toWrite;
	}

	char* parse(const POD::ConstBuffer& format) {
		p = format.ptr;
		last = p;
		end = p + format.len;
		
		dataBuf[0] = dataBuf[Buf_Size-1] = 0;
		pos = 0;
		
		return parseItem(true);
	}
	
	char* parseItem(bool lastPass = false) {
		#define CHECK_END ({if (p == end) { break; }})
		int len = 0;
		
		while(true) {
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
				if (alignmentSign) alignment *= -1;
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
				if (formatStart)
					currentFormat = POD::ConstBuffer(formatStart, p-formatStart);
				needsProcessing = true;
				
				if (lastPass) {
					len += eat(POD::ConstBuffer("{noarg}", 7));
				}
				
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
