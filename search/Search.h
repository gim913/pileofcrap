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

// this is kinda crappy, but will do atm
template <size_t> struct DummyDoUnsigned { typedef e_uint Result; };
template <> struct DummyDoUnsigned<1> { typedef e_ubyte Result; };
template <> struct DummyDoUnsigned<2> { typedef e_ushort Result; };

/*
 * The goal is to have at least three things here
 * - simple pattern search (BM)
 * - pattern search with '.' wildcard
 * - multi pattern search
 * 
 * and maybe also multipattern with '.' wildcard
 * 
 * The algorithm were selected to have low memory footprint in common applications
 * 
 * (usually short patterns < 100 bytes,
 *  in case of multipattern, small number of patterns, below 100)
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
		static const size_t Pattern_Stack_Limit = 100;
		
		typedef typename UnConst<Ch>::Result PlainCh;
		// doing static_cast to Index later probably isn't
		// best idea in the world ;p
		typedef typename DummyDoUnsigned<sizeof(Ch)> ::Result Index;
		typedef POD::TBuffer<Ch> Buf;
		
		Pattern(const Buf& pattern) : pattern(pattern), heapShift(0), goodShift(0) {
			init();
		}
		
		~Pattern() {
			delete [] heapShift;
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
				int bc = badChar[ static_cast<Index>(hayStack.ptr[i+j]) ]+j;
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
			
			if (heapShift) {
				delete [] heapShift;
				heapShift = 0;
			}

			for(size_t i = 0; i < Bad_Char_Len; ++i) {
				badChar[i] = 1;
			}
			
			for (size_t i = 0; i < last; ++i) {
				PlainCh j = pattern.ptr[i];
				badChar[ static_cast<Index>(j) ] = -i;
			}
			
			// this will temporarily hold length
			// of a match with a suffix of a pattern
			int* suffixes = 0;
			if (m > Pattern_Stack_Limit) {
				suffixes = new int[m];
				
			} else {
				suffixes = stackShift + Pattern_Stack_Limit;
			}
			
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
			
			if (m > Pattern_Stack_Limit) {
				heapShift = new int[m];
				goodShift = heapShift;
				
			} else {
				goodShift = stackShift;
			}
			
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
			
			if (suffixes != stackShift + Pattern_Stack_Limit) {
				delete [] suffixes;
			}
		}
		
	private:
		Buf pattern;
		int badChar[Bad_Char_Len];
		int* goodShift;
		int* heapShift;
		int stackShift[2*Pattern_Stack_Limit];
	};
	
	
	template <class Ch>
	POD::TBuffer<Ch> pattern(const POD::TBuffer<Ch>& needle, const POD::TBuffer<Ch>& hayStack) {
		Pattern<Ch> pat(needle);
		return pat.search(hayStack);
	}


	/**
	 * naive implementation +
	 * added badChar heuristics from Boyer-Moore
	 */
	template <class Ch>
	class PatternDot {
	public:
		static const size_t Bad_Char_Len = 1 << (sizeof(Ch)*8);
		typedef typename UnConst<Ch>::Result PlainCh;
		// doing static_cast to Index later probably isn't
		// best idea in the world ;p
		typedef typename DummyDoUnsigned<sizeof(Ch)>::Result Index;
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
				i += std::max(1, badChar[ static_cast<Index>(hayStack.ptr[i+j]) ]+j);
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
					Index j = static_cast<Index>(pattern.ptr[i]);
					if (-i < badChar[j])
						badChar[j] = -i;
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
	 * based on "Multi-Pattern String Matching with q-grams"
	 * http://www.cs.hut.fi/~tarhio/papers/jea.pdf
	 * 
	 * this is sensible to be parametrized only with char/e_ubyte
	 * 
	 * if Qgrams_Count is <= 7, qgrams will be stored in
	 * statically allocated memory, otherwise memory for
	 * will qgrams will be dynamically allocated
	 */
	template <class Ch, size_t Qgrams_Count = 7>
	class MultiPattern {
	public:
		typedef POD::TBuffer<Ch> Buf;
		// Number of e_uint's that are needed to keep 256*256 bits
		static const int Qgram_Size = (1 << 16) / (8 * sizeof(e_uint));

#define GRAMSET(cell, idx, ptr) ((cell[idx*Qgram_Size + (((unsigned int)(*(e_ushort*)(ptr)))>>5)]) |= (1 << ((*(e_ushort*)(ptr))&0x1f)))
#define GRAMGET(cell, idx, ptr) ((cell[idx*Qgram_Size + (((unsigned int)(*(e_ushort*)(ptr)))>>5)]) &  (1 << ((*(e_ushort*)(ptr))&0x1f)))

		// remember that members are initialized in order
		// they are in the class and not in the order of initialization list
		MultiPattern(const Buf* patterns, size_t patternsCount) :
				patterns(patterns),
				patternsCount(patternsCount),
				heapTable(Qgrams_Count > 7 ? new e_uint[Qgram_Size * Qgrams_Count] : NULL),
				gramTable(Qgrams_Count > 7 ? heapTable : stackTable)
		{
			init();
		}
		
		~MultiPattern() {
			if (heapTable) {
				delete [] heapTable;
			}
		}
		
		Buf search(const Buf& hayStack) {
			size_t i = 0;
			if (!initialized)
				return Buf(0, 0);
			//std::cout << "matching ;p " << std::endl;
			while (i <= (hayStack.len - minPattern)) {
				int j;
				for (j = minPattern - 1 -1; j >= 0; --j) {
					//std::cout << "checking: " << j << " " << hayStack.ptr[i+j] << hayStack.ptr[i+j+1] << " " << (hayStack.ptr +i+j) << std::endl;
					if (!GRAMGET(gramTable, j, hayStack.ptr +i +j)) {
						i += j;
						break;
					}
				}
				if (j <= 0) {
					// ATM I don't need it now optimized for large number of
					// patterns so doing this naively is fine
					Ch elem = hayStack.ptr[i];
					for (size_t k = 0; k < patternsCount; k++) {
						if (patterns[k].ptr[0] == elem && patterns[k].len <= hayStack.len - i) {
							if (!memcmp(hayStack.ptr + i, patterns[k].ptr, patterns[k].len)) {
								return Buf(hayStack.ptr + i, hayStack.len - i);
							}
						}
					}
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
			minPattern = Qgrams_Count + 1;
			// find shortest pattern
			for (size_t i = 0; i < patternsCount; ++i) {
				if (patterns[i].len < minPattern)
					minPattern = patterns[i].len;
			}
			initialized = false;
			if (minPattern <= 1) {
				return;
			}

			memset (gramTable, 0, sizeof(gramTable));
			
			for (size_t i = 0; i < patternsCount; ++i) {
				for (size_t j = 0; j < minPattern - 1; ++j) {
					for (size_t k = j; k < minPattern -1; ++k) {
						// std::cout << "adding : " << patterns[i].ptr[j] << patterns[i].ptr[j+1] << " on pos: " << k << std::endl;
						GRAMSET(gramTable, k, patterns[i].ptr + j);
					}
				}
			}
			initialized = true;
		}
		
	private:
		const Buf* patterns;
		size_t patternsCount;
		size_t minPattern;
		// 2048 * 4 * Q (8192 * Q=7 = 56k)
		e_uint *heapTable;
		e_uint stackTable[Qgrams_Count <= 7 ? (Qgrams_Count * Qgram_Size) : 1];
		e_uint *gramTable;
		bool initialized;
#undef GRAMGET
#undef GRAMSET
	};
	
	template <class Ch>
	POD::TBuffer<Ch> multiPattern(const POD::TBuffer<Ch>* patterns, size_t patternsCount, const POD::TBuffer<Ch>& hayStack) {
		MultiPattern<Ch> mp(patterns, patternsCount);
		return mp.search(hayStack);
	}

	/*
	 * I want to base this one on Shift-Or + Qgrams
	 * 
	 */
	template <class Ch, size_t Qgrams_Count = 7>
	class MultiPatternDot {
		typedef POD::TBuffer<Ch> Buf;
		typedef typename UnConst<Ch>::Result PlainCh;
		// Number of e_uint's that are needed to keep 256*256 bits
		static const int Qgram_Size = (1 << 16) / (8 * sizeof(e_uint));

#define GRAMSET(cell, idx, ptr) ((cell[idx*Qgram_Size + (((unsigned int)(*(e_ushort*)(ptr)))>>5)]) |= (1 << ((*(e_ushort*)(ptr))&0x1f)))
#define GRAMGET(cell, idx, ptr) ((cell[idx*Qgram_Size + (((unsigned int)(*(e_ushort*)(ptr)))>>5)]) &  (1 << ((*(e_ushort*)(ptr))&0x1f)))
		
	public:
		// maybe it's not the wisest idea to get only one dot for all patterns
		// but in most cases it should do the work
		MultiPatternDot(const Buf* patterns, size_t patternsCount, PlainCh dot) :
				patterns(patterns),
				patternsCount(patternsCount),
				dot(dot),
				gramTable(Qgrams_Count > 7 ? 0 : stackTable)
		{
			//init();
		}
		
		~MultiPatternDot() {
		}
		
		Buf search(const Buf& hayStack) {
			if (!initialized)
				return Buf(0, 0);
			
			return Buf(0, 0);
		}
		
	protected:
		void init() {
			minPattern = Qgrams_Count + 1;
			
			// find shortest pattern
			for (size_t i = 0; i < patternsCount; ++i) {
				if (patterns[i].len < minPattern)
					minPattern = patterns[i].len;
			}
			initialized = false;
			if (minPattern <= 1) {
				return;
			}

			memset (gramTable, 0, sizeof(gramTable));
			for (size_t i = 0; i < patternsCount; ++i) {
				for (size_t j = 0; j < minPattern - 1; ++j) {
					int state = 0;
					state += !!(patterns[i].ptr[j+1] == dot);
					state += (!!(patterns[i].ptr[j] == dot)) << 1;
					
					switch (state) {
						case 3:
							// if there are two dots one after another
							// we need to fill all the bits in gram table
							// for current index ''j''
							for (size_t k = 0; k < Qgram_Size; ++k)
								gramTable[j*Qgram_Size + k] = -1;
							break;
						
						// those two are endian-dependent...
						case 2:
							break;
							
						case 1:
							break;
							
						default:
							GRAMSET(gramTable, j, patterns[i].ptr + j);
							break;
					}
				}
			}
			initialized = true;
		}
		
	private:
		const Buf* patterns;
		size_t patternsCount;
		PlainCh dot;
		size_t minPattern;
		
		e_uint stackTable[Qgrams_Count <= 7 ? (Qgrams_Count * Qgram_Size) : 1];
		e_uint *gramTable;
		bool initialized;

#undef GRAMSET
#undef GRAMGET
	};
}

#endif /* GIM_SEARCH_SEARCH_H */
