#ifndef DP_CPP98_SHARED_PTR
#define DP_CPP98_SHARED_PTR

#include <cctype>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <typeinfo>
#include <ostream>
#include "bits/smart_ptr_bases.h"
#include "bits/static_assert_no_macro.h"
#include "cpp98/type_traits.h"
#include "bits/version_defs.h"


namespace dp {
	

	//Forward decs
	template<typename StoredT>
	class weak_ptr;

	template<typename StoredT, typename Deleter>
	class scoped_ptr;

	template<typename StoredT, typename Deleter>
	class lite_ptr;

	template<typename T>
	class shared_ptr;

	struct bad_weak_ptr;

	template<typename T>
	class enable_shared_from_this;

	namespace detail {
#ifndef DP_BORLAND
		template<typename U, typename Ptr, bool b = dp::is_base_of<dp::enable_shared_from_this<U>, U>::value>
		struct enable_from_this_check {
			void operator()(U*, const dp::shared_ptr<Ptr>&) {}
		};
		template<typename U, typename Ptr>
		struct enable_from_this_check<U, Ptr, true> {
			void operator()(U* inPtr, const dp::shared_ptr<Ptr>& inBase) {
				if (inPtr && inPtr->weak_this.expired()) {
					inPtr->weak_this = dp::shared_ptr<typename dp::remove_cv<U>::type>(inBase, const_cast<typename dp::remove_cv<U>::type*>(inPtr));
				}

			}
		};
#else
		//Again, borland ties our hands. enable_shared_from_this is effectively dead in Borland land
		template<typename T,typename U>
		struct enable_from_this_check {
			void operator()(T*, const dp::shared_ptr<U>&) {}
		};
#endif
	}

	//C++ does not require this declaration
	//Borland does, and they have the audacity to put QUESTIONABLE!!! in the error message if you omit it.
    template<typename T, typename Deleter>
    Deleter* get_deleter(const dp::shared_ptr<T>& inPtr);


	//We don't need a common base on this one, as the standard does not specify that a operator[] must only be present on array type shared pointers.
	//We will assert it statically to minimise UB.
	template<typename StoredT>
	class shared_ptr : public dp::detail::shared_ownership_base<StoredT> {
	public:
		typedef typename dp::remove_extent<StoredT>::type	element_type;

	private:

		typedef StoredT stored_type;   //Two-phase lookup fixer
		typedef typename dp::detail::shared_ownership_base<StoredT> Base;
		

		template<typename U>
		friend class shared_ptr;

		template<typename U>
		friend class weak_ptr;

		template<typename Deleter, typename T>
		friend Deleter* get_deleter(const dp::shared_ptr<T>& inPtr);

		template<typename DelT>
		DelT* get_deleter() const {
			return static_cast<DelT*>(Base::m_control->get_deleter(typeid(DelT)));
		}

	public:

		typedef typename dp::weak_ptr<stored_type>				weak_type;

		//No using Base::Base in C++98 :(
		shared_ptr() : Base() {}

		template<typename U>
		explicit shared_ptr(U* inPtr) : Base(inPtr) {
			detail::enable_from_this_check<U,stored_type>()(inPtr,*this);
		}

		template<typename U, typename Deleter>
		shared_ptr(U* inPtr, Deleter inDel) : Base(inPtr, inDel) {
			detail::enable_from_this_check<U,stored_type>()(inPtr,*this);
		}

		template<typename U, typename Deleter, typename Alloc>
		shared_ptr(U* inPtr, Deleter inDel, Alloc inAlloc) : Base(inPtr, inDel, inAlloc) {
			detail::enable_from_this_check<U,stored_type>()(inPtr,*this);
		}

		//Aliasing ctor
		template<typename U>
		shared_ptr(const shared_ptr<U>& inPtr, element_type* r) : Base(inPtr, r) {}

		//Copy ctor
		shared_ptr(const shared_ptr& inPtr) : Base(inPtr) {}

		template<typename U>
		shared_ptr(const shared_ptr<U>& inPtr) : Base(inPtr) {}

		template<typename U>
		shared_ptr(const dp::weak_ptr<U>& inPtr) {
			dp::static_assert_98<dp::detail::compatible_ptr_type<U, stored_type>::value>();
			if (inPtr.expired()) throw dp::bad_weak_ptr();
			Base::m_ptr = inPtr.m_ptr;
			Base::m_control = inPtr.m_control;
			if (Base::m_control) Base::m_control->inc_shared();
		}

#ifndef DP_CPP17_OR_HIGHER
		template<typename U>
		shared_ptr(std::auto_ptr<U>& inPtr) : Base(inPtr) {}
#endif
		template<typename U, typename Deleter>
		shared_ptr(dp::scoped_ptr<U, Deleter>& inPtr) : Base(inPtr) {}

		template<typename U, typename Deleter>
		shared_ptr(dp::lite_ptr<U, Deleter>& inPtr) : Base(inPtr) {}

		using Base::operator=;

		shared_ptr& operator=(const shared_ptr& other) {
			shared_ptr copy(other);
			this->swap(copy);
			return *this;
		}
		

		void swap(shared_ptr& in) {
			this->Base::swap(in);
		}
		

		element_type* get() const {
			return Base::m_ptr;
		}

		//Accessors, filtering for array-ness
		stored_type& operator*() const {
			dp::static_assert_98<!dp::is_array<stored_type>::value>();
			return *get();
		}
		stored_type* operator->() const {
			dp::static_assert_98<!dp::is_array<stored_type>::value>();
			return get();
		}
		element_type& operator[](std::ptrdiff_t index) const {
			dp::static_assert_98<dp::is_array<stored_type>::value>();
			return get()[index];
		}

		
		template<typename U>
		bool owner_before(const shared_ptr<U>& inPtr) {
			return Base::m_control < inPtr.m_control;
		}

		template<typename U>
		bool owner_before(const dp::weak_ptr<U>& inPtr) {
			return Base::m_control < inPtr.m_control;
		}
	};


	template<typename StoredT>
	class weak_ptr {

		dp::detail::shared_control_block_base* m_control;
		StoredT* m_ptr;	//Non-owning, but required to make sure everything behaves if we go around in a circle of conversions


		template<typename U>
		friend class weak_ptr;

		template<typename U>
		friend class shared_ptr;

	public:

		typedef typename dp::remove_extent<StoredT>::type element_type;

		weak_ptr() : m_control(NULL), m_ptr(NULL) {}

		weak_ptr(const weak_ptr& inPtr) : m_control(inPtr.m_control), m_ptr(inPtr.m_ptr) {
			m_control->inc_weak();
		}

		template<typename U>
		weak_ptr(const weak_ptr<U>& inPtr) : m_control(inPtr.m_control), m_ptr(inPtr.m_ptr) {
			dp::static_assert_98<dp::detail::compatible_ptr_type<U, StoredT>::value>();
			m_control->inc_weak();
		}

		template<typename U>
		weak_ptr(const dp::shared_ptr<U>& inPtr) : m_control(inPtr.m_control), m_ptr(inPtr.get()) {
			dp::static_assert_98<dp::detail::compatible_ptr_type<U, StoredT>::value>();
			m_control->inc_weak();
		}

		~weak_ptr() {
			if (m_control) m_control->dec_weak();
		}

		weak_ptr& operator=(const weak_ptr& inPtr) {
			weak_ptr copy(inPtr);
			this->swap(copy);
			return *this;
		}

		template<typename U>
		typename dp::enable_if<dp::detail::compatible_ptr_type<U, StoredT>::value, weak_ptr&>::type operator=(const weak_ptr<U>& inPtr) {
			weak_ptr copy(inPtr);
			this->swap(copy);
			return *this;
		}

		template<typename U>
		typename dp::enable_if<dp::detail::compatible_ptr_type<U, StoredT>::value, weak_ptr&>::type operator=(const dp::shared_ptr<U>& inPtr) {
			weak_ptr copy(inPtr);
			this->swap(copy);
			return *this;
		}

		void reset() {
			m_control->dec_weak();
			m_control = NULL;
			m_ptr = NULL;
		}

		void swap(weak_ptr& inPtr) {
			using std::swap;
			swap(m_ptr, inPtr.m_ptr);
			swap(m_control, inPtr.m_control);
		}

		std::size_t use_count() const {
			return m_control ? m_control->shared_count : 0;
		}

		bool expired() const {
			return use_count() == 0;
		}

		dp::shared_ptr<StoredT> lock() const {
			return expired() ? dp::shared_ptr<StoredT>() : shared_ptr<StoredT>(*this);
		}

		template<typename U>
		bool owner_before(const weak_ptr<U>& other) const {
			return m_control < other.m_control;
		}
		template<typename U>
		bool owner_before(const dp::shared_ptr<U>& other) const {
			return m_control < other.m_control;
		}


	};

	/*
	*	PREVENTATIVE NON-DEFINITIONS
	*/
	template<typename T>
	class shared_ptr<T&>;

	template<typename T>
	class weak_ptr<T&>;

	/*
	*  HELPER CLASSES AND FUNCTIONS
	*/

	struct bad_weak_ptr : std::exception 
#ifdef DP_BORLAND_EXCEPTIONS
		, System::Sysutils::Exception{
		bad_weak_ptr() : System::Sysutils::Exception(L"Bad weak ptr") {}
#else
		{
#endif
		virtual const char* what() const throw() {
			return "Bad weak ptr";
		}

	};


	/*
	*  Ordinarily I don't bother with functions which require variadic templates, as it's only possible to recreate a finite amount of the possible sets,
	*  and so my interface breaks with the standard.
	*  make_shared is important enough to be an exception.
	*/
	template<typename T>
	typename dp::enable_if<!dp::is_array<T>::value, dp::shared_ptr<T> >::type make_shared() {
		return dp::shared_ptr<T>(new T);
	}
	template<typename T, typename U>
	typename dp::enable_if<!dp::is_array<T>::value, dp::shared_ptr<T> >::type make_shared(const U& in) {
		return dp::shared_ptr<T>(new T(in));
	}
	template<typename T, typename U, typename V>
	typename dp::enable_if<!dp::is_array<T>::value, dp::shared_ptr<T> >::type make_shared(const U& inU, const V& inV) {
		return dp::shared_ptr<T>(new T(inU, inV));
	}
	template<typename T, typename U, typename V, typename W>
	typename dp::enable_if<!dp::is_array<T>::value, dp::shared_ptr<T> >::type make_shared(const U& inU, const V& inV, const W& inW) {
		return dp::shared_ptr<T>(new T(inU, inV, inW));
	}
	template<typename T, typename U, typename V, typename W, typename X>
	typename dp::enable_if<!dp::is_array<T>::value, dp::shared_ptr<T> >::type make_shared(const U& inU, const V& inV, const W& inW, const X& inX) {
		return dp::shared_ptr<T>(new T(inU, inV, inW, inX));
	}

	//More may be added as needed.

	template<typename T>
	typename dp::enable_if<dp::is_unbounded_array<T>::value, dp::shared_ptr<T> >::type make_shared(std::size_t N) {
		typedef typename dp::remove_extent<T>::type elemT;
		return dp::shared_ptr<T>(new elemT[N]);
	}

	template<typename T>
	typename dp::enable_if<dp::is_bounded_array<T>::value, dp::shared_ptr<T> >::type make_shared() {
		typedef typename dp::remove_extent<T>::type elemT;
		return dp::shared_ptr<T>(new elemT[dp::extent<T>::value]);
	}

	//Not strictly initializing in order but I'm not a compiler maker.
	template<typename T>
	typename dp::enable_if<dp::is_unbounded_array<T>::value, dp::shared_ptr<T> >::type make_shared(std::size_t N, const typename dp::remove_extent<T>::type& u) {
		typedef typename dp::remove_extent<T>::type elemT;
		dp::shared_ptr<T> temp(new elemT[N]);
		for (std::size_t i = 0; i < N; ++i) temp[i] = u;
		return temp;
	}

	template<typename T>
	typename dp::enable_if<dp::is_bounded_array<T>::value, dp::shared_ptr<T> >::type make_shared(const typename dp::remove_extent<T>::type& u) {
		typedef typename dp::remove_extent<T>::type elemT;
		dp::shared_ptr<T> temp(new elemT[dp::extent<T>::value]);
		for (std::size_t i = 0; i < dp::extent<T>::value; ++i) temp[i] = u;
		return temp;
	}




	template<typename T, typename Alloc>
	typename dp::enable_if<!dp::is_array<T>::value, dp::shared_ptr<T> >::type allocate_shared(const Alloc& alloc) {
		return dp::shared_ptr<T>(new T, dp::default_delete<T>(), alloc);
	}
	template<typename T, typename Alloc, typename U>
	typename dp::enable_if<!dp::is_array<T>::value, dp::shared_ptr<T> >::type allocate_shared(const Alloc& alloc, const U& inU) {
		return dp::shared_ptr<T>(new T(inU), dp::default_delete<T>(), alloc);
	}
	template<typename T, typename Alloc, typename U, typename V>
	typename dp::enable_if<!dp::is_array<T>::value, dp::shared_ptr<T> >::type allocate_shared(const Alloc& alloc, const U& inU, const V& inV) {
		return dp::shared_ptr<T>(new T(inU, inV), dp::default_delete<T>(), alloc);
	}
	template<typename T, typename Alloc, typename U, typename V, typename W>
	typename dp::enable_if<!dp::is_array<T>::value, dp::shared_ptr<T> >::type allocate_shared(const Alloc& alloc, const U& inU, const V& inV, const W& inW) {
		return dp::shared_ptr<T>(new T(inU, inV, inW), dp::default_delete<T>(), alloc);
	}
	template<typename T, typename Alloc, typename U, typename V, typename W, typename X>
	typename dp::enable_if<!dp::is_array<T>::value, dp::shared_ptr<T> >::type allocate_shared(const Alloc& alloc, const U& inU, const V& inV, const W& inW, const X& inX) {
		return dp::shared_ptr<T>(new T(inU, inV, inW, inX), dp::default_delete<T>(), alloc);
	}

	template<typename T, typename Alloc>
	typename dp::enable_if<dp::is_unbounded_array<T>::value, dp::shared_ptr<T> >::type allocate_shared(const Alloc& alloc, std::size_t N) {
		typedef typename dp::remove_extent<T>::type elemT;
		return dp::shared_ptr<T>(new elemT[N], dp::default_delete<T>(), alloc);
	}

	template<typename T, typename Alloc>
	typename dp::enable_if<dp::is_bounded_array<T>::value, dp::shared_ptr<T> >::type allocate_shared(const Alloc& alloc) {
		typedef typename dp::remove_extent<T>::type elemT;
		return dp::shared_ptr<T>(new elemT[dp::extent<T>::value], dp::default_delete<T>(), alloc);
	}

	template<typename T, typename Alloc>
	typename dp::enable_if<dp::is_unbounded_array<T>::value, dp::shared_ptr<T> >::type allocate_shared(const Alloc& alloc, std::size_t N, const typename dp::remove_extent<T>::type& u) {
		typedef typename dp::remove_extent<T>::type elemT;
		dp::shared_ptr<T> temp(new elemT[N], dp::default_delete<T>(), alloc);
		for (std::size_t i = 0; i < N; ++i) temp[i] = u;
		return temp;
	}

	template<typename T, typename Alloc>
	typename dp::enable_if<dp::is_bounded_array<T>::value, dp::shared_ptr<T> >::type allocate_shared(const Alloc& alloc, const typename dp::remove_extent<T>::type& u) {
		typedef typename dp::remove_extent<T>::type elemT;
		dp::shared_ptr<T> temp(new elemT[dp::extent<T>::value], dp::default_delete<T>(), alloc);
		for (std::size_t i = 0; i < dp::extent<T>::value; ++i) temp[i] = u;
		return temp;
	}

	template<typename T, typename U>
	dp::shared_ptr<T> static_pointer_cast(const dp::shared_ptr<U>& inPtr) {
		typedef typename dp::shared_ptr<T>::element_type ResT;
		return dp::shared_ptr<T>(inPtr, static_cast<ResT>(inPtr.get()));
	}
	template<typename T, typename U>
	dp::shared_ptr<T> dynamic_pointer_cast(const dp::shared_ptr<U>& inPtr) {
		typedef typename dp::shared_ptr<T>::element_type ResT;
		return dp::shared_ptr<T>(inPtr, dynamic_cast<ResT>(inPtr.get()));
	}
	template<typename T, typename U>
	dp::shared_ptr<T> const_pointer_cast(const dp::shared_ptr<U>& inPtr) {
		typedef typename dp::shared_ptr<T>::element_type ResT;
		return dp::shared_ptr<T>(inPtr, const_cast<ResT>(inPtr.get()));
	}
	template<typename T, typename U>
	dp::shared_ptr<T> reinterpret_pointer_cast(const dp::shared_ptr<U>& inPtr) {
		typedef typename dp::shared_ptr<T>::element_type ResT;
		return dp::shared_ptr<T>(inPtr, reinterpret_cast<ResT>(inPtr.get()));
	}

	template<typename Deleter, typename T>
	Deleter* get_deleter(const dp::shared_ptr<T>& inPtr) {
		return inPtr.template get_deleter<Deleter>();
	}

	template<typename T, typename U>
	bool operator==(const dp::shared_ptr<T>& lhs, const dp::shared_ptr<U>& rhs) {
		return lhs.get() == rhs.get();
	}
	template<typename T, typename U>
	bool operator!=(const dp::shared_ptr<T>& lhs, const dp::shared_ptr<U>& rhs) {
		return !(lhs == rhs);
	}
	template<typename T, typename U>
	bool operator<(const dp::shared_ptr<T>& lhs, const dp::shared_ptr<U>& rhs) {
		return lhs.get() < rhs.get();
	}
	template<typename T, typename U>
	bool operator>(const dp::shared_ptr<T>& lhs, const dp::shared_ptr<U>& rhs) {
		return rhs < lhs;
	}
	template<typename T, typename U>
	bool operator>=(const dp::shared_ptr<T>& lhs, const dp::shared_ptr<U>& rhs) {
		return rhs < lhs || rhs == lhs;
	}
	template<typename T, typename U>
	bool operator<=(const dp::shared_ptr<T>& lhs, const dp::shared_ptr<U>& rhs) {
		return lhs < rhs || lhs == rhs;
	}

	template<typename CharT, typename Traits, typename StoredT>
	std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const dp::shared_ptr<StoredT>& rhs) {
		os << rhs.get();
		return os;
	}

	template<typename T>
	void swap(dp::shared_ptr<T>& lhs, dp::shared_ptr<T>& rhs) {
		lhs.swap(rhs);
	}
	template<typename T>
	void swap(dp::weak_ptr<T>& lhs, dp::weak_ptr<T>& rhs) {
		lhs.swap(rhs);
	}

	/*
	*  SUPPORT OBJECTS
	*/
	template<typename T = void>
	struct owner_less;

	template<>
	struct owner_less<void> {
		template<typename T>
		bool operator()(const dp::weak_ptr<T>& lhs, const dp::shared_ptr<T>& rhs) const {
			return lhs.owner_before(rhs);
		}
		template<typename T>
		bool operator()(const dp::shared_ptr<T>& lhs, const dp::weak_ptr<T>& rhs) const {
			return lhs.owner_before(rhs);
		}
	};

	template<typename T>
	struct owner_less<dp::shared_ptr<T> > : dp::owner_less<void> {
		bool operator()(const dp::shared_ptr<T>& lhs, const dp::shared_ptr<T>& rhs) {
			return lhs.owner_before(rhs);
		}
	};
	template<typename T>
	struct owner_less<dp::weak_ptr<T> > : dp::owner_less<void> {
		bool operator()(const dp::weak_ptr<T>& lhs, const dp::weak_ptr<T>& rhs) {
			return lhs.owner_before(rhs);
		}
	};


#ifndef DP_BORLAND
	template<typename T>
	class enable_shared_from_this {
	protected:


	public:
		dp::weak_ptr<T> weak_this;

		enable_shared_from_this() {}
		enable_shared_from_this(const enable_shared_from_this& other) : weak_this(other.weak_this) {}

		dp::shared_ptr<T> shared_from_this() {
			return dp::shared_ptr<T>(weak_this);
		}
		dp::shared_ptr<const T> shared_from_this() const {
			return dp::shared_ptr<T>(weak_this);
		}
		dp::weak_ptr<T> weak_from_this() {
			return weak_this;
		}
		dp::weak_ptr<const T> weak_from_this() const {
			return weak_this;
		}
	};
#endif









}




#endif