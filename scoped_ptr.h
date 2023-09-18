#ifndef DP_CPP98_SCOPED_PTR
#define DP_CPP98_SCOPED_PTR

#include <cstddef>
#include <algorithm>
#include <ostream>

#include "type_traits.h"
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

namespace dp {

	template<typename T>
	struct default_delete {
		void operator()(T* in) {
			delete in;
		}
	};
	template<typename T>
	struct default_delete<T[]> {
		void operator()(T* in) {
			delete[] in;
		}
	};

	namespace detail {
		template<typename ValT, typename DelT, bool = dp::is_pointer<DelT>::value>
		struct deleter_holder {
		protected:
			DelT m_deleter;

			template<typename T>
			deleter_holder(T in) : m_deleter(in) {}

			void delete_resource(ValT* in) {
				m_deleter(in);
			}

			DelT& get_deleter_impl() {
				return m_deleter;
			}
			const DelT& get_deleter_impl() const {
				return m_deleter;
			}
		};
		template<typename ValT, typename DelT>
		struct deleter_holder<ValT, DelT, false> : DelT {
		protected:
			deleter_holder() {}

			deleter_holder(const DelT& del) : DelT(del) {}

			void delete_resource(ValT* in) {
				this->operator()(in);
			}

			DelT& get_deleter_impl() {
				return static_cast<DelT&>(*this);
			}
			const DelT& get_deleter_impl() const {
				return static_cast<DelT&>(*this);
			}
		};

		//Since the majority of both versions of scoped_ptr are identical,
		//we DRY off our code with a common base
		template<typename T, typename Deleter = dp::default_delete<T> >
		class scoped_ptr_base : detail::deleter_holder<T, Deleter> {
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

			scoped_ptr_base& operator=(pointer in) {
				this->reset(in);
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
	* And now all we need is to add our specific class instances with their specific interfaces
	* Plus whatever C++98 requires of us
	*/
	template<typename T, typename Deleter>
	class scoped_ptr : public dp::detail::scoped_ptr_base<T, Deleter> {

		typedef dp::detail::scoped_ptr_base<T, Deleter> Base;

	public:

		//Alas no inheriting constructors in C++98
		explicit scoped_ptr(T* in = NULL) : Base(in) {}

		explicit scoped_ptr(T* in, Deleter del_inst) : Base(in, del_inst) {}

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

	template<typename T, typename Deleter >
	class scoped_ptr<T[], Deleter> : public dp::detail::scoped_ptr_base<T, Deleter> {

		typedef dp::detail::scoped_ptr_base<T, Deleter> Base;

	public:

		//Alas no inheriting constructors in C++98
		explicit scoped_ptr(T* in = NULL) : Base(in) {}

		explicit scoped_ptr(T* in, Deleter del_inst) : Base(in, del_inst) {}

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

}

#endif
