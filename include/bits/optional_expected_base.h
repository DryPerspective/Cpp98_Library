#ifndef DP_CPP98_OPTIONAL_EXPECTED_BASE
#define DP_CPP98_OPTIONAL_EXPECTED_BASE

#include <new>
#include <algorithm> //For std::swap, which lived in <algorithm> before C++11

#include "bits/type_traits_ns.h"

namespace dp {
namespace detail {
	template<typename T, std::size_t ContSize>
	class opt_exp_base {
	protected:

		typedef T contained_type; //Typedef to avoid two-phase lookup issues

		//Union trick to delay initialization
		union {
			unsigned char m_Storage[ContSize];					//The data representing the object.
			long double m_phony_max_align;          			//Hacky analogue of std::max_align_t
		};
		bool      m_HasValue;

		//Cut down on reinterpret_casts all over the place
		inline T& storedObject() {
			return *reinterpret_cast<T*>(m_Storage);
		}

		inline const T& storedObject() const {
			return *reinterpret_cast<const T*>(m_Storage);
		}

        //As optionals and expecteds have different behaviour for default construction
        //We abstract the difference here
        explicit opt_exp_base(bool in) : m_HasValue(in){
            if(m_HasValue) new (m_Storage) T();
        }


		~opt_exp_base()
		{
			this->reset();
		}


	public:

		//Public facing typedef. Less ugly than public-private-public switcheroo to use the other one
		typedef T value_type;


		opt_exp_base(const opt_exp_base& in) : m_HasValue(in.m_HasValue) {
			//Not initialized so no risk of overwriting a valid object
			if (m_HasValue) new (m_Storage) T(*in);
		}

		//Don't try to be clever and remove these constructors from the overload set is U is not convertible to T. It creates problems.
		//Because operator bool cannot be explicit, putting that gate up at the overload selection stage results in an undesirable conversion
		//optional<T> -> bool -> optional<any_integral_type>
		template<typename U>
		explicit opt_exp_base(const opt_exp_base<U, ContSize>& in) : m_HasValue(in.has_value()) {
			if (m_HasValue) new (m_Storage) T(*in);
		}

		template<typename U>
		opt_exp_base(const U& Value) : m_HasValue(true) {
			new (m_Storage) T(Value);
		}

		template<typename U>
		opt_exp_base& operator=(const U& in) {
			if (m_HasValue) {
				using std::swap;
				T copy(in);
				swap(storedObject(), copy);
			}
			else {
				new (m_Storage) T(in);
				m_HasValue = true;
			}
			return *this;
		}

		template<typename U>
		opt_exp_base& operator=(const opt_exp_base<U, ContSize>& in) {
			//Note that opt_exp_base::swap's exception specification depends on both swapping and copying.
			//Copy-and-swap should still maintain the strong exception guarantee here. Assuming no abnormal exception behaviour
			//which would also trip up a usual copy-and-swap (e.g. throwing destructor), the latest an exception can be thrown is 
			//in the first operation within swap(), at which point the original state in both cases will not have been modified
			//Since those operations are either a swap (which should be noexcept) or a placement new into uninitialized memory,
			//the meaningful state of the program should not change
			opt_exp_base<T, ContSize> copy(in);
			this->swap(copy);
			return *this;
		}



		void reset() {
			if (m_HasValue) {
				this->storedObject().~T();
				m_HasValue = false;
			}
		}

		//Note we can't use a general template here because
		//std::swap doesn't work for multiple types, even convertible ones
		void swap(dp::detail::opt_exp_base<T, ContSize>& other) {
			//Because of the possibility of one or more object being uninitialized, this can't be a simple swap
			//As such its noexcept specification isn't simple either
			//This is non-throwing if swap(T,T) is nonthrowing and if T is nothrow-copy-constructible.

			/*
			*   Four Important possibilities
			*   1: Both opt_exp_bases have a value - we need a value-wise swap not a bitwise one
			*   2: We have a value but are swapped with an empty.
			*   3: We don't have a value but the other does - can't dereference an uninitialized pointer so we placement new
			*   4: Neither has a value so we don't need to do anything anyway
			*/
			using std::swap;	//Two-step swap, and the m_HasValue needs std::swap anyway
			if (m_HasValue && other.has_value()) {
				swap(this->storedObject(), other.storedObject());
			}
			else if (m_HasValue) {
				new (other.m_Storage) T(this->storedObject());
				this->reset();
			}
			else if (other.m_HasValue) {
				new (m_Storage) T(other.storedObject());
				other.reset();
			}

			swap(m_HasValue, other.m_HasValue);
		}

		const T& operator*() const {
			return storedObject();
		}

		T& operator*() {
			return storedObject();
		}

		const T* operator->() const {
			return &(storedObject());
		}

		T* operator->() {
			return &(storedObject());
		}

		bool has_value() const { return m_HasValue; }
		operator bool() const { return has_value(); }



		//Alas no move semantics/forwarding references
		template<typename Other>
		T value_or(const Other& other) {
			if (has_value()) return storedObject();
			else return other;
		}


	};

}
}


#endif