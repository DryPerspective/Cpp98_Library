#ifndef DP_CPP98_STRING
#define DP_CPP98_STRING

#include <string>
#include <algorithm>
#include <functional>
#include <cstdlib>
#include <cerrno>
#include <stdexcept>
#include <cstdio>
#include <cwchar>
#include "cpp98/type_traits.h"
#include "cpp98/iterator.h"

//Full documentation at: https://github.com/DryPerspective/Cpp98_Library/wiki/String

namespace dp {

	/*
	*  Note that std::erase and std::erase_if are from C++20 so don't have standard counterparts in C++17
	*/

	template< class CharT, class Traits, class Alloc, class Pred >
	typename std::basic_string<CharT, Traits, Alloc>::size_type erase_if(std::basic_string<CharT, Traits, Alloc>& c, Pred pred) {
		typedef typename std::basic_string<CharT, Traits, Alloc> StringT;
		typename StringT::iterator it = std::remove_if(c.begin(), c.end(), pred);
		typename StringT::size_type r = c.end() - it;
		c.erase(it, c.end());
		return r;
	}

	template< class CharT, class Traits, class Alloc, class U >
	typename std::basic_string<CharT, Traits, Alloc>::size_type erase(std::basic_string<CharT, Traits, Alloc>& c, const U& value) {
		typedef typename std::basic_string<CharT, Traits, Alloc> StringT;
		typename StringT::iterator it = std::remove(c.begin(), c.end(), value);
		typename StringT::size_type r = c.end() - it;
		c.erase(it, c.end());
		return r;
	}

	/*
	*	String conversion functions
	*/

	inline long stol(const std::string& str, std::size_t* pos = NULL, int base = 10) {
		const char* ptr = str.c_str();
		char* errPtr = NULL;

		errno = 0;
		long val = std::strtol(ptr, &errPtr, base);
		if (errno == ERANGE) {
			throw std::out_of_range("string conversion out of range");
		}
		if (ptr == errPtr) {
			throw std::invalid_argument("string conversion invalid arg");
		}
		if (pos) {
			*pos = static_cast<std::size_t>(errPtr - ptr);
		}
		return static_cast<long>(val);
	}

	inline long stol(const std::wstring& str, std::size_t* pos = NULL, int base = 10) {
		const wchar_t* ptr = str.c_str();
		wchar_t* errPtr = NULL;

		errno = 0;
		long val = std::wcstol(ptr, &errPtr, base);
		if (errno == ERANGE) {
			throw std::out_of_range("string conversion out of range");
		}
		if (ptr == errPtr) {
			throw std::invalid_argument("string conversion invalid arg");
		}
		if (pos) {
			*pos = static_cast<std::size_t>(errPtr - ptr);
		}
		return static_cast<long>(val);
	}

	inline int stoi(const std::string& str, std::size_t* pos = NULL, int base = 10) {
		return static_cast<int>(dp::stol(str, pos, base));
	}

	inline int stoi(const std::wstring& str, std::size_t* pos = NULL, int base = 10) {
		return static_cast<int>(dp::stol(str, pos, base));
	}

	inline unsigned long stoul(const std::string& str, std::size_t* pos = NULL, int base = 10) {
		const char* ptr = str.c_str();
		char* errPtr = NULL;

		errno = 0;
		long val = std::strtoul(ptr, &errPtr, base);
		if (errno == ERANGE) {
			throw std::out_of_range("string conversion out of range");
		}
		if (ptr == errPtr) {
			throw std::invalid_argument("string conversion invalid arg");
		}
		if (pos) {
			*pos = static_cast<std::size_t>(errPtr - ptr);
		}
		return static_cast<unsigned long>(val);
	}

	inline unsigned long stoul(const std::wstring& str, std::size_t* pos = NULL, int base = 10) {
		const wchar_t* ptr = str.c_str();
		wchar_t* errPtr = NULL;

		errno = 0;
		long val = std::wcstoul(ptr, &errPtr, base);
		if (errno == ERANGE) {
			throw std::out_of_range("string conversion out of range");
		}
		if (ptr == errPtr) {
			throw std::invalid_argument("string conversion invalid arg");
		}
		if (pos) {
			*pos = static_cast<std::size_t>(errPtr - ptr);
		}
		return static_cast<unsigned long>(val);
	}

	inline float stof(const std::string& str, std::size_t* pos = NULL) {
		const char* ptr = str.c_str();
		char* errPtr = NULL;

		errno = 0;
		double val = std::strtod(ptr, &errPtr);
		if (errno == ERANGE) {
			throw std::out_of_range("string conversion out of range");
		}
		if (ptr == errPtr) {
			throw std::invalid_argument("string conversion invalid arg");
		}
		if (pos) {
			*pos = static_cast<std::size_t>(errPtr - ptr);
		}
		return static_cast<float>(val);
	}

	inline float stof(const std::wstring& str, std::size_t* pos = NULL) {
		const wchar_t* ptr = str.c_str();
		wchar_t* errPtr = NULL;

		errno = 0;
		double val = std::wcstod(ptr, &errPtr);
		if (errno == ERANGE) {
			throw std::out_of_range("string conversion out of range");
		}
		if (ptr == errPtr) {
			throw std::invalid_argument("string conversion invalid arg");
		}
		if (pos) {
			*pos = static_cast<std::size_t>(errPtr - ptr);
		}
		return static_cast<float>(val);
	}

	inline double stod(const std::string& str, std::size_t* pos = NULL) {
		const char* ptr = str.c_str();
		char* errPtr = NULL;

		errno = 0;
		double val = std::strtod(ptr, &errPtr);
		if (errno == ERANGE) {
			throw std::out_of_range("string conversion out of range");
		}
		if (ptr == errPtr) {
			throw std::invalid_argument("string conversion invalid arg");
		}
		if (pos) {
			*pos = static_cast<std::size_t>(errPtr - ptr);
		}
		return static_cast<double>(val);
	}

	inline double stod(const std::wstring& str, std::size_t* pos = NULL) {
		const wchar_t* ptr = str.c_str();
		wchar_t* errPtr = NULL;

		errno = 0;
		double val = std::wcstod(ptr, &errPtr);
		if (errno == ERANGE) {
			throw std::out_of_range("string conversion out of range");
		}
		if (ptr == errPtr) {
			throw std::invalid_argument("string conversion invalid arg");
		}
		if (pos) {
			*pos = static_cast<std::size_t>(errPtr - ptr);
		}
		return static_cast<double>(val);
	}

	inline long double stold(const std::string& str, std::size_t* pos = NULL) {
		const char* ptr = str.c_str();
		char* errPtr = NULL;

		errno = 0;
		long double val = std::strtold(ptr, &errPtr);
		if (errno == ERANGE) {
			throw std::out_of_range("string conversion out of range");
		}
		if (ptr == errPtr) {
			throw std::invalid_argument("string conversion invalid arg");
		}
		if (pos) {
			*pos = static_cast<std::size_t>(errPtr - ptr);
		}
		return static_cast<long double>(val);
	}

	inline long double stold(const std::wstring& str, std::size_t* pos = NULL) {
		const wchar_t* ptr = str.c_str();
		wchar_t* errPtr = NULL;

		errno = 0;
		long double val = std::wcstold(ptr, &errPtr);
		if (errno == ERANGE) {
			throw std::out_of_range("string conversion out of range");
		}
		if (ptr == errPtr) {
			throw std::invalid_argument("string conversion invalid arg");
		}
		if (pos) {
			*pos = static_cast<std::size_t>(errPtr - ptr);
		}
		return static_cast<long double>(val);
	}

	/*
	*	DR Functions
	*/
	template<typename CharT, typename Traits, typename Alloc>
	CharT& front(std::basic_string<CharT, Traits, Alloc>& str){
		return str[0];
	}
	template<typename CharT, typename Traits, typename Alloc>
	CharT& back(std::basic_string<CharT, Traits, Alloc>& str){
		return str[str.size() - 1];
	}
	template<typename CharT, typename Traits, typename Alloc>
	const CharT& front(const std::basic_string<CharT, Traits, Alloc>& str){
		return str[0];
	}
	template<typename CharT, typename Traits, typename Alloc>
	const CharT& back(const std::basic_string<CharT, Traits, Alloc>& str){
		return str[str.size() - 1];
	}
	template<typename CharT, typename Traits, typename Alloc>
	void pop_back(std::basic_string<CharT, Traits, Alloc>& str){
		str.erase(str.end() - 1);
	}
	template<typename CharT, typename Traits, typename Alloc>
	typename std::basic_string<CharT, Traits, Alloc>::const_iterator cbegin(const std::basic_string<CharT, Traits, Alloc>& in){
		return typename std::basic_string<CharT, Traits, Alloc>::const_iterator(in.begin());
	}
	template<typename CharT, typename Traits, typename Alloc>
	typename std::basic_string<CharT, Traits, Alloc>::const_iterator cend(const std::basic_string<CharT, Traits, Alloc>& in){
		return typename std::basic_string<CharT, Traits, Alloc>::const_iterator(in.end());
	}
	template<typename CharT, typename Traits, typename Alloc>
	typename std::basic_string<CharT, Traits, Alloc>::const_reverse_iterator crbegin(const std::basic_string<CharT, Traits, Alloc>& in){
		return typename std::basic_string<CharT, Traits, Alloc>::const_reverse_iterator(in.rbegin());
	}
	template<typename CharT, typename Traits, typename Alloc>
	typename std::basic_string<CharT, Traits, Alloc>::const_reverse_iterator crend(const std::basic_string<CharT, Traits, Alloc>& in){
		return typename std::basic_string<CharT, Traits, Alloc>::const_reverse_iterator(in.rend());
	}



#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

	template<typename T>
	typename dp::enable_if<dp::is_signed<T>::value && dp::is_integral<T>::value, std::string>::type to_string(T in) {
		char buf[21];	//Big enough to hold the full range
		std::sprintf(buf, "%ld", in);
		return std::string(dp::begin(buf), std::find(dp::begin(buf), dp::end(buf),0)); 
	}
	template<typename T>
	typename dp::enable_if<dp::is_unsigned<T>::value && dp::is_integral<T>::value, std::string>::type to_string(T in) {
		char buf[21];
		std::sprintf(buf, "%lu", in);
		return std::string(dp::begin(buf), std::find(dp::begin(buf), dp::end(buf), 0));
	}
	template<typename T>
	typename dp::enable_if<dp::is_same<T, float>::value || dp::is_same<T, double>::value, std::string>::type to_string(long double in) {
		char buf[21];
		std::sprintf(buf, "%Lf", in);
		std::string str(dp::begin(buf), std::find(dp::begin(buf), dp::end(buf), 0));
		if (!str.empty() && dp::back(str) == '0') {
			std::size_t lastChar = str.find_last_not_of('0');
			str = str.substr(0, lastChar + 1);
		}
		return str;
	}

	std::string to_string(long double in) {
		char buf[21];
		std::sprintf(buf, "%Lf", in);
		std::string str(dp::begin(buf), std::find(dp::begin(buf), dp::end(buf), 0));
		if (!str.empty() && dp::back(str) == '0') {
			std::size_t lastChar = str.find_last_not_of('0');
			str = str.substr(0, lastChar + 1);
		}
		return str;
	}

	template<typename T>
	typename dp::enable_if<dp::is_signed<T>::value && dp::is_integral<T>::value, std::wstring>::type to_wstring(T in) {
		wchar_t buf[21];
		std::swprintf(buf, 20, L"%ld", in);
		return std::wstring(dp::begin(buf), std::find(dp::begin(buf), dp::end(buf), 0));
	}
	template<typename T>
	typename dp::enable_if<dp::is_unsigned<T>::value && dp::is_integral<T>::value, std::wstring>::type to_wstring(T in) {
		wchar_t buf[21];
		std::swprintf(buf, 20, L"%lu", in);
		return std::wstring(dp::begin(buf), std::find(dp::begin(buf), dp::end(buf), 0));
	}
	template<typename T>
	typename dp::enable_if<dp::is_same<T, float>::value || dp::is_same<T, double>::value, std::string>::type to_wstring(T in){
		wchar_t buf[21];
		std::swprintf(buf, 20, L"%f", in);
		std::wstring str(dp::begin(buf), std::find(dp::begin(buf), dp::end(buf), 0));
		if (!str.empty() && dp::back(str) == '0') {
			std::size_t lastChar = str.find_last_not_of('0');
			str = str.substr(0, lastChar + 1);
		}
		return str;		
	}
	std::wstring to_wstring(long double in) {
		wchar_t buf[21];
		std::swprintf(buf, 20, L"%Lf", in);
		std::wstring str(dp::begin(buf), std::find(dp::begin(buf), dp::end(buf), 0));
		if (!str.empty() && dp::back(str) == '0') {
			std::size_t lastChar = str.find_last_not_of('0');
			str = str.substr(0, lastChar + 1);
		}
		return str;
	}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}
#endif