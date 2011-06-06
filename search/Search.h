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
		
		// I was thinking about doing Turbo-BM here, but BM
		// is easier to understand, so I hope it'll be easier
		// for someone else to adjust this to ones needs
		Buf search(const Buf& hayStack) {
			const int last = static_cast<int>(pattern.len) - 1;
			size_t i = 0;
			while (i <= (hayStack.len - pattern.len)) {
				int j = last;
				while (j>= 0 && pattern.ptr[j] == hayStack.ptr[i+j]) {
					j--;
				}
				if (j < 0) {
					return Buf(hayStack.ptr + i, hayStack.len - i);
				}
				int gs = goodShift[j];
				int bc = badChar[hayStack.ptr[i+j]]+j;
				i += std::max(gs, bc);
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
	 * naive implementation +
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
	
	/**
	 * BMH+qG
	 * 
	 * shortest pattern should be < 100
	 * this is sensible to be parametrized only with char/e_ubyte
	 */
	template <class Ch>
	class MultiPattern {
#define GRAMSET(cell,bt) ((cell[((unsigned int)bt)>>5]) |= (1 << (bt&0x1f)))
#define GRAMGET(cell,bt) ((cell[((unsigned int)bt)>>5]) &  (1 << (bt&0x1f)))
	public:
		typedef POD::TBuffer<Ch> Buf;
		// memory needed 100*8*4  for gram table 100*32
		static const size_t Max_Shortest_Pattern = 100;

		MultiPattern(const Buf* patterns, size_t patternsCount) : patterns(patterns), patternsCount(patternsCount) {
			init();
		}
		
		~MultiPattern() {
		}
		
		Buf search(const Buf& hayStack) {
			size_t i = 0;
			if (!initialized)
				return Buf(0, 0);
			std::cout << "matching ;p " << std::endl;
			while (i <= (hayStack.len - minPattern)) {
				int j;
				for (j = minPattern - 1; j >= 0; --j) {
					std::cout << "checking: " << j << " " << hayStack.ptr[i+j] << std::endl;
					if (!GRAMGET(gramTable[j], hayStack.ptr[i+j])) {
						i += j+1;
						break;
					}
				}
				if (j < 0) {
					std::cout << " check match at pos: " << i << std::endl;
					return Buf(hayStack.ptr + i, hayStack.len - i);
				}
				++i;
			}
			return Buf(0, 0);
		}
		
		const Buf& getPattern() const {
			return pattern;
		}
		
	protected:
		void init() {
			minPattern = Max_Shortest_Pattern+1;
			// find shortest pattern
			for (size_t i = 0; i < patternsCount; ++i) {
				if (patterns[i].len < minPattern)
					minPattern = patterns[i].len;
			}
			initialized = false;
			if (minPattern == Max_Shortest_Pattern+1)
				return;

			memset (gramTable, 0, sizeof(gramTable));
			
			for (size_t i = 0; i < patternsCount; ++i) {
				for (size_t j = 0; j < minPattern; ++j) {
					std::cout << "adding: " << patterns[i].ptr[j] << std::endl;
					for (size_t k = j; k < minPattern; ++k) {
						GRAMSET(gramTable[k], patterns[i].ptr[j]);
					}
				}
			}
			
			for (size_t j = 0; j < minPattern; ++j) {
				for (size_t k = 0; k < 256; ++k) {
					int t = GRAMGET(gramTable[j], k);
					std::cout << (char)(t?k:'_') << "";
				}
				std::cout << std::endl;
			}
			initialized = true;
		}
		
	private:
		const Buf* patterns;
		size_t patternsCount;
		size_t minPattern;
		e_uint gramTable[Max_Shortest_Pattern][8];
		bool initialized;
#undef GRAMGET
#undef GRAMSET
	};
	
	template <class Ch>
	POD::TBuffer<Ch> multiPattern(const POD::TBuffer<Ch>* patterns, size_t patternsCount, const POD::TBuffer<Ch>& hayStack) {
		MultiPattern<Ch> mp(patterns, patternsCount);
		return mp.search(hayStack);
	}
}

#endif /* GIM_SEARCH_SEARCH_H */
