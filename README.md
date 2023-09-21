# Cpp98_Library
A collection of tools which represent a large chunk of the modern C++ standard library, written in ISO C++98, and compatible with all future C++ standards. While I cannot recreate features which rely on core language features, much of the STL is implementable in C++98; though my general recommendation is to use an official stdlib distribution over this if you are on a modern C++ standard which has access to those features.
Created because I spend a fair bit of time maintaining and adapting legacy code which for various reasons cannot be updated to modern C++.

This is a header-only library. The include directory should be added to your project's include path, with the cpp98 subdirectory containing the headers which should see the outside world and and the bits subdirectory following the gcc pattern of being internal files for the library.

The code is designed to emulate the standard library as closely as is reasonable in this language standard. For example, if you want to use a std::array-style array, you would `#include "cpp98/array.h"` and construct it via `dp::array<foo,size> myarray = {....};`

All tools in this repo exist in `namespace dp`

## List of Features

A writeup of all the features can be found on [the project's wiki](https://github.com/DryPerspective/Cpp98_Library/wiki), but a simple list of all headers included can be found here.

* algorithm
* array
* expected 
* iterator
* numeric
* optional 
* ratio
* rc_base 
* rc_ptr 
* scoped_ptr
* static_assert 
* string_view
* typeindex
* type_traits 
* utility

As a side note, this library also provides an entirely separate header which contains string_view types for the Borland `UnicodeString` and `AnsiString` types.



