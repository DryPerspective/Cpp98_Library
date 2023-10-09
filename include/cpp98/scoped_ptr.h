#ifndef DP_CPP98_SCOPED_PTR
#define DP_CPP98_SCOPED_PTR

#include "bits/version_defs.h"

#include <cstddef>
#include <algorithm>
#include <ostream>

#ifndef DP_CPP17_OR_HIGHER
#include <memory>
#endif


#include "cpp98/type_traits.h"
#include "bits/smart_ptr_bases.h"
#include "bits/static_assert_no_macro.h"

/*
*   A very simple scope-local smart pointer class for C++03. It is in no way as sophisticated as
*   the C++11 smart pointers like unique_ptr and shared_ptr, and should be replaced by std::unique_ptr when the opportunity arises.
*   It does however have the benefit of not having some of the quirks of std::auto_ptr.
*
*   Specifically - this is a non-copyable pointer. Copying is disabled and will lead to UB if you try to force it to happen.
*   While this does significantly limit the scope of possible use-cases (e.g. easy passing around, polymorphism), it side-steps
*   the issue with auto_ptr and pre-11 C++'s lack of defined move semantics.
*
*   It is safe enough for its common enough uses, but it is not safe in a multithreaded environment or some other features.
*   As always, understand the implications of what you're doing before you do it.
*   Another limitation is that its member and factory functions can only initialize data with 0 or 1 constructor arguments.
*   This is simply because we dont have variadic templates at our disposal and handwriting every permutation is silly.
*/

namespace dp{

	namespace detail {
		//Since the majority of both versions of scoped_ptr are identical,
		//we DRY off our code with a common base
		template<typename T, typename Deleter>
		class scoped_ptr_base : dp::detail::deleter_holder<T, Deleter> {
		protected:
			T* m_data;

			//We explicitly forbid copying.
			scoped_ptr_base(const scoped_ptr_base&);
			scoped_ptr_base& operator=(const scoped_ptr_base&);

			typedef const T* const_pointer;

			~scoped_ptr_base() { this->reset(); }

		public:

			typedef T       element_type;
			typedef T* pointer;
			typedef Deleter deleter_type;

			explicit scoped_ptr_base(T* in = NULL) : m_data(in) {}

			explicit scoped_ptr_base(pointer in, deleter_type del_inst) : detail::deleter_holder<T, Deleter>(del_inst), m_data(in) {}

			template<typename ValT, typename DelT>
			explicit scoped_ptr_base(scoped_ptr_base<ValT, DelT>& inPtr) : detail::deleter_holder<T, Deleter>(inPtr.get_deleter()), m_data(inPtr.release()) {}

#ifndef DP_CPP17_OR_HIGHER
			explicit scoped_ptr_base(std::auto_ptr<T>& in) : m_data(in.release()) {}
#endif


			template<typename ValT, typename DelT>
			scoped_ptr_base& operator=(scoped_ptr_base<ValT, DelT>& inPtr) {
				scoped_ptr_base copy(inPtr);
				this->swap(copy);
				return *this;
			}



			const T* get() const { return m_data; }
			T* get() { return m_data; }

			Deleter& get_deleter() { return this->get_deleter_impl(); }
			const Deleter& get_deleter() const { return this->get_deleter_impl(); }

			void swap(scoped_ptr_base& other) {
				using std::swap;
				swap(m_data, other.m_data);
			}

			//Release ownership of the resource and return the raw pointer to it.
			pointer release() {
				T* temp = m_data;
				m_data = NULL;
				return temp;
			}

			//Replace the resource we currently own with a new one
			//Or just delete the resource we have
			void reset(pointer in = NULL) {
				if (m_data != in) {
					this->delete_resource(m_data);
					m_data = in;
				}
			}

			operator bool() const {
				return m_data;
			}

		};
	}



	/*
	* Bits/smart_ptr_bases.h contains the shared functionality/base classes of all our smart pointer classes, placed there because both this and future smart pointers
	* May need stateful deleters.
	* And now all we need is to add our specific class instances with their specific interfaces
	* Plus whatever C++98 requires of us
	*/
	template<typename T, typename Deleter = dp::default_delete<T> >
	class scoped_ptr : public dp::detail::scoped_ptr_base<T, Deleter> {

		typedef dp::detail::scoped_ptr_base<T, Deleter> Base;

	public:

		//Alas no inheriting constructors in C++98
		explicit scoped_ptr(T* in = NULL) : Base(in) {}

		explicit scoped_ptr(T* in, Deleter del_inst) : Base(in, del_inst) {}

		template<typename ValT, typename DelT>
		explicit scoped_ptr(scoped_ptr<ValT, DelT>& inPtr) : Base(inPtr) {}

#ifndef DP_CPP17_OR_HIGHER
		explicit scoped_ptr(std::auto_ptr<T>& in) : Base(in) {}
#endif

		using Base::operator=;

		const T& operator*() const { return *Base::m_data; }
		T& operator*() { return *Base::m_data; }
		const T* operator->() const { return Base::m_data; }
		T* operator->() { return Base::m_data; }

	};




	/*
	*   ARRAY VERSION
	*   Don't mix and match array and non-array pointers, dumb or smart. Calling the wrong delete variant will cause UB.
	*   There's no way for me to enforce that in this interface, so just don't be an idiot.
	*/

	template<typename T, typename Deleter>
	class scoped_ptr<T[], Deleter> : public dp::detail::scoped_ptr_base<T, Deleter> {

		typedef dp::detail::scoped_ptr_base<T, Deleter> Base;

	public:

		//Alas no inheriting constructors in C++98
		explicit scoped_ptr(T* in = NULL) : Base(in) {}

		explicit scoped_ptr(T* in, Deleter del_inst) : Base(in, del_inst) {}

		template<typename ValT, typename DelT>
		explicit scoped_ptr(scoped_ptr<ValT, DelT>& inPtr) : Base(inPtr) {}

#ifndef DP_CPP17_OR_HIGHER
		explicit scoped_ptr(std::auto_ptr<T>& in) : Base(in) {}
#endif

		using Base::operator=;

		//Rather than dereference and pointer access operators, we provide array access
		const T*& operator[](std::size_t N) const {
			return Base::m_data[N];
		}
		T& operator[](std::size_t N) {
			return Base::m_data[N];
		}
	};

	/*
	*  DISABLER NON-DEFINITIONS
	*/
	template<typename T>
	class scoped_ptr<T&>;

	template<typename T, typename Del>
	class scoped_ptr<T, Del&>;

	template<typename T, typename Del>
	class scoped_ptr<T&, Del&>;


	/*
	* UTILITY FUNCTIONS
	*/
	template<typename T, typename Deleter>
	void swap(dp::scoped_ptr<T, Deleter>& lhs, dp::scoped_ptr<T, Deleter>& rhs) {
		lhs.swap(rhs);
	}
	template<typename CharT, typename Traits, typename T, typename Deleter>
	std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const dp::scoped_ptr<T, Deleter>& ptr) {
		os << ptr.get();
		return os;
	}

	/*
	* COMPARISON OPERATORS
	*/
	template<typename T1, typename D1, typename T2, typename D2>
	bool operator==(const dp::scoped_ptr<T1, D1>& lhs, const dp::scoped_ptr<T2, D2>& rhs) {
		return lhs.get() == rhs.get();
	}
	template<typename T1, typename D1, typename T2, typename D2>
	bool operator!=(const dp::scoped_ptr<T1, D1>& lhs, const dp::scoped_ptr<T2, D2>& rhs) {
		return !(lhs == rhs);
	}
	template<typename T1, typename D1, typename T2, typename D2>
	bool operator<(const dp::scoped_ptr<T1, D1>& lhs, const dp::scoped_ptr<T2, D2>& rhs) {
		return lhs.get() < rhs.get();
	}
	template<typename T1, typename D1, typename T2, typename D2>
	bool operator<=(const dp::scoped_ptr<T1, D1>& lhs, const dp::scoped_ptr<T2, D2>& rhs) {
		return !(rhs < lhs);
	}
	template<typename T1, typename D1, typename T2, typename D2>
	bool operator>(const dp::scoped_ptr<T1, D1>& lhs, const dp::scoped_ptr<T2, D2>& rhs) {
		return rhs < lhs;
	}
	template<typename T1, typename D1, typename T2, typename D2>
	bool operator>=(const dp::scoped_ptr<T1, D1>& lhs, const dp::scoped_ptr<T2, D2>& rhs) {
		return !(lhs < rhs);
	}

	namespace detail {
		//Borland has trouble with identifying functions.
		//So instead we provide a slightly less bulletproof options for them.
		template<typename Deleter>
		struct assert_not_fptr {
#ifndef DP_BORLAND			
			typename dp::static_assert_98<!dp::is_function<typename dp::remove_pointer<Deleter>::type>::value> assertion;
#else
			typename dp::static_assert_98<!dp::is_pointer<Deleter>::value> assertion;
#endif
		};
	}


	/*
	*  LITE POINTER
	*  For cases where you don't get EBO and sizeof(smart_pointer<T> == sizeof(T*) must hold
	*  WET but we can't inherit or compose
	*/
	template<typename T, typename Deleter = dp::default_delete<T> >
	class lite_ptr {
		T* m_data;

		//We explicitly forbid copying.
		lite_ptr(const lite_ptr&);
		lite_ptr& operator=(const lite_ptr&);

		typedef const T* const_pointer;

	public:
		typedef T  element_type;
		typedef T* pointer;


		explicit lite_ptr(T* in = NULL) : m_data(in) {
			detail::assert_not_fptr<Deleter>();			
		}


#ifndef DP_CPP17_OR_HIGHER
		explicit lite_ptr(std::auto_ptr<T>& in) : m_data(in.release()) {
			detail::assert_not_fptr<Deleter>();
		}
#endif

		~lite_ptr() {
			this->reset();
		}


		const T* get() const { return m_data; }
		T* get() { return m_data; }

		Deleter get_deleter() const {
			return Deleter();
		}

		void swap(lite_ptr& other) {
			using std::swap;
			swap(m_data, other.m_data);
		}

		//Release ownership of the resource and return the raw pointer to it.
		pointer release() {
			T* temp = m_data;
			m_data = NULL;
			return temp;
		}

		const T& operator*() const { 
			dp::static_assert_98<!dp::is_array<T>::value>(); 
			return *m_data; 
		}
		T& operator*() {
			dp::static_assert_98<!dp::is_array<T>::value>();
			return *m_data;
		}
		const T* operator->() const {
			dp::static_assert_98<!dp::is_array<T>::value>();
			return m_data;
		}
		T* operator->() {
			dp::static_assert_98<!dp::is_array<T>::value>();
			return *m_data;
		}

		//Rather than dereference and pointer access operators, we provide array access
		const T*& operator[](std::size_t N) const {
			dp::static_assert_98<dp::is_array<T>::value>();
			return m_data[N];
		}
		T& operator[](std::size_t N) {
			dp::static_assert_98<dp::is_array<T>::value>();
			return m_data[N];
		}

		//Replace the resource we currently own with a new one
		//Or just delete the resource we have
		void reset(pointer in = NULL) {
			if (m_data != in) {
				Deleter()(m_data);
				m_data = in;
			}
		}

		operator bool() const {
			return m_data;
		}

	};

	/*
	*  DISABLER NON-DEFINITIONS
	*/
	template<typename T>
	class lite_ptr<T&>;

	template<typename T, typename Del>
	class lite_ptr<T, Del&>;

	template<typename T, typename Del>
	class lite_ptr<T&, Del&>;
	

	/*
	* UTILITY FUNCTIONS
	*/
	template<typename T, typename Deleter>
	void swap(dp::lite_ptr<T, Deleter>& lhs, dp::lite_ptr<T, Deleter>& rhs) {
		lhs.swap(rhs);
	}
	template<typename CharT, typename Traits, typename T, typename Deleter>
	std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const dp::lite_ptr<T, Deleter>& ptr) {
		os << ptr.get();
		return os;
	}

	/*
	* COMPARISON OPERATORS
	*/
	template<typename T1, typename D1, typename T2, typename D2>
	bool operator==(const dp::lite_ptr<T1, D1>& lhs, const dp::lite_ptr<T2, D2>& rhs) {
		return lhs.get() == rhs.get();
	}
	template<typename T1, typename D1, typename T2, typename D2>
	bool operator!=(const dp::lite_ptr<T1, D1>& lhs, const dp::lite_ptr<T2, D2>& rhs) {
		return !(lhs == rhs);
	}
	template<typename T1, typename D1, typename T2, typename D2>
	bool operator<(const dp::lite_ptr<T1, D1>& lhs, const dp::lite_ptr<T2, D2>& rhs) {
		return lhs.get() < rhs.get();
	}
	template<typename T1, typename D1, typename T2, typename D2>
	bool operator<=(const dp::lite_ptr<T1, D1>& lhs, const dp::lite_ptr<T2, D2>& rhs) {
		return !(rhs < lhs);
	}
	template<typename T1, typename D1, typename T2, typename D2>
	bool operator>(const dp::lite_ptr<T1, D1>& lhs, const dp::lite_ptr<T2, D2>& rhs) {
		return rhs < lhs;
	}
	template<typename T1, typename D1, typename T2, typename D2>
	bool operator>=(const dp::lite_ptr<T1, D1>& lhs, const dp::lite_ptr<T2, D2>& rhs) {
		return !(lhs < rhs);
	}


}

#endif
