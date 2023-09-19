#ifndef DP_CPP98_ARRAY
#define DP_CPP98_ARRAY

#include <cctype>
#include <stdexcept>
#include <algorithm>
#include <iterator>

/*
* A pre-C++11 stand-in for std::array, which mimicks its interface and functionality.
* This means it can (and should) be replaced with std::array when compiling in C++11 and up
*/

namespace dp{

template<typename T, std::size_t N>
struct array{
    T m_data[N];

    typedef T                   value_type;
    typedef std::size_t         size_type;
    typedef std::ptrdiff_t      difference_type;
    typedef value_type&         reference;
    typedef const value_type&   const_reference;
    typedef value_type*         pointer;
    typedef const value_type*   const_pointer;

    typedef value_type*                             iterator;
    typedef const value_type*                       const_iterator;
    typedef std::reverse_iterator<iterator>         reverse_iterator;
    typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;


    reference at(size_type index){
        if(index >= N) throw std::out_of_range("Bad array access");
        return m_data[index];
    }

    const_reference at(size_type index) const{
        if(index >= N) throw std::out_of_range("Bad array access");
        return m_data[index];
    }

    reference operator[](size_type index){
        return m_data[index];
    }

    const_reference operator[](size_type index) const{
        return m_data[index];
    }

    reference front(){
        return m_data[0];
    }

    const_reference front() const{
        return m_data[0];
    }

    reference back(){
        return m_data[N-1];
    }

    const_reference back() const{
        return m_data[N-1];
    }

    pointer data(){
        return m_data;
    }

    const_pointer data() const{
        return m_data;
    }

    size_type size() const{
        return N;
    }

    size_type max_size() const{
        return size();
    }

    bool empty() const{
        return size() == 0;
    }

    void fill(const_reference value){
        std::fill(this->begin(), this->end(), value);
    }

    void swap(array& other){
        std::swap_ranges(this->begin(), this->end(), other.begin());
    }

    iterator begin(){
        return iterator(m_data);
    }

    const_iterator begin() const{
        return iterator(m_data);
    }

    iterator end(){
        return iterator(m_data + size());
    }

    const_iterator end() const{
        return iterator(m_data + size());
    }

    //Admittedly the following iterator functions were only standardised in C++11
    //But for the sake of completeness
    const_iterator cbegin() const{
        return iterator(m_data);
    }

    const_iterator cend() const{
        return iterator(m_data + size());
    }

    reverse_iterator rbegin(){
        return reverse_iterator(end());
    }

    reverse_iterator rend(){
        return reverse_iterator(begin());
    }

    const_reverse_iterator rbegin() const{
        return reverse_iterator(end());
    }

    const_reverse_iterator rend() const{
        return reverse_iterator(begin());
    }

    const_reverse_iterator crbegin() const{
        return reverse_iterator(end());
    }

    const_reverse_iterator crend() const{
        return reverse_iterator(begin());
    }

};

template<typename T, std::size_t N>
bool operator==(const array<T,N>& lhs, const array<T,N>& rhs){
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template<typename T, std::size_t N>
bool operator!=(const array<T,N>& lhs, const array<T,N>& rhs){
    return !(lhs == rhs);
}

template<typename T, std::size_t N>
bool operator<(const array<T,N>& lhs, const array<T,N>& rhs){
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template<typename T, std::size_t N>
bool operator <=(const array<T,N>& lhs, const array<T,N>& rhs){
    return (lhs == rhs) || (lhs < rhs);
}

template<typename T, std::size_t N>
bool operator>(const array<T,N>& lhs, const array<T,N>& rhs){
    return !(lhs <= rhs);
}

template<typename T, std::size_t N>
bool operator >=(const array<T,N>& lhs, const array<T,N>& rhs){
    return !(lhs < rhs);
}

template<typename T, std::size_t N>
void swap(array<T,N>& lhs, array<T,N>& rhs){
    lhs.swap(rhs);
}

}

#endif