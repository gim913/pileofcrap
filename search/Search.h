/** Search.h
 * String searching routines
 * 
 * PileOfCrap (http://code.google.com/p/pileofcrap/)
 * 2011 Michal 'GiM' Spadlinski
 */
#ifndef GIM_SEARCH_SEARCH_H
#define GIM_SEARCH_SEARCH_H

#include "../types.h"

#include <iostream>
#include <algorithm>
#include <cstring>

template <class U> struct UnConst {
	typedef U Result;
};
template <class U> struct UnConst<const U> {
	typedef U Result;
};

template <class U> struct UnConst<const U&> {
	typedef U& Result;
};


/*
 * The goal is to have at least three things here
 * - simple pattern search (BM)
 * - pattern search with '.' wildcard
 * - multi pattern search (probably Wu-Manbar)
 * 
 * and maybe also multipattern with '.' wildcard
 */
namespace Search {
	
	/**
	 * Pattern matching using Boyer-Moore
	 * 
	 * Currently badChar is done naively, so it's sensible
	 * to instantiate this tamplate only with char or wchar
	 */
	template <class Ch>
	class Pattern {
	public:
		static const char Template_Arg_Limit[sizeof(Ch) > 2 ? -1 : 0];
		static const size_t badCharLen = 1 << (sizeof(Ch)*8);
		typedef POD::TBuffer<Ch> Buf;
		
		Pattern(const Buf& pattern) : pattern(pattern), badChar(0), goodShift(0) {
			init();
		}
		
		~Pattern() {
			delete [] badChar;
			delete [] goodShift;
		}
		
		void reset(const Buf& _pattern) {
			pattern = _pattern;
			init();
		}
		
		Buf search(const Buf& hayStack) {
			size_t i = 0;
			while (i <= (hayStack.len - pattern.len)) {
				int j;
				for (j = static_cast<int>(pattern.len) - 1; j >= 0; j--) {
					if (pattern.ptr[j] != hayStack.ptr[i+j]) {
						break;
					}
				}
				if (j < 0) {
					return Buf(hayStack.ptr + i, hayStack.len - i);
				}
				i += std::max(1, badChar[hayStack.ptr[i+j]]+ j);
			}
			return Buf(0, 0);
		}
		
		const Buf& getPattern() const {
			return pattern;
		}
		
	protected:
		void init() {
			int m = pattern.len;
			if (goodShift)
				delete [] goodShift;
				
			if (!badChar)
				badChar = new int[badCharLen];
			
			// is there sense to do that?
			if (sizeof(Ch) == 1) {
				memset(badChar, m, badCharLen);
				
			} else {
				for(size_t i = 0; i < badCharLen; ++i) {
					badChar[i] = m;
				}
			}
			
			for (size_t i = 0; i < m; ++i) {
				badChar[pattern.ptr[i]] = -i;
			}
			
			// this will temporarily hold length
			// of a match with a suffix of a pattern
			int* suffixes = new int[m];
			suffixes[m - 1] = m;
			int matchEnd, matchBegin = m -1;
			for (int i = m - 2; i >= 0; --i) {
				if (i > matchBegin && suffixes[i + m - 1 - matchEnd] < i - matchBegin) {
					suffixes[i] = suffixes[i + m - 1 - matchEnd];
					
				} else {
					if (i < matchBegin) {
						matchBegin = i;
					}
					matchEnd = i;
					while (matchBegin >= 0 && pattern.ptr[matchBegin] == pattern.ptr[matchBegin + m - 1 - matchEnd])
						--matchBegin;
					suffixes[i] = matchEnd - matchBegin;
				}
			}
			
			
			goodShift = new int[m];
			for(size_t i = 0; i < m; ++i) {
				goodShift[i] = m;
			}
			
			int j = 0;
			const int last = m - 1;
			for (int i = last; i >= 0; --i) {
				// if the suffix is also the prefix
				if (suffixes[i] == i+1) {
					for (; j < last - i; ++j) { 
						if (goodShift[j] == m) {
							goodShift[j] = last - i;
						}
					}
				}
			}
			
			for (int i = 0; i < m - 1; ++i) {
				goodShift[last - suffixes[i]] = last - i;
			}
			
			delete [] suffixes;
		}
		
	private:
		Buf pattern;
		int* badChar;
		int* goodShift;
	};
	
	
	template <class Ch>
	POD::TBuffer<Ch> pattern(const POD::TBuffer<Ch>& needle, const POD::TBuffer<Ch>& hayStack) {
		Pattern<Ch> pat(needle);
		return pat.search(hayStack);
	}


	/**
	 * naive implementation
	 */
	template <class Ch>
	class PatternDot {
	public:
		typedef typename UnConst<Ch>::Result PlainCh;
		typedef POD::TBuffer<Ch> Buf;

		/// decide if the ctor should take Buf + dot
		/// or some pod, that will wrap them (DotBuffer)
		PatternDot(const Buf& pattern, PlainCh dot) : pattern(pattern), dot(dot) {
		}
		
		~PatternDot() {
		}
		
		void reset(const Buf& _pattern, PlainCh dot) {
			pattern = _pattern;
			dot = dot;
		}
		
		Buf search(const Buf& hayStack) {
			size_t i = 0;
			while (i <= (hayStack.len - pattern.len)) {
				int j;
				for (j = static_cast<int>(pattern.len) - 1; j >= 0; j--) {
					if (pattern.ptr[j] != hayStack.ptr[i+j] && pattern.ptr[j] != dot) {
						break;
					}
				}
				if (j < 0) {
					return Buf(hayStack.ptr + i, hayStack.len - i);
				}
				i += 1;
			}
			return Buf(0, 0);
		}
		
		const Buf& getPattern() const {
			return pattern;
		}
		
	protected:
		void init() {
		}
		
	private:
		Buf pattern;
		PlainCh dot;
	};
	
	
	template <class Ch>
	POD::TBuffer<Ch> patternDot(const POD::TBuffer<Ch>& needle, typename PatternDot<Ch>::PlainCh dot, const POD::TBuffer<Ch>& hayStack) {
		PatternDot<Ch> pat(needle, dot);
		return pat.search(hayStack);
	}
}

#endif /* GIM_SEARCH_SEARCH_H */
