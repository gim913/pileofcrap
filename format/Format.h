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
	
	size_t eat(const POD::Buffer& buf) {
		// space left
		size_t toWrite = Buf_Size-1 - pos;
		if (buf.len < toWrite) {
			toWrite = buf.len;
		}
		std::cout << "eating... " << buf.len << " " << buf.ptr << std::endl;
		memcpy(dataBuf+pos, buf.ptr, toWrite);
		pos += toWrite;
	}
	
	char* format(const POD::Buffer& format) {
		Ch* p = format.ptr;
		Ch* end = p + format.len;
		Ch* last = p;
		int len = 0;
		
		while (true) {
			while (p < end && *p != '{') {
				++p;
			}
			
			// emit fragment
			len += eat(POD::Buffer(last, p-last));
			
			if (p == end) { break; }
			
			// skip '{'
			
			p++;
			// check 'escaped' brace
			if (*p == '{') {
				last = p;
				p++; // skip the '{{'
				continue;
			}
			
			// calculate index
			int index = 0;
			bool hasIndex = false;
			if (*p >= '0' && *p <= '9') {
				hasIndex = true;
				while (*p >= '0' && *p <= '9') {
					index = 10*index + (*p - '0');
				}
			}
			
			// ok check the alignment
			if (*p == ',') {
				p++;
			}
			
			// ok check format string
			if (*p == ':') {
				p++;
			}
			
			if (*p == '}') {
				p++;
				last = p;
			}
		}
		return dataBuf;
	}
};

typedef FormatB<> Format;

#endif // GIM_FORMAT_FORMAT_H
