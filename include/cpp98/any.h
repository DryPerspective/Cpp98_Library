#ifndef DP_CPP98_ANY
#define DP_CPP98_ANY

#include <typeinfo>
#include <stdexcept>

#include "cpp98/type_traits.h"

#ifdef DP_BORLANd
#include "bits/type_traits_ns.h"
#endif

namespace dp {

	class any;

	namespace detail {

		template<typename ValueType>
		void* any_caster(const dp::any*);



#ifndef DP_BORLAND
		template<typename T>
		struct valid_any_type {
			typedef typename dp::decay<T>::type decay_type;
			static const bool value = !dp::is_same<decay_type, dp::any>::value && dp::is_copy_constructible<decay_type>::value;
		};
#else
		//If you're on Borland, good luck to you trying to store functions in an any
		template<typename T, bool = dp::is_array<T>::value>
		struct valid_any_impl {
			typedef typename dp::decay_array<T>::type type;
		};
		template<typename T>
		struct valid_any_impl<T, false>{
			typedef typename dp::remove_cvref<T>::type type;
		};
		template<typename T>
		struct valid_any_type {
			typedef typename valid_any_impl<T>::type decay_type;
			static const bool value = !dp::is_same<decay_type, dp::any>::value;
		};

#endif
	}


	class any {
		//Operation performed by our manager function
		struct op {
			enum type {
				access,
				get_type_info,
				clone,
				destroy
			};
		};

		//Type-cheesey way to only store one pointer at a time.
		union arg {
			void* m_object;
			const std::type_info* m_info;
			any* m_any;
		};

		//Pointer to the resource. Can't scoped/unique_ptr<void> and shared_ptr adds unnecessary overhead
		void* m_storage;

		//Our magic manager which tracks everything for us.
		typedef void(*manager_ptr)(op::type, const any*, arg*);
		manager_ptr m_manager;

		//Templated manager which "magically" tracks the internal type for us.
		template<typename T>
		struct manager {
			static void manage(op::type inOp, const any* inAny, arg* inArg) {
				const T* ptr = static_cast<const T*>(inAny->m_storage);
				switch (inOp) {
				case op::access:
					inArg->m_object = const_cast<T*>(ptr);
					break;
				case op::get_type_info:
					inArg->m_info = &typeid(T);
					break;
				case op::clone:
					inArg->m_any->m_storage = new T(*ptr);
					inArg->m_any->m_manager = inAny->m_manager;
					break;
				case op::destroy:
					delete ptr;
					break;
				}
			}

			template<typename ValT>
			static void create(void*& inPtr, const ValT& val) {
				inPtr = new T(val);
			}
		};


		template<typename ValueType>
		friend void* dp::detail::any_caster(const any* in);

	public:

		any() : m_storage(NULL), m_manager(NULL) {}
		any(const any& other) {
			if (other.has_value()) {
				arg newArg;
				newArg.m_any = this;
				other.m_manager(op::clone, &other, &newArg);
			}
			else {
				m_manager = NULL;
				m_storage = NULL;
			}
		}
		template<typename ValueType>
		any(const ValueType& value, typename dp::enable_if<dp::detail::valid_any_type<ValueType>::value, bool>::type = true) : m_manager(&manager<typename dp::detail::valid_any_type<ValueType>::decay_type>::manage) {
			typedef manager<typename dp::detail::valid_any_type<ValueType>::decay_type> man;
			man::create(m_storage, value);			
		}

		~any() {
			this->reset();
		}

		any& operator=(const any& other) {
			any copy(other);
			this->swap(copy);
			return *this;
		}

		template<typename ValueType>
		typename dp::enable_if<dp::detail::valid_any_type<ValueType>::value, any&>::type operator=(const ValueType& value) {
			any copy(value);
			this->swap(copy);
			return *this;
		}

		void reset() {
			if (this->has_value()) {
				m_manager(op::destroy, this, NULL);
				m_manager = NULL;
			}
		}

		void swap(any& other) {
			using std::swap;
			swap(m_storage, other.m_storage);
			swap(m_manager, other.m_manager);		
		}

		bool has_value() const {
			return m_manager != NULL;
		}

		const std::type_info& type() const {
			if (!this->has_value()) return typeid(void);
			arg id;
			m_manager(op::get_type_info, this, &id);
			return *id.m_info;
		}
	};

	void swap(dp::any& lhs, dp::any& rhs) {
		lhs.swap(rhs);
	}

#ifndef DP_BORLAND_EXCEPTIONS
	struct bad_any_cast : public std::exception {
		virtual const char* what() throw() {
			return "Bad any cast";
		}
	};
#else
	struct bad_any_cast : public System::Sysutils::Exception {
		bad_any_cast() : System::Sysutils::Exception("Bad any cast") {}
	};
#endif


	namespace detail {
		template<typename ValueType>
		void* any_caster(const dp::any* in) {
			typedef typename dp::remove_cv<ValueType>::type base_type;
			//Compare address first to hopefully avoid RTTI from typeid
			if (in->m_manager == &any::manager<base_type>::manage || in->type() == typeid(ValueType)) return in->m_storage;
			return NULL;
		}
	}

	template<typename ValueType>
	ValueType* any_cast(dp::any* in) {
		void* ptr = dp::detail::any_caster<ValueType>(in);
		if (ptr) return static_cast<ValueType*>(ptr);
		throw dp::bad_any_cast();
	}

	template<typename ValueType>
	const ValueType* any_cast(const dp::any* in) {
		return any_cast<ValueType>(const_cast<any*>(in));
	}

	template<typename ValueType>
	ValueType any_cast(const dp::any& in) {
		typedef typename dp::remove_cvref<ValueType>::type base_type;
		return static_cast<ValueType>(*dp::any_cast<base_type>(&in));
	}

	template<typename ValueType>
	ValueType any_cast(dp::any& in) {
		typedef typename dp::remove_cvref<ValueType>::type base_type;
		return static_cast<ValueType>(*dp::any_cast<base_type>(&in));
	}



}

#endif