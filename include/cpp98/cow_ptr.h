#ifndef DP_CPP98_COW_PTR
#define DP_CPP98_COW_PTR

#include "cpp98/type_traits.h"

#include "bits/smart_ptr_bases.h"
#include "bits/static_assert_no_macro.h"

#include <algorithm>
#include <ostream>

namespace dp {


	/*
	*	A copy-on-write smart pointer.
	*	Like shared_ptr, this pointer offers shared ownership of a resource through reference counting, only deleting the original resource when the last pointer to it dies.
	*	Unlike shared_ptr, this pointer uses copy-on-write. When access to the underlying resource is requested in a non-const context, and if the pointer is not the only
	*   owner of the resource, it makes a copy of the resource. Pointers in use previous to this change remain unchanged and point to the same "immutable" resource, but 
	*   changes are reflected in any pointer that makes them (and any pointer spawned off of that pointer).
	*/
	template<typename StoredT>
	class cow_ptr {

		typedef typename dp::detail::shared_control_block_base BlockT;
		typedef typename dp::remove_extent<StoredT>::type stored_type;


		stored_type* m_ptr;
		BlockT* m_control;

		void make_copy() {
			//If we're not the only pointer using the resource
			if (m_control && !this->unique()) {
				BlockT* newBlock = m_control->clone();
				m_control->dec_shared();
				m_control = newBlock;
				m_ptr = static_cast<stored_type*>(m_control->get());
			}
		}

	public:

		typedef typename dp::remove_extent<StoredT>::type element_type;

		explicit cow_ptr() : m_ptr(NULL), m_control(NULL) {}

		template<typename U>
		explicit cow_ptr(U* in) : m_ptr(in), m_control(new dp::detail::shared_block_no_deleter<StoredT>(in)) {
			dp::static_assert_98<dp::detail::compatible_ptr_type<U, StoredT>::value>();
		}

		template<typename U, typename DelT>
		cow_ptr(U* in, DelT inDel) : m_ptr(in), m_control(new dp::detail::shared_block_with_deleter<StoredT, DelT>(in, inDel)) {
			dp::static_assert_98<dp::detail::compatible_ptr_type<U, StoredT>::value>();
		}

		template<typename U, typename DelT, typename Alloc>
		cow_ptr(U* in, DelT inDel, Alloc alloc) : m_ptr(in), m_control(new dp::detail::shared_block_with_allocator<StoredT, DelT, Alloc>(in, inDel, alloc)) {
			dp::static_assert_98<dp::detail::compatible_ptr_type<U, StoredT>::value>();
		}

		cow_ptr(const cow_ptr& inPtr) : m_ptr(inPtr.m_ptr), m_control(inPtr.m_control) {
			if (m_control) m_control->inc_shared();
		}

		template<typename U>
		cow_ptr(const dp::cow_ptr<U>& inPtr) : m_ptr(inPtr.m_ptr), m_control(inPtr.m_control) {
			dp::static_assert_98<dp::detail::compatible_ptr_type<U, StoredT>::value>();
			if (m_control) m_control->inc_shared();
		}

		~cow_ptr() {
			if (m_control) m_control->dec_shared();
		}

		cow_ptr& operator=(const cow_ptr& inPtr) {
			cow_ptr copy(inPtr);
			this->swap(copy);
			return *this;
		}

		void swap(cow_ptr& inPtr) {
			using std::swap;
			swap(m_ptr, inPtr.m_ptr);
			swap(m_control, inPtr.m_control);
		}

		void reset() {
			m_control->dec_shared();
		}

		void reset(StoredT* in) {
			BlockT* newBlock = new BlockT(in);

			m_control->dec_shared();
			m_ptr = in;
			m_control = newBlock;
		}

		const StoredT* get() const {
			return m_ptr;
		}
		StoredT* get() {
			make_copy();
			return m_ptr;
		}

		const StoredT& operator*() const {
			dp::static_assert_98<!dp::is_array<StoredT>::value>();
			return *get();
		}
		StoredT& operator*() {
			dp::static_assert_98<!dp::is_array<StoredT>::value>();
			return *get();
		}

		const StoredT* operator->() const {
			dp::static_assert_98<!dp::is_array<StoredT>::value>();
			return get();
		}
		StoredT* operator->() {
			dp::static_assert_98<!dp::is_array<StoredT>::value>();
			return get();
		}

		const element_type& operator[](std::size_t index) const {
			dp::static_assert_98<dp::is_array<StoredT>::value>();
			return get()[index];
		}
		element_type& operator[](std::size_t index) {
			dp::static_assert_98<dp::is_array<StoredT>::value>();
			return get()[index];
		}

		std::size_t use_count() const {
			return m_control->shared_count;
		}

		bool unique() const {
			return use_count() == 1;
		}

		operator bool() const {
			return get() != NULL;
		}

		template<typename U>
		bool owner_before(const dp::cow_ptr<U>& inPtr) const {
			return get() < inPtr.get();
		}
	};

	template<typename StoredT>
	void swap(dp::cow_ptr<StoredT>& lhs, dp::cow_ptr<StoredT>& rhs){
		lhs.swap(rhs);
	}

	template<typename T>
	typename dp::enable_if<!dp::is_array<T>::value, dp::cow_ptr<T> >::type make_cow() {
		return dp::cow_ptr<T>(new T);
	}
	template<typename T, typename U>
	typename dp::enable_if<!dp::is_array<T>::value, dp::cow_ptr<T> >::type make_cow(const U& in) {
		return dp::cow_ptr<T>(new T(in));
	}
	template<typename T, typename U, typename V>
	typename dp::enable_if<!dp::is_array<T>::value, dp::cow_ptr<T> >::type make_cow(const U& inU, const V& inV) {
		return dp::cow_ptr<T>(new T(inU, inV));
	}
	template<typename T, typename U, typename V, typename W>
	typename dp::enable_if<!dp::is_array<T>::value, dp::cow_ptr<T> >::type make_cow(const U& inU, const V& inV, const W& inW) {
		return dp::cow_ptr<T>(new T(inU, inV, inW));
	}
	template<typename T, typename U, typename V, typename W, typename X>
	typename dp::enable_if<!dp::is_array<T>::value, dp::cow_ptr<T> >::type make_cow(const U& inU, const V& inV, const W& inW, const X& inX) {
		return dp::cow_ptr<T>(new T(inU, inV, inW, inX));
	}

	template<typename T>
	typename dp::enable_if<dp::is_unbounded_array<T>::value, dp::cow_ptr<T> >::type make_cow(std::size_t N) {
		typedef typename dp::remove_extent<T>::type elemT;
		return dp::cow_ptr<T>(new elemT[N]);
	}

	template<typename T>
	typename dp::enable_if<dp::is_bounded_array<T>::value, dp::cow_ptr<T> >::type make_cow() {
		typedef typename dp::remove_extent<T>::type elemT;
		return dp::cow_ptr<T>(new elemT[dp::extent<T>::value]);
	}

	template<typename T>
	typename dp::enable_if<dp::is_unbounded_array<T>::value, dp::cow_ptr<T> >::type make_cow(std::size_t N, const typename dp::remove_extent<T>::type& u) {
		typedef typename dp::remove_extent<T>::type elemT;
		dp::cow_ptr<T> temp(new elemT[N]);
		for (std::size_t i = 0; i < N; ++i) temp[i] = u;
		return temp;
	}

	template<typename T>
	typename dp::enable_if<dp::is_bounded_array<T>::value, dp::cow_ptr<T> >::type make_cow(const typename dp::remove_extent<T>::type& u) {
		typedef typename dp::remove_extent<T>::type elemT;
		dp::cow_ptr<T> temp(new elemT[dp::extent<T>::value]);
		for (std::size_t i = 0; i < dp::extent<T>::value; ++i) temp[i] = u;
		return temp;
	}

	template<typename T, typename U>
	bool operator==(const dp::cow_ptr<T>& lhs, const dp::cow_ptr<U>& rhs) {
		return lhs.get() == rhs.get();
	}
	template<typename T, typename U>
	bool operator!=(const dp::cow_ptr<T>& lhs, const dp::cow_ptr<U>& rhs) {
		return !(lhs == rhs);
	}
	template<typename T, typename U>
	bool operator<(const dp::cow_ptr<T>& lhs, const dp::cow_ptr<U>& rhs) {
		return lhs.get() < rhs.get();
	}
	template<typename T, typename U>
	bool operator>(const dp::cow_ptr<T>& lhs, const dp::cow_ptr<U>& rhs) {
		return rhs < lhs;
	}
	template<typename T, typename U>
	bool operator>=(const dp::cow_ptr<T>& lhs, const dp::cow_ptr<U>& rhs) {
		return rhs < lhs || rhs == lhs;
	}
	template<typename T, typename U>
	bool operator<=(const dp::cow_ptr<T>& lhs, const dp::cow_ptr<U>& rhs) {
		return lhs < rhs || lhs == rhs;
	}

	template<typename CharT, typename Traits, typename StoredT>
	std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const dp::cow_ptr<StoredT>& rhs) {
		os << rhs.get();
		return os;
	}
}



#endif
