# Cpp98_Tools
A small collection of tools which are compatible with the C++98 standard as well as all future standards. Most of these tools accomplish something which was added in a future standard (e.g. Optional is an analogue of `std::optional`), and I will recommend that if you are using them and you update to a lanugage standard which supports that feature you swap out the tool accordingly - I have designed the interfaces of these tools to make it as painless a process as possible.
Created because I spend a fair bit of time maintaining and adapting legacy code which for various reasons cannot be updated to modern C++.

All tools in this repo exist in `namespace dp`

## List of Features
* **array** - An analogue of `std::array`, sharing the same functionality and interface. 
* **optional** - An an analogue of `std::optional` - a class which optionally contains a value, is in a well-defined state in all cases, and which does not construct a held value until it is required to hold one.
* **rc_base** - A general-purpose base class for creating reference counted, COW classes. Intended for use in rc_ptr as well as other classes which seek to add ref-counting, COW behaviour.
* **rc_ptr** - A reference-counted smart pointer, an analogue of `std::shared_ptr`. Supports single-object and array versions, as well as a `make_rc_ptr` function (analogue of `std::make_shared`), for single values only. In the array case, I'm still working on finding the cleanest solution to ensure overload resolution picks the correct function in even the most pathological cases.
* **scoped_ptr** - A unique-owning, scope-local smart pointer. An analogue of `std::unique_ptr`, but of course without move semantics. Supports single-object and array versions as well as a `make_scoped` function (analogue of `std::make_unique`) for both single value and array types.
* **type_traits** - A recreation of many of the standard type traits from the standard type_traits header. It is unfortunately missing a few key elements such as `_t` and `_v` helpers and a few traits; however these rely on either language features added in future standards or compiler intrinsics/compiler magic to function so cannot be recreated here.

## FAQ
* **Why do some names match their standard library counterpart (e.g. array) and others don't (e.g. scoped_ptr)?**
  The distinction there is simple - constructs which match the contract of the standard library object (within the confines of C++98) share the name. Constructs which are lacking meaningful parts of their standard library counterpart do not. For example - `scoped_ptr` does not have any move semantics associated with it. It can't, we're in C++98. But move semantics are a large part of what makes `std::unique_ptr` what it is; and a moveable unique-ownership pointer is meaningfully different from a scope-local, unmoveable `scoped_ptr`. On the other hand, `array` matches the interface and requirements of `std::array` more exactly; and while it too is missing move semantics, they are a significantly less important part of what makes `std::array` what it is, so the name is the same.
