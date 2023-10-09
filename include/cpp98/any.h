#ifndef DP_CPP98_ANY
#define DP_CPP98_ANY

#include <typeinfo>
#include <stdexcept>

#include "cpp98/type_traits.h"
#include "bits/unbound_storage.h"
#include "bits/version_defs.h"

#ifdef DP_BORLAND
#include "bits/type_traits_ns.h"
#endif

//I know how much space will be optimal in my own internal implementation of unbound_storage.
#define DP_ANY_MAX_STACK_SIZE sizeof(long double)

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

		typedef dp::unbound_storage<DP_ANY_MAX_STACK_SIZE> storage_type;

		//Operation performed by our manager function
		struct op {
			enum type {
				access,
				get_type_info,
				clone,
				destroy,
				transfer
			};
		};

		//Type-cheesey way to only store one pointer at a time.
		union arg {
			void* m_object;
			const std::type_info* m_info;
			any* m_any;
		};

		//Templated manager which "magically" tracks the internal type for us.
		template<typename T>
		struct manager_heap {
			static void manage(op::type inOp, const any* inAny, arg* inArg) {
				const T* ptr = static_cast<const T*>(inAny->m_storage.template get<void*>());
				switch (inOp) {
				case op::access:
					inArg->m_object = const_cast<T*>(ptr);
					break;
				case op::get_type_info:
					inArg->m_info = &typeid(T);
					break;
				case op::clone:
					inArg->m_any->m_storage.template get<void*>() = new T(*ptr);
					inArg->m_any->m_manager = inAny->m_manager;
					break;
				case op::destroy:
					delete ptr;
					break;
				case op::transfer:
					inArg->m_any->m_storage.template assign<void*>(inAny->m_storage.template get<void*>());
					inArg->m_any->m_manager = inAny->m_manager;
					const_cast<any*>(inAny)->m_manager = NULL;
				}
			}

			template<typename ValT>
			static void create(storage_type& inStorage, const ValT& val) {
				inStorage.template get<void*>() = new T(val);
			}

			static T* access(const storage_type& in) {
				return static_cast<T*>(in.get<void*>());
			}
		};

		template<typename T>
		struct manager_stack {
			static void manage(op::type inOp, const any* inAny, arg* inArg) {
				const T* ptr = static_cast<const T*>(&inAny->m_storage.template get<T>());
				switch (inOp) {
				case op::access:
					inArg->m_object = const_cast<T*>(ptr);
					break;
				case op::get_type_info:
					inArg->m_info = &typeid(T);
					break;
				case op::clone:
					inArg->m_any->m_storage.template construct<T>(*ptr);
					inArg->m_any->m_manager = inAny->m_manager;
					break;
				case op::destroy:
					ptr->~T();
					break;
				case op::transfer:
					inArg->m_any->m_storage.template construct<T>(*ptr);
					ptr->~T();
					inArg->m_any->m_manager = inAny->m_manager;
					const_cast<any*>(inAny)->m_manager = NULL;
					break;
				}

			}

			template<typename ValT>
			static void create(storage_type& inStorage, const ValT& val) {
				inStorage.template construct<T>(val);
			}

			static T* access(const storage_type& in) {
				const void* buff = &in.get<T>();
				return static_cast<T*>(const_cast<void*>(buff));
			}
		};

		//Metafunction to get the correct manager type for a given value type.
		template<typename ValueType>
		struct get_manager_type {
			typedef typename dp::detail::valid_any_type<ValueType>::decay_type decay_type;
			typedef typename dp::conditional<(sizeof(decay_type) > DP_ANY_MAX_STACK_SIZE), manager_heap<ValueType>, manager_stack<ValueType> >::type type;
		};


		template<typename ValueType>
		friend void* dp::detail::any_caster(const any* in);

		//Small storage optimization for the held object
		storage_type m_storage;

		//Our magic manager which tracks everything for us.
		typedef void(*manager_ptr)(op::type, const any*, arg*);
		manager_ptr m_manager;

	public:

		any() : m_storage(), m_manager(NULL) {}

		any(const any& other) {
			if (other.has_value()) {
				arg newArg;
				newArg.m_any = this;
				other.m_manager(op::clone, &other, &newArg);
			}
			else {
				m_manager = NULL;
				m_storage.construct<void*>((void*)NULL);
			}
		}
		template<typename ValueType>
		any(const ValueType& value, typename dp::enable_if<dp::detail::valid_any_type<ValueType>::value, bool>::type = true) {//: m_manager(&typename get_manager_type<ValueType>::type::manage) {
			typedef typename get_manager_type<ValueType>::type man;
			m_manager = &man::manage;
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
		
		void swap(any& other) {
			//Can't do a flat swap as the anys may contain different types
			if (!this->has_value() && !other.has_value()) return;
			if (this->has_value() && other.has_value()) {
				if (this == &other) return;

				//Convoluted form of temp = A, A = B, B = temp
				any temp;
				arg temp_arg;
				//Transfer other to temp
				temp_arg.m_any = &temp;
				other.m_manager(op::transfer, &other, &temp_arg);
				//Transfer this to other
				temp_arg.m_any = &other;
				this->m_manager(op::transfer, this, &temp_arg);
				//Transfer temp to this
				temp_arg.m_any = this;
				temp.m_manager(op::transfer, &temp, &temp_arg);
			}
			else {
				any* empty = this->has_value() ? &other : this;
				any* full = this->has_value() ? this : &other;
				arg temp_arg;
				temp_arg.m_any = empty;
				full->m_manager(op::transfer, full, &temp_arg);
			}
					
		}
		
		void reset() {
			if (this->has_value()) {
				m_manager(op::destroy, this, NULL);
				m_manager = NULL;
			}
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
		virtual const char* what() const throw() {
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
			if (in->m_manager == &any::get_manager_type<base_type>::type::manage || in->type() == typeid(ValueType)) {
				return any::get_manager_type<ValueType>::type::access(in->m_storage);
			}
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

#undef DP_ANY_MAX_STACK_SIZE

#endif