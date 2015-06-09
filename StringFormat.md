# This is some initial doc, so stuff is in kinda random order #

# Introduction #

format/Format.h contains initial implementation of C++ version of .Net-like [String.Format](http://msdn.microsoft.com/en-us/library/system.string.format.aspx)

# Details #

There are some constraints, that I want to be fullfiled, here's excerpt
from the comments:
```cpp

//   1 I want it to be C++ (which kinda sux cause using C++0x features, would make it easier)
//   2 no exceptions
//   3 I don't care about resulting size (of executable ;p)
//   4 there won't be support for time/date formatting
//   5 similarily there won't be support for 'currency', cause I don't want
//      and don't need to play with locale
// I'm not quite sure yet if I will actually finish this
```

and additionally: I don't want to use va\_arg stuff, which is kinda connected to point 3

There will also be some important differences:
  * no support for indexing variables,
  * probably no support for custom formatting
  * ...TBC

Formatted string is returned via some local variable,
so if you'd like to use this in multi-threded app you should
probably have separate formatter for each thread

Default mode for integer types is decimal, default mode for pointers is hexadecimal


