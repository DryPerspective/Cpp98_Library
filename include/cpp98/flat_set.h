#ifndef DP_FLAT_SET
#define DP_FLAT_SET

/*
*   Flat set, which mimicks the C++23 feature of the same name.
*   As we drift further into modern C++ and further away from C++98, it becomes progressively
*   harder to make even a fair representation of the interface and functionality of the real thing.
*   Case in point: This set can't use uses-allocator construction, because even if we could implement it
*   we can't add those constructors to std::vector. We have no move semantics so the operations which extract
*   or replace the original container must invoke copies.
*   At some level, I'm a little uneasy with a library tool which breaks its counterpart's contract
*   in certain subtle ways. But the number of times I've needed de facto this in real code keeps growing
*   and maintaining the invariant without constant checking for sorted and uniqued will make things cleaner.
*/

#include <functional>
#include <vector>
#include <iterator>
#include <algorithm>

namespace dp {

    struct sorted_unique_t {};
    static const sorted_unique_t sorted_unique = {};

    template<typename Key, typename Comp = std::less<Key>, typename Container = std::vector<Key> >
    class flat_set : private Comp {  //Private inheritance in the hope for EBO on stateless comps.

        Container m_storage;

        Comp& get_comp() {
            return static_cast<Comp&>(*this);
        }
        const Comp& get_comp() const {
            return static_cast<const Comp&>(*this);
        }

        //Why a struct? Because we need to pass it into functions which expect an equality relation
        //And PMFs are tricky without binders.
        //Also, the comparison may be stateful so we need this specific instance's comparison functor
        struct equal_from_comp {
            Comp& m_comp;
            equal_from_comp(Comp& in) : m_comp(in) {}

            bool operator()(const Key& lhs, const Key& rhs) const {
                return !m_comp(lhs, rhs) && !m_comp(rhs, lhs);
            }
        };

    public:
        typedef Container                                       container_type;
        typedef Key                                             key_type;
        typedef Key                                             value_type;
        typedef Comp                                            key_compare;
        typedef Comp                                            value_compare;
        typedef Key& reference;
        typedef const Key& const_reference;
        typedef typename Container::size_type                   size_type;
        typedef typename Container::difference_type             difference_type;
        typedef typename Container::iterator                    iterator;
        typedef typename Container::const_iterator              const_iterator;
        typedef typename std::reverse_iterator<iterator>        reverse_iterator;
        typedef typename std::reverse_iterator<const_iterator>  const_reverse_iterator;

        //Constructors
        flat_set() : Comp(), m_storage() {}
        //Uses-allocator constructors depend on corresponding constructors in the base container
        //but these largely didn't exist in C++98. So we skip them.
        explicit flat_set(const container_type& cont, const key_compare& comp = Comp()) : Comp(comp), m_storage(cont) {
            std::sort(m_storage.begin(), m_storage.end(), get_comp());
            m_storage.erase(std::unique(m_storage.begin(), m_storage.end(), equal_from_comp(get_comp())), m_storage.end());
        }

        explicit flat_set(sorted_unique_t, const container_type& cont, const key_compare& comp = Comp()) : Comp(comp), m_storage(cont) {}

        explicit flat_set(const key_compare& comp) : Comp(comp), m_storage() {}

        template<typename InputIter>
        explicit flat_set(InputIter first, InputIter last, const key_compare& comp = Comp()) : Comp(comp), m_storage() {
            insert(first, last);
        }

        template<typename InputIter>
        explicit flat_set(sorted_unique_t, InputIter first, InputIter last, const key_compare& comp = Comp()) : Comp(comp), m_storage(first, last) {}


        //Iterators
        iterator begin() {
            return m_storage.begin();
        }
        const_iterator begin() const {
            return m_storage.begin();
        }
        const_iterator cbegin() const {
            return begin();
        }
        iterator end() {
            return m_storage.end();
        }
        const_iterator end() const {
            return m_storage.end();
        }
        const_iterator cend() const {
            return end();
        }
        reverse_iterator rbegin() {
            return m_storage.rbegin();
        }
        const_reverse_iterator rbegin() const {
            return m_storage.rbegin();
        }
        const_reverse_iterator crbegin() const {
            return rbegin();
        }
        reverse_iterator rend() {
            return m_storage.rend();
        }
        const_reverse_iterator rend() const {
            return m_storage.rend();
        }
        const_reverse_iterator crend() const {
            return rend();
        }

        //Queriers
        bool empty() const {
            return m_storage.empty();
        }
        std::size_t size() const {
            return m_storage.size();
        }
        std::size_t max_size() const {
            return m_storage.max_size();
        }

        //Insertion
        std::pair<iterator, bool> insert(const key_type& val) {
            //Find the place to insert the eleme
            iterator insertion_loc = lower_bound(val);
            //If the value is already in the set, return false
            if (insertion_loc != end() && equal_from_comp(get_comp())(*insertion_loc, val)) {
                return std::make_pair(insertion_loc, false);
            }
            //Otherwise, insert and go
            else {
                m_storage.insert(insertion_loc, val);
                return std::make_pair(insertion_loc, true);
            }

        }
        iterator insert(const_iterator iter, const key_type& val) {
            return m_storage.insert(iter, val);
        }
        template<typename InputIt>
        void insert(InputIt begin, InputIt end) {
            m_storage.insert(m_storage.end(), begin, end);
            std::sort(m_storage.begin(), m_storage.end(), get_comp());
            m_storage.erase(std::unique(m_storage.begin(), m_storage.end(), equal_from_comp(get_comp())), m_storage.end());

        }
        //The spec says that this is equivalent to the previous function. Odd.
        template<typename InputIt>
        void insert(sorted_unique_t, InputIt begin, InputIt end) {
            insert(begin, end);
        }

        //Extraction
        //No moves and no ref qualification, so this is an expensive function to call
        container_type extract() {
            container_type cont = m_storage;
            clear();
            return cont;
        }

        void clear() {
            m_storage.clear();
        }
        void replace(const container_type& cont) {
            m_storage = cont;
        }

        void swap(flat_set& other) {
            using std::swap;
            swap(m_storage, other.m_storage);
            swap(get_comp(), other.get_comp());
        }

        //I've not forgotten the const_iterator overloads
        //But vector didn't get them until C++11
        iterator erase(iterator pos) {
            return m_storage.erase(pos);
        }
        iterator erase(iterator begin, iterator end) {
            return m_storage.erase(begin, end);
        }
        iterator erase(const Key& val) {
            iterator pos = find(val);
            return m_storage.erase(val);
        }


        //Lookup
        iterator find(const key_type& val) {
            iterator lb = lower_bound(val);
            if (equal_from_comp(get_comp())(val, *lb)) return lb;
            return end();
        }
        const_iterator find(const key_type& val) const {
            iterator lb = lower_bound(val);
            if (equal_from_comp(get_comp())(val, *lb)) return lb;
            return end();
        }
        size_type count(const key_type& val) const {
            return std::count(m_storage.begin(), m_storage.end(), val, get_comp());
        }
        bool contains(const key_type& val) const {
            //This is a set so count can only be 1 or 0
            return count(val) == 1;
        }


        iterator lower_bound(const key_type& val) {
            return std::lower_bound(m_storage.begin(), m_storage.end(), val, get_comp());
        }
        const_iterator lower_bound(const key_type& val) const {
            return std::lower_bound(m_storage.begin(), m_storage.end(), val, get_comp());
        }
        iterator upper_bound(const key_type& val) {
            return std::upper_bound(m_storage.begin(), m_storage.end(), val, get_comp());
        }
        const_iterator upper_bound(const key_type& val) const {
            return std::upper_bound(m_storage.begin(), m_storage.end(), val, get_comp());
        }
        std::pair<iterator, iterator> equal_range(const key_type& val) {
            return std::equal_range(m_storage.begin(), m_storage.end(), val, get_comp());
        }
        std::pair<const_iterator, const_iterator> equal_bound(const key_type& val) const {
            return std::equal_range(m_storage.begin(), m_storage.end(), val, get_comp());
        }


        friend bool operator==(const flat_set& lhs, const flat_set& rhs) {
            return lhs.m_storage == rhs.m_storage;
        }
        friend bool operator!=(const flat_set& lhs, const flat_set& rhs) {
            return !(lhs == rhs);
        }
        friend bool operator<(const flat_set& lhs, const flat_set& rhs) {
            return lhs.m_storage < rhs.m_storage;
        }
        friend bool operator<=(const flat_set& lhs, const flat_set& rhs) {
            return (lhs < rhs) || (lhs == rhs);
        }
        friend bool operator>(const flat_set& lhs, const flat_set& rhs) {
            return !(lhs <= rhs);
        }
        friend bool operator>=(const flat_set& lhs, const flat_set& rhs) {
            return !(lhs < rhs);
        }


    };

    template<typename Key, typename Comp, typename Container>
    void swap(flat_set<Key, Comp, Container>& lhs, flat_set<Key, Comp, Container>& rhs) {
        lhs.swap(rhs);
    }

    template<typename Key, typename Comp, typename Container, typename Pred>
    void erase_if(flat_set<Key, Comp, Container>& lhs, Pred pred) {
        lhs.erase(std::remove_if(lhs.begin(), lhs.end(), pred), lhs.end());
    }



}


#endif