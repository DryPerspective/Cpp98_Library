#ifndef CPP98_SCOPED_PTR
#define CPP98_SCOPED_PTR

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
		};
		template<typename ValT, typename DelT>
		struct deleter_holder<ValT, DelT, false> : DelT {
		protected:
			deleter_holder() {}

			deleter_holder(const DelT& del) : DelT(del) {}

			void delete_resource(ValT* in) {
				this->operator()(in);
			}
		};
	}


	template<typename T, typename Deleter = dp::default_delete<T> >
	class scoped_ptr : detail::deleter_holder<T, Deleter> {

		T* m_data;

		//We explicitly forbid copying.
		scoped_ptr(const scoped_ptr&);
		scoped_ptr& operator=(const scoped_ptr&);

	public:

		typedef T       element_type;
		typedef T*		pointer;
		typedef Deleter deleter_type;

		explicit scoped_ptr(T* in = NULL) : m_data(in) {}

		explicit scoped_ptr(pointer in, deleter_type del_inst) : detail::deleter_holder<T, Deleter>(del_inst), m_data(in) {}

		scoped_ptr& operator=(pointer in) {
			this->reset(in);
			return *this;
		}

		~scoped_ptr() { this->reset(); }

		void swap(scoped_ptr& other) {
			using std::swap;
			swap(this->get(), other.get());
		}

		const element_type& operator*() const { return *m_data; }
		element_type& operator*() { return *m_data; }
		const pointer operator->() const { return m_data; }
		pointer operator->() { return m_data; }
		const pointer get() const { return m_data; }
		pointer get() { return m_data; }

		//Get_deleter. Unfortunately we must do more template trickery to disambiguate whether to return a function pointer or a deleter class

		template<bool = dp::is_pointer<Deleter>::value>
		deleter_type& get_deleter() { return m_deleter; }
		template<>
		deleter_type& get_deleter<false>() { return static_cast<Deleter&>(*this); }

		template<bool = dp::is_pointer<Deleter>::value>
		const deleter_type& get_deleter() const { return m_deleter; }
		template<>
		const deleter_type& get_deleter<false>() const { return static_cast<Deleter&>(*this); }


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

	/*
	*   ARRAY VERSION
	*   Don't mix and match array and non-array pointers, dumb or smart. Calling the wrong delete variant will cause UB.
	*   There's no way for me to enforce that in this interface, so just don't be an idiot.
	*/

	template<typename T, typename Deleter >
	class scoped_ptr<T[], Deleter> : detail::deleter_holder<T, Deleter> {

		T* m_data;

		//Again we forbid copying
		scoped_ptr(const scoped_ptr&);
		scoped_ptr& operator=(const scoped_ptr&);

	public:

		typedef T       element_type;
		typedef T*		pointer;
		typedef Deleter deleter_type;

		explicit scoped_ptr(pointer in = NULL) : m_data(in) {}

		explicit scoped_ptr(pointer in, deleter_type del_inst) : detail::deleter_holder<T, Deleter>(del_inst), m_data(in) {}

		//We use the correct delete operator.
		~scoped_ptr() {
			this->reset();
		}

		scoped_ptr& operator=(pointer in) {
			this->reset(in);
			return *this;
		}

		void swap(scoped_ptr& other) {
			using std::swap;
			swap(this->get(), other.get());
		}

		//Rather than dereference and pointer access operators, we provide array access
		const element_type& operator[](std::size_t N) const {
			return m_data[N];
		}
		element_type& operator[](std::size_t N) {
			return m_data[N];
		}

		//As before, offer functions to get, reset, and release
		const pointer get() const {
			return m_data;
		}
		pointer get() {
			return m_data;
		}

		pointer release() {
			T* temp = m_data;
			m_data = NULL;
			return temp;
		}

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


	//Freestanding make_scoped functions, mirroring std::make_unique and std::make_shared from C++11
	//Set up so that updating to those pointers is a simple find and replace
	//These make the allocation of a new resource and the binding it to a scoped_ptr an atomic operation to the caller
	//Meaning that we should not leak even if the eval order messes around.
	//enable_if to prevent dp::make_scoped<int[]> from picking this overload and going full UB.
	template<typename T>
	typename dp::enable_if<!dp::is_array<T>::value, dp::scoped_ptr<T> >::type make_scoped() {
		return dp::scoped_ptr<T>(new T);
	}

	template<typename T, typename U>
	dp::scoped_ptr<T> make_scoped(const U& in) {
		return dp::scoped_ptr<T>(new T(in));
	}

	//Make_scoped functions for the array version
	//Note no "default" option available for array types.
	template<typename T>
	dp::scoped_ptr<T[]> make_scoped(std::size_t N){
		return dp::scoped_ptr<T[]>(new T[N]);
	}


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
