#ifndef DP_CPP98_CCTYPE
#define DP_CPP98_CCTYPE

#include <cctype>

/*
*   For various reasons, there are two problems with the stock cctype functions
*   * It is illegal to take their address, so they cannot be used in range algorithm calls
*   * They deal in ints rather than character types so their use is undefined if the platform's char type cannot be represented as an unsigned char or EOF
* 
*   These functions solve both problems.
* 
*  Full documentation at: https://github.com/DryPerspective/Cpp98_Library/wiki/cctype
*/

namespace dp {

    inline bool isalnum(char ch) {
        return std::isalnum(static_cast<unsigned char>(ch));
    }

    inline bool isalpha(char ch) {
        return std::isalpha(static_cast<unsigned char>(ch));
    }

    inline bool islower(char ch) {
        return std::islower(static_cast<unsigned char>(ch));
    }

    inline bool isupper(char ch) {
        return std::isupper(static_cast<unsigned char>(ch));
    }

    inline bool isdigit(char ch) {
        return std::isdigit(static_cast<unsigned char>(ch));
    }

    inline bool isxdigit(char ch) {
        return std::isxdigit(static_cast<unsigned char>(ch));
    }

    inline bool iscntrl(char ch) {
        return std::iscntrl(static_cast<unsigned char>(ch));
    }

    inline bool isgraph(char ch) {
        return std::isgraph(static_cast<unsigned char>(ch));
    }

    inline bool isspace(char ch) {
        return std::isspace(static_cast<unsigned char>(ch));
    }

    inline bool isprint(char ch) {
        return std::isprint(static_cast<unsigned char>(ch));
    }

    inline bool ispunct(char ch) {
        return std::ispunct(static_cast<unsigned char>(ch));
    }

    inline char tolower(char ch) {
        return static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }

    inline char toupper(char ch) {
        return static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    }

}


#endif