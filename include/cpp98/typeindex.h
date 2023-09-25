#ifndef DP_CPP98_TYPEINDEX
#define DP_CPP98_TYPEINDEX

/*
*   Type_index, for ordering types. Unfortunately there is no hash support.
*   No standard hashing support until C++11 and that's a wheel which I haven't reinvented myself.
*/

#include <typeinfo>

namespace dp{

class type_index{
    const std::type_info* m_info; 

    const std::type_info& get() const{
        return *m_info;
    }   

    public:

    type_index(const std::type_info& in) : m_info(&in) {}

    const char* name() const{
        return m_info->name();
    }

    bool operator<(const type_index& rhs) const{
        return m_info->before(rhs.get());
    }
    //Simplest way to guarantee equivalence
    bool operator==(const type_index& rhs) const{
        return !(*this < rhs) && !(rhs < *this);
    }
    bool operator!=(const type_index& rhs) const{
        return !(*this == rhs);
    }
    bool operator>(const type_index& rhs) const{
        return rhs < *this;
    }
    bool operator<=(const type_index& rhs) const{
        return !(rhs < *this);
    }
    bool operator>=(const type_index& rhs) const{
        return !(*this < rhs);
    }

};

}
#endif