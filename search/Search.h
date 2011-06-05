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
		static const size_t Bad_Char_Len = 1 << (sizeof(Ch)*8);
		typedef POD::TBuffer<Ch> Buf;
		
		Pattern(const Buf& pattern) : pattern(pattern), goodShift(0) {
			init();
		}
		
		~Pattern() {
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
				i += std::max(goodShift[j], badChar[hayStack.ptr[i+j]]+j);
			}
			return Buf(0, 0);
		}
		
		const Buf& getPattern() const {
			return pattern;
		}
		
	protected:
		void init() {
			const int m = pattern.len;
			const int last = m - 1;
			
			if (goodShift)
				delete [] goodShift;

			for(size_t i = 0; i < Bad_Char_Len; ++i) {
				badChar[i] = 1;
			}
			
			for (size_t i = 0; i < last; ++i) {
				badChar[pattern.ptr[i]] = -i;
			}
			
			// this will temporarily hold length
			// of a match with a suffix of a pattern
			int* suffixes = new int[m];
			suffixes[m - 1] = m;
			int matchEnd, matchBegin = last;
			for (int i = m - 2; i >= 0; --i) {
				if (i > matchBegin && suffixes[i + last - matchEnd] < i - matchBegin) {
					suffixes[i] = suffixes[i + last - matchEnd];
					
				} else {
					if (i < matchBegin) {
						matchBegin = i;
					}
					matchEnd = i;
					while (matchBegin >= 0 && pattern.ptr[matchBegin] == pattern.ptr[last + matchBegin - matchEnd]) {
						--matchBegin;
					}
					suffixes[i] = matchEnd - matchBegin;
				}
			}
			
			
			goodShift = new int[m];
			for(size_t i = 0; i < m; ++i) {
				goodShift[i] = m;
			}
			
			int j = 0;
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
			
			// '< last', cause suffixes[last] is always m
			for (int i = 0; i < last; ++i) {
				goodShift[last - suffixes[i]] = last - i;
			}
			
			delete [] suffixes;
		}
		
	private:
		Buf pattern;
		int badChar[Bad_Char_Len];
		int* goodShift;
	};
	
	
	template <class Ch>
	POD::TBuffer<Ch> pattern(const POD::TBuffer<Ch>& needle, const POD::TBuffer<Ch>& hayStack) {
		Pattern<Ch> pat(needle);
		return pat.search(hayStack);
	}


	/**
	 * naive implementation
	 * added badChar heuristics from B-M
	 */
	template <class Ch>
	class PatternDot {
	public:
		static const size_t Bad_Char_Len = 1 << (sizeof(Ch)*8);
		typedef typename UnConst<Ch>::Result PlainCh;
		typedef POD::TBuffer<Ch> Buf;

		/// decide if the ctor should take Buf + dot
		/// or some pod, that will wrap them (DotBuffer)
		PatternDot(const Buf& pattern, PlainCh dot) : pattern(pattern), dot(dot) {
			init();
		}
		
		~PatternDot() {
		}
		
		void reset(const Buf& _pattern, PlainCh dot) {
			pattern = _pattern;
			dot = dot;
			init();
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
				i += std::max(1, badChar[hayStack.ptr[i+j]]+j);
			}
			return Buf(0, 0);
		}
		
		const Buf& getPattern() const {
			return pattern;
		}
		
	protected:
		void init() {
			const int m = static_cast<int>(pattern.len);
			const int last = m-1;
			int lastDotPos = last;
			
			while(lastDotPos > 0 && pattern.ptr[lastDotPos] != dot) {
				--lastDotPos;
			}
			
			if (!lastDotPos)
				lastDotPos = -1;

			for(size_t i = 0; i < Bad_Char_Len; ++i) {
				badChar[i] = -lastDotPos;
			}
			
			// i must be int due to the comparison below
			for (int i = 0; i < last; ++i) {
				if (pattern.ptr[i] != dot) {
					if (-i < badChar[pattern.ptr[i]])
						badChar[pattern.ptr[i]] = -i;
				}
			}
			
//			for(size_t i = 0; i < Bad_Char_Len; ++i) {
//				std::cout << badChar[i] << " ";
//				if (!((i+1)%16)) std::cout << std::endl;
//			}
		}
		
	private:
		Buf pattern;
		int badChar[Bad_Char_Len];
		PlainCh dot;
	};
	
	
	template <class Ch>
	POD::TBuffer<Ch> patternDot(const POD::TBuffer<Ch>& needle, typename PatternDot<Ch>::PlainCh dot, const POD::TBuffer<Ch>& hayStack) {
		PatternDot<Ch> pat(needle, dot);
		return pat.search(hayStack);
	}
}

#endif /* GIM_SEARCH_SEARCH_H */
