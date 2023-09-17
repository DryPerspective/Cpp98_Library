# Cpp98_Tools
A small collection of tools which are compatible with the C++98 standard as well as all future standards. Most of these tools accomplish something which was added in a future standard (e.g. Optional is an analogue of `std::optional`), and I will recommend that if you are using them and you update to a lanugage standard which supports that feature you swap out the tool accordingly - I have designed the interfaces of these tools to make it as painless a process as possible.
Created because I spend a fair bit of time maintaining and adapting legacy code which for various reasons cannot be updated to modern C++.

All tools in this repo exist in `namespace dp`

## List of Features
* **algorithm** - Standard library algorithms added in C++11 and onwards, which would normally live in the `<algorithm>` header.
* **array** - An analogue of `std::array`, sharing the same functionality and interface. 
* **numeric** - A light implementation of *some* of the functions added to the `<numeric>` header from C++11 and up. Does not include algorithms added for parallelisation purposes as C++98 has no standard concurrency primitives.
* **optional** - An an analogue of `std::optional` - a class which optionally contains a value, is in a well-defined state in all cases, and which does not construct a held value until it is required to hold one.
* **range_functions** - Contains analogues of the C++11-C++17 "range access" functions : `std::begin()`; `std::end()`; c-, r-, and cr- versions of the same; `std::size()`, `std::ssize()`, `std::empty()`, and `std::data()`. This also adds some generic iterator types to complement use of `dp::begin()` etc in a generic way; which can be safely replaced by `auto` on a future standard.
* **ratio** - A recreation of the standard `<ratio>` header, for compile time rational arithmetic.
* **rc_base** - A general-purpose base class for creating reference counted, COW classes. Intended for use in rc_ptr as well as other classes which seek to add ref-counting, COW behaviour.
* **rc_ptr** - A reference-counted copy-on-write smart pointer, an analogue of `std::shared_ptr`. Supports single-object and array versions, as well as a `make_rc_ptr` function (analogue of `std::make_shared`), for single values only. In the array case, I'm still working on finding the cleanest solution to ensure overload resolution picks the correct function in even the most pathological cases.
* **scoped_ptr** - A unique-owning, scope-local smart pointer. An analogue of `std::unique_ptr`, but of course without move semantics. Supports single-object and array versions as well as a `make_scoped` function (analogue of `std::make_unique`) for both single value and array types.
* **static_assert** - A basic but effective replacement for the `static_assert` keyword. As we are replacing a keyword with a class (and to avoid naming collisions with the keyword), the syntax is a little different. An expression `dp::static_assert_98<condition>()` will assert `condition` at compile time. It also includes a macro `STATIC_ASSERT(condition)`. The macro will not be defined if `STATIC_ASSERT` is already defined elsewhere by some other header, and can be suppressed entirely by defining `DP_NO_ASSERT_MACRO`.
* **type_traits** - A recreation of many of the standard type traits from the standard type_traits header. It is unfortunately missing a few key elements such as `_t` and `_v` helpers and a few traits; however these rely on either language features added in future standards or compiler intrinsics/compiler magic to function so cannot be recreated here.
* **type_traits_ns** - A collection of **n**on-**s**tandard general-purpose type traits which support other parts of this repo; but which are not specified to appear in the modern type_traits header. While they will be needed for some other headers in in the repo to function; this tradeoff seemed the best of options between that greatly complicating code in the other files, or hiding general-purpose traits in the detail namespace and hoping that I don't need them more than once or run into naming collisions.

## FAQ
* **Why do you not have X from Y header?** 
When adding a new feature or header I strive to include every part of that header in modern code which I reasonably can. Many library features in modern C++ rely on modern core language features or compiler magic, and so these are not included in the repo. There are also many features in the modern C++ standard library which are perfectly possible to implement in C++98, and these are what I include.
There does exist a third category. Objects which may be technically possible to implement in C++98 but which are far, far more trouble than it's worth. For instance, it is probably possible to implement `<format>` into C++98 with enough template wizardry, but the tradeoff between implementing all that template work and how useful it would actually be in real code is balanced far too far against using it that it's not worth bothering with.

* **Why do some names match their standard library counterpart (e.g. array) and others don't (e.g. scoped_ptr)?**
  The distinction there is simple - constructs which match the contract of the standard library object (within the confines of C++98) share the name. Constructs which are lacking meaningful parts of their standard library counterpart do not. For example - `scoped_ptr` does not have any move semantics associated with it. It can't, we're in C++98. But move semantics are a large part of what makes `std::unique_ptr` what it is; and a moveable unique-ownership pointer is meaningfully different from a scope-local, unmoveable `scoped_ptr`. On the other hand, `array` matches the interface and requirements of `std::array` more exactly; and while it too is missing move semantics, they are a significantly less important part of what makes `std::array` what it is, so the name is the same.

* **What about range_functions and similar "collection" headers?**
The C++ standard library often mandates that some functions be included in several different headers. Take the range functions for example - as of C++23, `std::begin` is specified to appear in 14 different standard library headers, and none of them exclusively hold those functions. It seemed better to give such headers their own names rather than to use a name of a header which is irrelevant to the contents of the file, or a name which promises more features than I was ultimately able to include.
