#ifndef DP_CPP98_BORLAND_STRING_PROCESSING
#define DP_CPP98_BORLAND_STRING_PROCESSING


/*
*  Much of my work is done in Embarcadero C++Builder still running C++98, and indeed that was in part the impetus to make this library.
*  There is one snag however - Embarcadero string types. These types are not convertible with std::string types, use their own idea of indexing
*  and processing, and ultimately are their own awkward mess to deal with.
*  Rather than sully the rest of this ISO C++98 library with a bunch of checks against Embarcadero types, I provide this header, which adapts
*  the core elements of the library to those types, which can be safely ignored by people in a different predicament from my own.
*/

#ifndef __BORLANDC__
#error "This header requires a Borland/C++Builder compiler."
#elif defined(DP_BORLAND) //If on C++98 Borland

#include "cpp98/string_view.h"
#include "cpp98/iterator.h"

namespace dp {


    class AnsiString_view : public dp::string_view {

        typedef dp::string_view Base;

    public:

        AnsiString_view() : Base() {}
        AnsiString_view(const Base& other) : Base(other) {}
        AnsiString_view(Base::const_pointer begin, Base::size_type size) :Base(begin, size) {}
        AnsiString_view(Base::const_pointer begin) : Base(begin) {}
		AnsiString_view(const std::string& str) : Base(str) {}

        AnsiString_view(const System::AnsiString& str) : Base(str.c_str()) {}

		using Base::operator=;

		operator AnsiString() const{
			return AnsiString(this->data(), this->length());
		}

    };

    class UnicodeString_view : public dp::wstring_view {

		typedef dp::wstring_view Base;

        //To prevent implicit conversions to a temporary Unicodestring, and lifetime issues.
		UnicodeString_view(const char*);

    public:

        UnicodeString_view() : Base() {}
        UnicodeString_view(const Base& other) : Base(other) {}
        UnicodeString_view(Base::const_pointer begin, Base::size_type size) :Base(begin, size) {}
		UnicodeString_view(Base::const_pointer begin) : Base(begin) {}
		UnicodeString_view(const std::wstring& str) : Base(str) {}

		UnicodeString_view(const System::UnicodeString& str) : Base(str.c_str()) {}




		using Base::operator=;

		operator UnicodeString() const{
			return UnicodeString(this->data(), this->length());
		}

    };

    inline const char* data(dp::AnsiString_view in) {
        return in.data();
    }
    inline const wchar_t* data(dp::UnicodeString_view in) {
        return in.data();
    }
    inline const char* data(const System::AnsiString& in) {
		return in.c_str();      //We use c_str() because .data() returns a const void* in Borland world.
    }

    inline const wchar_t* data(const System::UnicodeString& in) {
        return in.c_str();
    }

    inline bool empty(const System::AnsiString& in) {
        return in.IsEmpty();
    }
    inline bool empty(const System::UnicodeString& in) {
        return in.IsEmpty();
    }

    inline std::size_t size(const System::AnsiString& in) {
        return in.Length();
    }
    inline std::size_t size(const System::UnicodeString& in) {
        return in.Length();
    }

    inline std::ptrdiff_t ssize(const System::AnsiString& in) {
        return in.Length();
    }
    inline std::ptrdiff_t ssize(const System::UnicodeString& in) {
        return in.Length();
    }



}
#else 
//Otherwise we must be on at least C++17 Borland
#include <string_view>

namespace dp {
    class AnsiString_view : public std::string_view {

        using Base = std::string_view;

    public:
        using Base::Base;
        AnsiString_view(const AnsiString& as) : Base(as.c_str()) {}

        explicit operator AnsiString() const {
            return AnsiString(this->data(), this->length());
        }
    };

    class UnicodeString_view : public std::wstring_view {

        using Base = std::wstring_view;

    public:
        using Base::Base;
        UnicodeString_view(const UnicodeString& us) : Base(us.c_str()) {}

        //Avoid a temporary from Borland's bending of the type system
        UnicodeString_view(const char*) = delete;


        explicit operator UnicodeString() const {
            return UnicodeString(this->data(), this->length());
        }
    };

    inline namespace literals {
        inline namespace string_view_literals {
            constexpr dp::AnsiString_view operator""_asv(const char* str, std::size_t size) {
                return dp::AnsiString_view(str, size);
            }
            constexpr dp::UnicodeString_view operator""_usv(const wchar_t* str, std::size_t size) {
                return dp::UnicodeString_view(str, size);
            }
    }
}

#endif  	//ifdef __BORLANDC__
#endif      //Header guard