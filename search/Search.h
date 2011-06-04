/** Search.h
 * String searching routines
 * 
 * PileOfCrap (http://code.google.com/p/pileofcrap/)
 * 2011 Michal 'GiM' Spadlinski
 */
#ifndef GIM_SEARCH_SEARCH_H
#define GIM_SEARCH_SEARCH_H

#include "../types.h"

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
	 * This will implement Boyer-Moore
	 * 
	 * Currently badChar is done naively,
	 * so it's sensible to instantiate this tamplate
	 * only with char or wchar
	 */
	template <class Ch>
	class Pattern {
	public:
		static const char Template_Arg_Limit[sizeof(Ch) > 2 ? -1 : 0];
		typedef POD::TBuffer<Ch> Buf;
		
		Pattern(const Buf& pattern) : pattern(pattern), badChar(0), goodShift(0) {
		}
		
		~Pattern() {
		}
		
		void reset(const Buf& _pattern) {
			pattern = _pattern;
		}
		
		Buf search(const Buf& hayStack) {
			return Buf(0, 0);
		}
		
		const Buf& getPattern() const {
			return pattern;
		}
		
	protected:
		void init() {
			if (goodShift)
				delete [] goodShift;
			if (!badChar)
				badChar = new Ch[badCharLen];
			
			if (sizeof(Ch) == 1) {
				memset(badChar, pattern.len, badCharLen);
				
			} else {
				for(size_t i = 0; i < badCharLen; ++i) {
					badChar[i] = pattern.len;
				}
			}
			
			for (size_t i = 0; i < pattern.len; ++i) {
				badChar[pattern.ptr[i]] = pattern.len - i - 1;
			}
		}
		
	private:
		Buf pattern;
		static const size_t badCharLen = 1 << (sizeof(Ch)*8);
		Ch* badChar;
		Ch* goodShift;
	};
	
	
	template <class Ch>
	POD::TBuffer<Ch> pattern(const POD::TBuffer<Ch>& needle, const POD::TBuffer<Ch>& hayStack) {
		Pattern<Ch> pat(needle);
		return pat.search(hayStack);
	}
}

#endif /* GIM_SEARCH_SEARCH_H */
