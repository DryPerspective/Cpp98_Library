# Cpp98_Tools
A small collection of tools which are compatible with the C++98 standard as well as all future standards. Most of these tools accomplish something which was added in a future standard (e.g. Optional is an analogue of `std::optional`), and I will recommend that if you are using them and you update to a lanugage standard which supports that feature you swap out the tool accordingly - I have designed the interfaces of these tools to make it as painless a process as possible.
Created because I spend a fair bit of time maintaining and adapting legacy code which for various reasons cannot be updated to modern C++.

All tools in this repo exist in `namespace dp`

## List of Features
* **Array** - An analogue of `std::array`, sharing the same functionality and interface. 
* **Optional** - An an analogue of `std::optional` - a class which optionally contains a value, is in a well-defined state in all cases, and which does not construct a held value until it is required to hold one.
* **RCBase** - A general-purpose base class for creating reference counted, COW classes. Intended for use in RCPtr as well as other classes which seek to add ref-counting, COW behaviour.
* **RCPtr** - A reference-counted smart pointer, an analogue of `std::shared_ptr`. Supports single-object and array versions, as well as a make_RCPtr function (analogue of `std::make_shared`), for single values only. In the array case, I'm still working on finding the cleanest solution to ensure overload resolution picks the correct function in even the most pathological cases.
* **SmrtPtr** - A unique-owning, scope-local smart pointer. An analogue of `std::unique_ptr`, but of course without move semantics. Supports single-object and array versions as well as a make_Smrt function (analogue of `std::make_unique`) for both single value and array types.
