#ifndef DP_CPP98_STRING_VIEW
#define DP_CPP98_STRING_VIEW

#include <iterator>
#include <stdexcept>
#include <memory>
#include <algorithm>
#include <string>
#include <ostream>

#include "cpp98/iterator.h"
#include "bits/fat_pointer.h"
#include "bits/misc_memory_functions.h"


namespace dp{
    template<typename CharT, typename Traits = std::char_traits<CharT> >
    class basic_string_view{

        dp::fat_pointer<const CharT> ptr;

        public:
        typedef Traits          traits_type;
        typedef CharT           value_type;
        typedef CharT*          pointer;
        typedef const CharT*    const_pointer;
        typedef CharT&          reference;
        typedef const CharT&    const_reference;
        typedef std::size_t     size_type;
        typedef std::ptrdiff_t  difference_type;

        typedef const CharT*                            const_iterator;
        typedef const_iterator                          iterator;
        typedef std::reverse_iterator<iterator>         reverse_iterator;
        typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;

        static const size_type npos = std::basic_string<CharT, Traits>::npos;

        basic_string_view() : ptr(NULL, 0) {}
        basic_string_view(const basic_string_view& other) : ptr(other.ptr) {}
        basic_string_view(const CharT* begin, size_type size) : ptr(begin, size) {}
        basic_string_view(const CharT* begin) : ptr(begin, Traits::length(begin)) {}
        //We can't take the standard way of attaching a converting operator to std::string
        basic_string_view(const std::basic_string<CharT, Traits>& str) : ptr(str.data(), str.size()) {}

        basic_string_view& operator=(const basic_string_view& other) {
            ptr = other.ptr;
            return *this;
        }

        iterator begin() const{
            return ptr;
        }
        const_iterator cbegin() const{
            return ptr;
        }
        iterator end() const{
            return ptr.begin + ptr.size;
        }
        const_iterator cend() const{
            return ptr.begin + ptr.size;
        }
        reverse_iterator rbegin() const{
            return reverse_iterator(end());
        }
        const_reverse_iterator crbegin() const{
            return const_reverse_iterator(cend());
        }
        reverse_iterator rend() const{
            return reverse_iterator(begin());
        }
        const_reverse_iterator crend() const{
            return const_reverse_iterator(cbegin());
        }

        const_reference operator[](size_type index) const{
            return ptr[index];
        }
        const_reference at(size_type index) const{
            if(index >= ptr.size) throw std::out_of_range("Out of range string_view access");
            return ptr[index];
        }

        const_reference front() const{
            return ptr[0];
        }
        const_reference back() const{
            return ptr[ptr.size - 1];
        }

        const_pointer data() const{
            return ptr;
        }

        size_type size() const{
            return ptr.size;
        }
        size_type length() const{
            return size();
        }

        size_type max_size() const{
            return std::allocator<CharT>::max_size();
        }

        bool empty() const{
            return size() == 0;
        }

        void remove_prefix(size_type n){
            std::advance(ptr.begin, n);
            ptr.size -= n;
        }

        void remove_suffix(size_type n){
            ptr.size -= n;
        }

        void swap(basic_string_view& other){
            std::swap(ptr.begin, other.ptr.begin);
            std::swap(ptr.size, other.ptr.size);
        }

        size_type copy(CharT* dest, size_type count, size_type pos = 0) const{
            size_type rcount =  std::min(count, size() - pos);
            Traits::copy(dest, data() + pos, rcount);
            return rcount;
        }

        basic_string_view substr(size_type pos = 0, size_type count = npos){
            if(pos > size()) throw std::out_of_range("Out of range string_view access");
            return basic_string_view(ptr.begin + pos, ptr.begin + pos + std::min(count, size() - pos));
        }

        int compare(basic_string_view v) const{
            return Traits::compare(data(), v.data(), std::min(size(), v.size()));
        }
        int compare(size_type pos1, size_type count1, basic_string_view v) const{
            return substr(pos1, count1).compare(v);
        }
        int compare(size_type pos1, size_type count1, basic_string_view v, size_type pos2, size_type count2 ) const{
            return substr(pos1, count1).compare(v.substr(pos2, count2));
        }
        int compare(const CharT* s) const{
            return compare(basic_string_view(s));
        }
        int compare(size_type pos1, size_type count1, const CharT* s) const{
            return substr(pos1, count1).compare(basic_string_view(s));
        }
        int compare(size_type pos1, size_type count1, const CharT* s, size_type count2) const{
            return substr(pos1, count1).compare(basic_string_view(s, count2));
        }

        bool starts_with(basic_string_view v) const{
            return substr(0, v.size()) == v;
        }
        bool starts_with(CharT ch){
            return !empty() && Traits::eq(front(), ch);
        }
        bool starts_with(const CharT* s){
            return starts_with(basic_string_view(s));
        }

        bool ends_with(basic_string_view v) const{
            return size() >= v.size() && compare(size() - v.size(), npos, v) == 0;
        }
        bool ends_with(CharT ch){
            return !empty() && Traits::eq(back(), ch);
        }
        bool ends_with(const CharT* s){
            return ends_with(basic_string_view(s));
        }

        bool contains(basic_string_view v) const{
            return find(v) != npos;
        }
        bool contains(CharT ch) const{
            return find(ch) != npos;
        }
        bool contains(const CharT* s) const{
            return find(s) != npos;
        }

        size_type find(basic_string_view v, size_type pos = 0) const{
            //Alas we must use indexing as we need to return the index.
            for(size_type i = pos; i < size(); ++i){
                if(Traits::eq(ptr[i],v.front())){
                    //Iterate down both views in parallel
                    size_type thisIndex = i;
                    for(size_type otherIndex = 0; 
                        thisIndex < (size() - thisIndex) && otherIndex < v.size(); ++thisIndex, ++otherIndex){
                            //And commit a minor heresy to break out of this nested loop if they don't match.
                            if(!Traits::eq(ptr[thisIndex], v[otherIndex])) goto NextIteration;
                    }
                    //If we get this far we have a full match
                    return i;
                    //Otherwise we keep trying
                    NextIteration:;
                }
            }
            return npos;
        }
        size_type find(CharT ch, size_type pos = 0) const{
            return find(basic_string_view(dp::addressof(ch), 1), pos);
        }
        size_type find(const CharT* s, size_type pos, size_type count) const{
            return find(basic_string_view(s, count), pos);
        }
        size_type find(const CharT* s, size_type pos = 0) const{
            return find(basic_string_view(s), pos);
        }

        size_type rfind(basic_string_view v, size_type pos = npos) const{
            size_type i = std::min(size() - 1, pos);
            for(const_reverse_iterator It = crbegin(); It != crend(); ++It, --i){
                if(Traits::eq(*It, v.back())){
                    size_type thisIter = i;
                    for(const_reverse_iterator It = v.crbegin(); It != v.crend(); ++It, --thisIter){
                        if(!Traits::eq(ptr[thisIter],*It)) goto NextIteration;
                    }
                    return i;
                    NextIteration:;
                }
            }
            return npos;
        }
        size_type rfind(CharT ch, size_type pos = npos) const{
            return rfind(basic_string_view(dp::addressof(ch), 1), pos);
        }
        size_type rfind(const CharT* s, size_type pos, size_type count) const{
            return rfind(basic_string_view(s, count), pos);
        }
        size_type rfind(const CharT* s, size_type pos = npos) const{
            return rfind(basic_string_view(s), pos);
        }

        size_type find_first_of(basic_string_view v, size_type pos = 0) const{
            for(size_type i = pos; i < size(); ++i){
                for(const_iterator it = v.cbegin(); it != v.cend(); ++it){
                    if(Traits::eq(ptr[i],*it)) return i;
                }
            }
            return npos;
        }
        size_type find_first_of(CharT ch, size_type pos = 0) const{
            return find_first_of(basic_string_view(dp::addressof(ch), 1), pos);
        }
        size_type find_first_of(const CharT* s, size_type pos, size_type count) const{
            return find_first_of(basic_string_view(s, count), pos);
        }
        size_type find_first_of(const CharT* s, size_type pos = 0) const{
            return find_first_of(basic_string_view(s), pos);
        }

        size_type find_last_of(basic_string_view v, size_type pos = npos) const{
            size_type i = std::min(size() - 1, pos);
            for(const_reverse_iterator It = crbegin(); It != crend(); ++It, --i){
                for(const_iterator vIt = v.cbegin(); vIt != v.cend(); ++vIt){
                    if(Traits::eq(*It,*vIt)) return i;
                }
            }
            return npos;
        }
        size_type find_last_of(CharT ch, size_type pos = npos) const{
            return find_last_of(basic_string_view(dp::addressof(ch), 1), pos);
        }
        size_type find_last_of(const CharT* s, size_type pos, size_type count) const{
            return find_last_of(basic_string_view(s, count), pos);
        }
        size_type find_last_of(const CharT* s, size_type pos = npos) const{
            return find_last_of(basic_string_view(s), pos);
        }

        size_type find_first_not_of(basic_string_view v, size_type pos = 0) const{
             for(size_type i = pos; i < size(); ++i){
                for(const_iterator it = v.cbegin(); it != v.cend(); ++it){
                    if(!Traits::eq(ptr[i],*it)) return i;
                }
            }
            return npos;           
        }
        size_type find_first_not_of(CharT ch, size_type pos = 0) const{
            return find_first_not_of(basic_string_view(dp::addressof(ch), 1), pos);
        }
        size_type find_first_not_of(const CharT* s, size_type pos, size_type count) const{
            return find_first_not_of(basic_string_view(s, count), pos);
        }
        size_type find_first_not_of(const CharT* s, size_type pos = 0) const{
            return find_first_not_of(basic_string_view(s), pos);
        }

        size_type find_last_not_of(basic_string_view v, size_type pos = npos) const{
            size_type i = std::min(size() - 1, pos);
            for(const_reverse_iterator It = crbegin(); It != crend(); ++It, --i){
                for(const_iterator vIt = v.cbegin(); vIt != v.cend(); ++vIt){
                    if(!Traits::eq(*It,*vIt)) return i;
                }
            }
            return npos;            
        }
        size_type find_last_not_of(CharT ch, size_type pos = npos) const{
            return find_last_not_of(basic_string_view(dp::addressof(ch), 1), pos);
        }
        size_type find_last_not_of(const CharT* s, size_type pos, size_type count) const{
            return find_last_not_of(basic_string_view(s, count), pos);
        }
        size_type find_last_not_of(const CharT* s, size_type pos = npos) const{
            return find_last_not_of(basic_string_view(s), pos);
        }

        operator std::basic_string<CharT, Traits>() const{
            return std::basic_string<CharT, Traits>(begin(), end());
        }
    };

    typedef basic_string_view<char>     string_view;
    typedef basic_string_view<wchar_t>  wstring_view;


    template<typename CharT, typename Traits>
    bool operator==(const basic_string_view<CharT, Traits>& lhs, const basic_string_view<CharT, Traits>& rhs){
        if(lhs.size() != rhs.size()) return false;
        return Traits::compare(lhs.data(), rhs.data(), lhs.size()) == 0;
    }
    template<typename CharT, typename Traits>
    bool operator!=(const basic_string_view<CharT, Traits>& lhs, const basic_string_view<CharT, Traits>& rhs){
        return !(lhs == rhs);
    }
    template<typename CharT, typename Traits>
    bool operator<(const basic_string_view<CharT, Traits>& lhs, const basic_string_view<CharT, Traits>& rhs){
        int comp =  Traits::compare(lhs.data(), rhs.data, std::min(lhs.size(), rhs.size()));
        return comp == 0 ? lhs.size() < rhs.size() : comp < 0 ? true : false;
    }
    template<typename CharT, typename Traits>
    bool operator<=(const basic_string_view<CharT, Traits>& lhs, const basic_string_view<CharT, Traits>& rhs){
        return (lhs == rhs) || (lhs < rhs);
    }
    template<typename CharT, typename Traits>
    bool operator>(const basic_string_view<CharT, Traits>& lhs, const basic_string_view<CharT, Traits>& rhs){
        return rhs < lhs;
    }
    template<typename CharT, typename Traits>
    bool operator>=(const basic_string_view<CharT, Traits>& lhs, const basic_string_view<CharT, Traits>& rhs){
        return rhs <= lhs;
    }

    namespace detail{
        template<typename CharT, typename Traits>
        void os_pad(std::basic_ostream<CharT,Traits>& os, CharT fill, std::streamsize width){
            while(width--) os.put(fill);
        }
    }

    template<typename CharT, typename Traits>
    std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, dp::basic_string_view<CharT,Traits> sv){
        typename std::basic_ostream<CharT, Traits>::sentry s(os);
        if (s) {
            CharT fill = os.fill();
            std::streamsize width = os.width();
            bool left = os.flags() & std::ios::left;
            bool right = os.flags() & std::ios::right;
            bool fixed = os.flags() & std::ios::fixed;

            if (dp::ssize(sv) < width) {
                std::streamsize padding_len = width - sv.size();
                if (right) detail::os_pad(os, fill, padding_len);
                os.write(sv.data(), sv.size());
                if (left) detail::os_pad(os, fill, padding_len);
            }
            else {
                os.write(sv.data(), fixed ? width : sv.size());
            }
        }
        os.width(0);
        return os;
    }

    template<typename CharT, typename Traits>
    std::basic_string<CharT, Traits> operator+=(std::basic_string<CharT, Traits>&  lhs, dp::basic_string_view<CharT, Traits> rhs){
        return lhs += std::basic_string<CharT, Traits>(rhs);
    }
    template<typename CharT, typename Traits>
    std::basic_string<CharT, Traits> operator+(std::basic_string<CharT, Traits> lhs, dp::basic_string_view<CharT, Traits> rhs){
        return lhs += rhs;
    }
    template<typename CharT, typename Traits>
    std::basic_string<CharT, Traits> operator+=(dp::basic_string_view<CharT, Traits> lhs, const std::basic_string<CharT, Traits>& rhs){
        return std::basic_string<CharT, Traits>(lhs) += rhs;
    }
    template<typename CharT, typename Traits>
    std::basic_string<CharT, Traits> operator+(dp::basic_string_view<CharT, Traits> lhs, const std::basic_string<CharT, Traits>& rhs){
        return lhs += rhs; //We're copying lhs anyway so these two end up equivalent
    }
    template<typename CharT, typename Traits>
    std::basic_string<CharT, Traits> operator+=(dp::basic_string_view<CharT, Traits> lhs, dp::basic_string_view<CharT, Traits> rhs){
        return std::basic_string<CharT, Traits>(lhs) += rhs;
    }
    template<typename CharT, typename Traits>
    std::basic_string<CharT, Traits> operator+(dp::basic_string_view<CharT, Traits> lhs, dp::basic_string_view<CharT, Traits> rhs){
        return std::basic_string<CharT, Traits>(lhs) += rhs;
    }

    template<typename CharT, typename Traits>
    void swap(dp::basic_string_view<CharT, Traits>& lhs, dp::basic_string_view<CharT, Traits>& rhs) {
        lhs.swap(rhs);
    }

    //Overload because strings are a special case and want const-qualified pointers, always.
    template<typename CharT, typename Traits>
    typename dp::basic_string_view<CharT, Traits>::const_pointer data(dp::basic_string_view<CharT, Traits> in) {
        return in.data();
    }

}

#endif