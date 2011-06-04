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
	 */
	template <class Ch>
	class Pattern {
	public:
		typedef POD::TBuffer< Ch > Buf;
		
		Pattern(const Buf& pattern) : pattern(pattern) {
		}
		
		void init(const Buf& _pattern) {
		}
		
		Buf search(const Buf& hayStack) {
			return Buf(0, 0);
		}
		
	private:
		Buf pattern;
	};
	
	
	template <class Ch>
	POD::TBuffer<Ch> pattern(const POD::TBuffer<Ch>& needle, const POD::TBuffer<Ch>& hayStack) {
		Pattern<Ch> pat(needle);
		return pat.search(hayStack);
	}
}

#endif /* GIM_SEARCH_SEARCH_H */
