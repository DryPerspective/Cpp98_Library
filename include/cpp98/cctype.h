#include <cctype>

/*
*   For various reasons, there are two problems with the stock cctype functions
*   * It is illegal to take their address, so they cannot be used in range algorithm calls
*   * They deal in ints rather than character types so their use is undefined if the platform's char type cannot be represented as an unsigned char or EOF
* 
*   These functions solve both problems.
*/

namespace dp {

    bool isalnum(char ch) {
        return std::isalnum(static_cast<unsigned char>(ch));
    }

    bool isalpha(char ch) {
        return std::isalpha(static_cast<unsigned char>(ch));
    }

    bool islower(char ch) {
        return std::islower(static_cast<unsigned char>(ch));
    }

    bool isupper(char ch) {
        return std::isupper(static_cast<unsigned char>(ch));
    }

    bool isdigit(char ch) {
        return std::isdigit(static_cast<unsigned char>(ch));
    }

    bool isxdigit(char ch) {
        return std::isxdigit(static_cast<unsigned char>(ch));
    }

    bool iscntrl(char ch) {
        return std::iscntrl(static_cast<unsigned char>(ch));
    }

    bool isgraph(char ch) {
        return std::isgraph(static_cast<unsigned char>(ch));
    }

    bool isspace(char ch) {
        return std::isspace(static_cast<unsigned char>(ch));
    }

    bool isprint(char ch) {
        return std::isprint(static_cast<unsigned char>(ch));
    }

    bool ispunct(char ch) {
        return std::ispunct(static_cast<unsigned char>(ch));
    }

    char tolower(char ch) {
        return static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }

    char toupper(char ch) {
        return static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    }

}
