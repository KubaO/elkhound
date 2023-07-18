smbase: A Utility Library
=========================

Introduction
============

"smbase" stands for Scott McPeak's Base Library (sorry, naming things is not my specialty). It's a bunch of utility modules I use in virtually all of my projects. The entire library is in the public domain.

*Functionality that overlaps the C++ Standard Library is being removed to ease maintenance.*

There is some overlap in functionality between smbase and the C++ Standard Library. Partly this is because smbase predates the standard library, and partly this is because that library has aspects to its design that I disagree with (for example, I think it is excessively templatized, given flaws in the C++ template mechanism). However, the intent is that client code can use smbase and the standard library at the same time.

smbase has developed organically, in response to specific needs. While each module individually has been reasonably carefully designed, the library as a whole has not. Consequently, the modules to not always orthogonally cover a given design space, and some of the modules are now considered obsolete (marked below as such).

Some of the links below refer to generated documentation files. If you are reading this from your local filesystem, you may have to say "make gendoc" (after "./configure") to get them.

Build Instructions
==================

SMBase is a static library. The build is configured by cmake.

Modules
=======

The following sections list all the smbase modules, grouped by functionality.

Linked Lists
------------

Linked lists are sequences of objects with O(1) insertion at the front and iterators for traversal. Most also have _mutators_ for traversing and modifying.

* [voidlist.h](voidlist.h), [voidlist.cc](voidlist.cc): The core of the linked list implementation used by [objlist.h](objlist.h).

There are a couple of variants that support O(1) appending.

* [vdtllist.h](vdtllist.h), [vdtllist.cc](vdtllist.cc): VoidTailList, the core of a linked list implementation which maintains a pointer to the last node for O(1) appends. Used by [astlist.h](astlist.h) and [taillist.h](taillist.h).
* [astlist.h](astlist.h): ASTList, a list class for use in abstract syntax trees.


Algorithms
----------

* [algo.h](algo.h): Some useful algorithms, in the style of the standard `<algorithm>` header.

  - `bool contains(container, value)` - a less verbose alternative to `std::find` when only a yes/no answer is needed
  - `int compareSortedSLists(a, b, diff, [extra])` - compares two sorted lists using a provided int-returning comparison function, the returned value has the same meaning as in `strcmp` and `memcmp`.
  - `void sortSList(container, diff, [extra])` - sorts the given container using a provided int-returning comparison function
  - `V* getPointerFromMap(map, key)` - returns the pointer-typed value for a given key, or nullptr
  - `V* getPointerFromSet(set, val)` - returns val if the set contains it, or nullptr

  The `S` in `SList` means "serf" - a non-owning pointers. `SList` is a general term that refers to a vector of pointers, a deque of pointers, etc.


Container Adapters
------------------

* [stack.h](stack.h): An equivalent of C++ standard `<stack>` adapter, with a bit more functionality.
  - Iteration in stack order (top-to-bottom) and reverse order (bottom-to-top) is supported
  - `clear()` is available to remove all items from the stack
  - `pusher()` provides a push-iterator that pushes items on top of the stack, in the spirit of `std::back_inserter`
  - There's less verbosity in the template arguments: the container type is a free template; scalar value types get `std::vector` by default as the backing storage, whereas non-scalar types get `std::deque`. For safety it's presumed that the non-scalar values may be referenced outside of the stack through pointers, so a non-pointer-preserving container like vector can't be used.


Arrays
------

Arrays are sequences of objects with O(1) random access and replacement.

The main array header, [array.h](array.h), contains several array classes. GrowArray supports bounds checking and a method to grow the array. ArrayStack supports a distinction between the _length_ of the sequence and the _size_ of the array allocated to store it, and grows the latter automatically.

* [array.h](array.h): Several array-like template classes, including growable arrays.

Arrays of Bits
--------------

Arrays of bits are handled specially, because they are implemented by storing multiple bits per byte.

* [bit2d.h](bit2d.h), [bit2d.cc](bit2d.cc): Two-dimensional array of bits.


Strings
-------

Strings are sequences of characters.

* [str.h](str.h), [str.cpp](str.cpp): The string class itself. Using the string class instead of char* makes handling strings as convenent as manipulating fundamental types like int or float. See also [string.txt](string.txt).

* [strtokp.h](strtokp.h), [strtokp.cpp](strtokp.cpp): StrtokParse, a class that parses a string similar to how strtok() works, but provides a more convenient (and thread-safe) interface. Similar to Java's StringTokenizer.

* [strutil.h](strutil.h), [strutil.cc](strutil.cc): A set of generic string utilities, including replace(), translate(), trimWhitespace(), encodeWithEscapes(), etc.

* [strtable.h](strtable.h): A table that interns the strings.


System Utilities
----------------

The following modules provide access to or wrappers around various low-level system services.

* [autofile.h](autofile.h), [autofile.cc](autofile.cc): AutoFILE, a simple wrapper around FILE* to open it or throw an exception, and automatically close it.

* [cycles.h](cycles.h) [cycles.c](cycles.c): Report total number of processor cycles since the machine was turned on. Uses the RDTSC instruction on x86.

* [missing.h](missing.h), [missing.cpp](missing.cpp): Implementations of a few C library functions that are not present on all platforms.

* [mypopen.h](mypopen.h), [mypopen.c](mypopen.c): Open a process, yielding two pipes: one for writing, one for reading.

* [syserr.h](syserr.h), [syserr.cpp](syserr.cpp): Intended to be a portable encapsulation of system-dependent error facilities like UNIX's errno and Win32's GetLastError(). It's not very complete right now.


Portability
-----------

These modules help insulate client code from the details of the system it is running on.

* [nonport.h](nonport.h), [nonport.cpp](nonport.cpp): A library of utility functions whose implementation is system-specific. Generally, I try to encapsulate all system depenencies as functions defined in nonport.

* [macros.h](macros.h): Defines bunch of useful macros. Does not include any other headers.

Allocation
----------

These modules provide additional control over the allocator.

* [ckheap.h](ckheap.h): Interface to check heap integrity. The underlying malloc implementation must support these entry points for it to work. I've extended Doug Lea's malloc ([malloc.c](malloc.c)) to do so.

* [objpool.h](objpool.h): ObjPool, a custom allocator for fixed-size objects with embedded 'next' links.


Exceptions
----------

These modules define or throw exceptions.

* [exc.h](exc.h), [exc.cpp](exc.cpp): Various exception classes. The intent is derive everything from xBase, so a program can catch this one exception type in main() and be assured no exception will propagate out of the program (or any other unit of granularity you want).

* [xassert.h](xassert.h): xassert is an assert()-like macro that throws an exception when it fails, instead of calling abort().

Serialization
-------------

The "flatten" serialization scheme is intended to allow sets of objects to read and write themselves to files.

* [bflatten.h](bflatten.h), [bflatten.cc](bflatten.cc): Implementation of the Flatten interface ([flatten.h](flatten.h)) for reading/writing binary files.

* [flatten.h](flatten.h), [flatten.cc](flatten.cc): Generic interface for serializing in-memory data structures to files. Similar to Java's Serializable, but independently conceived, and has superior version control facilities.

Compiler/Translator Support
---------------------------

smbase has a number of modules that are of use to programs that read and/or write source code.

* [hashline.h](hashline.h), [hashline.cc](hashline.cc): HashLineMap, a mechanism for keeping track of #line directives in C source files. Provides efficient queries with respect to a set of such directives.

* [srcloc.h](srcloc.h), [srcloc.cc](srcloc.cc): This module maintains a one-word data type called SourceLoc. SourceLoc is a location within some file, e.g. line/col or character offset information. SourceLoc also encodes _which_ file it refers to. This type is very useful for language processors (like compilers) because it efficiently encodes location formation. Decoding this into human-readable form is slower than incrementally updating it, but decoding is made somewhat efficient with some appropriate index structures.

* [warn.h](warn.h), [warn.cpp](warn.cpp): Intended to provide a general interface for user-level warnings; the design never really worked well.


Testing and Debugging
---------------------

* [breaker.h](breaker.h) [breaker.cpp](breaker.cpp): Function for putting a breakpoint in, to get debugger control just before an exception is thrown.

* [test.h](test.h): A few test-harness macros.

* [trace.h](trace.h), [trace.cc](trace.cc): Module for recording and querying a set of debug tracing flags. It is documented in [trace.md](trace.md).

* [trdelete.h](trdelete.h), [trdelete.cc](trdelete.cc): An operator delete which overwrites the deallocated memory with 0xAA before deallocating it.

Miscellaneous
-------------

* [crc.h](crc.h) [crc.cpp](crc.cpp): 32-bit cyclic redundancy check.

* [gprintf.h](gprintf.h), [gprintf.c](gprintf.c): General printf; calls a function to emit each piece.

* [point.h](point.h), [point.cc](point.cc): Point, a pair of integers.

Test Drivers
------------

Test drivers. Below are the modules that are purely test drivers for other modules. They're separated out from the list above to avoid the clutter.

* [testarray.cc](testarray.cc): Test driver for [array.h](array.h).
* [testcout.cc](testcout.cc): This is a little test program for use by [configure.pl](configure.pl).
* [tobjlist.cc](tobjlist.cc): Test driver for [objlist.h](objlist.h).
* [tobjpool.cc](tobjpool.cc) Test driver for [objpool.h](objpool.h).

Utility Scripts
---------------

* [run-flex.pl](run-flex.pl): Perl script to run flex and massage its output for portability.

* [sm_config.pm](sm_config.pm): This is a Perl module, intended to be used by configure scripts. It is mostly a library of useful routines, but also reads and writes some of the main script's global variables.

Module Dependencies
===================

The [scan-depends.pl](scan-depends.pl) script is capable of generating a [module dependency description](gendoc/dependencies.dot) in the [Dot](http://www.research.att.com/sw/tools/graphviz/) format. Not all the modules appear; I try to show the most important modules, and try to avoid making Dot do weird things. Below is its output.

![Module Dependencies](gendoc/dependencies.png)  
There's also a [Postscript version](gendoc/dependencies.ps).

