bsdiff/bspatch
==============
bsdiff and bspatch are libraries for building and applying patches to binary
files.

The original algorithm and implementation was developed by Colin Percival.  The
algorithm is detailed in his paper, [Naïve Differences of Executable Code](http://www.daemonology.net/papers/bsdiff.pdf).  For more information, visit his
website at <http://www.daemonology.net/bsdiff/>.

I maintain this project separately from Colin's work, with the goal of making
the core functionality easily embeddable in existing projects.

Contact
-------
[@MatthewEndsley](https://twitter.com/#!/MatthewEndsley)  
<https://github.com/mendsley/bsdiff>

License
-------
Copyright 2003-2005 Colin Percival  
Copyright 2012 Matthew Endsley

This project is governed by the BSD 2-clause license. For details see the file
titled LICENSE in the project root folder.

Overview
--------
There are two separate libraries in the project, bsdiff and bspatch. Each are
self contained in bsdiff.c and bspatch.c The easiest way to integrate is to
simply copy the c file to your source folder and build it.

The overarching goal was to modify the original bsdiff/bspatch code from Colin
and eliminate external dependencies and provide a simple interface to the core
functionality.

I've exposed relevant functions via the `_stream` classes. The only external
dependency not exposed is `memcmp` in `bsdiff`.

This library generates patches that are not compatible with the original bsdiff
tool. The incompatibilities were motivated by the patching needs for the game
AirMech <https://www.carbongames.com> and the following requirements:

* Eliminate/minimize any seek operations when applying patches
* Eliminate any required disk I/O and support embedded streams
* Ability to easily embed the routines as a library instead of an external binary
* Compile+run on all platforms we use to build the game (Windows, Linux, NaCl, OSX)

Compiling
---------
You may use either CMake or Make to build the example.

Examples
--------
Each project has an optional main function that serves as an example for using
the library. Simply defined `BSDIFF_EXECUTABLE` or `BSPATCH_EXECUTABLE` to
enable building the standalone tools.

Reference
---------
### bsdiff

	struct bsdiff_stream
	{
		void* opaque;
		void* (*malloc)(size_t size);
		void  (*free)(void* ptr);
		int   (*write)(struct bsdiff_stream* stream,
					   const void* buffer, int size);
	};

	int bsdiff(const uint8_t* old, int64_t oldsize, const uint8_t* new,
	           int64_t newsize, struct bsdiff_stream* stream);
		

In order to use `bsdiff`, you need to define functions for allocating memory and
writing binary data. This behavior is controlled by the `stream` parameter
passed to to `bsdiff(...)`.

The `opaque` field is never read or modified from within the `bsdiff` function.
The caller can use this field to store custom state data needed for the callback
functions.

The `malloc` and `free` members should point to functions that behave like the
standard `malloc` and `free` C functions.

The `write` function is called by bsdiff to write a block of binary data to the
stream. The return value for `write` should be `0` on success and non-zero if
the callback failed to write all data. In the default example, bzip2 is used to
compress output data.

`bsdiff` returns `0` on success and `-1` on failure.

### bspatch

	struct bspatch_stream
	{
		void* opaque;
		int (*read)(const struct bspatch_stream* stream,
		            void* buffer, int length);
	};

	int bspatch(const uint8_t* old, int64_t oldsize, uint8_t* new,
	            int64_t newsize, struct bspatch_stream* stream);

The `bspatch` function transforms the data for a file using data generated from
`bsdiff`. The caller takes care of loading the old file and allocating space for
new file data.  The `stream` parameter controls the process for reading binary
patch data.

The `opaque` field is never read or modified from within the bspatch function.
The caller can use this field to store custom state data needed for the read
function.

The `read` function is called by `bspatch` to read a block of binary data from
the stream.  The return value for `read` should be `0` on success and non-zero
if the callback failed to read the requested amount of data. In the default
example, bzip2 is used to decompress input data.

`bspatch` returns `0` on success and `-1` on failure. On success, `new` contains
the data for the patched file.
