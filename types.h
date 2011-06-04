/**
 * Some basic type wrappers
 * 
 * Michal 'GiM' Spadlinski
 */
#ifndef GIM_TYPES_H
#define GIM_TYPES_H

#if defined(_MSC_VER)

typedef signed __int8       e_byte;
typedef unsigned __int8     e_ubyte;
typedef signed __int16      e_short;
typedef unsigned __int16    e_ushort;
typedef signed __int32      e_int;
typedef unsigned __int32    e_uint;
typedef signed __int64      e_long;
typedef unsigned __int64    e_ulong;

#else /* !_MSC_VER */

/* size_t */
#  include <stddef.h>
#  include <stdint.h>

typedef int8_t              e_byte;
typedef uint8_t             e_ubyte;
typedef int16_t             e_short;
typedef uint16_t            e_ushort;
typedef int32_t             e_int;
typedef uint32_t            e_uint;
typedef int64_t             e_long;
typedef uint64_t            e_ulong;

#endif /* _MSC_VER */

namespace POD {
	template <class T>
	struct TBuffer
	{
		TBuffer(T* ptr, size_t len) : ptr(ptr), len(len) {}
		
		template <size_t S>
		TBuffer(T (&buf)[S]) : ptr(buf), len(S*sizeof(T)) {}
		
		T*     ptr;
		size_t len;
	};
	
	typedef TBuffer<const char> ConstBuffer;
	typedef TBuffer<char> Buffer;
}
#endif