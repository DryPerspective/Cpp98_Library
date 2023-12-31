#ifndef DP_CPP98_SMART_POINTER_BASES
#define DP_CPP98_SMART_POINTER_BASES

#include <typeinfo>
#include <memory>

#include "cpp98/type_traits.h"
#include "bits/version_defs.h"
#include "cpp98/null_ptr.h"

#include "bits/static_assert_no_macro.h"



namespace dp {
	//Default delete. "Publicly visible" in namespace dp
	template<typename T>
	struct default_delete {
		void operator()(T* in) {
			//No incomplete types or void allowed
			dp::static_assert_98<(sizeof(T) > 0)>();
			delete in;
		}
	};
	template<typename T>
	struct default_delete<T[]> {
		void operator()(T* in) {
			dp::static_assert_98<(sizeof(T) > 0)>();
			delete[] in;
		}
	};
	#ifndef DP_BORLAND
	template<typename T, std::size_t N>
	struct default_delete<T[N]> {
		void operator()(T* in) {
			dp::static_assert_98<(sizeof(T) > 0)>();
			delete[] in;
		}
	};
	#endif

	//Forward decs
	template<typename, typename>
	class scoped_ptr;

	template<typename, typename>
	class lite_ptr;

	template<typename>
	class shared_ptr;

	template<typename>
	class cow_ptr;


	namespace detail {


		/*
		*  UNIQUE OWNERSHIP DELETER HOLDERS
		*/
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

		/*
		*  SHARED OWNERSHIP CONTROL BLOCKS
		*/


		//Shared base for all control block types.
		class shared_control_block_base {
		protected:

			shared_control_block_base(const shared_control_block_base& other);
			shared_control_block_base& operator=(const shared_control_block_base&);

		
			shared_control_block_base() : shared_count(1), weak_count(1) {}

			virtual ~shared_control_block_base() {}

		public:
			std::size_t shared_count;
			std::size_t weak_count;

			virtual void destroy_resource() = 0;
			virtual void destroy_block() = 0;

			//Clone only used for cow_ptr
			virtual shared_control_block_base* clone() = 0;
			virtual void* get() = 0;

			virtual void* get_deleter(const std::type_info&) {
				return NULL;
			}

			void inc_shared() {
				++shared_count;
			}
			void inc_weak() {
				++weak_count;
			}
			void dec_shared() {
				if (--shared_count == 0) {
					destroy_resource();
					dec_weak();
				}

			}
			void dec_weak() {
				if (--weak_count == 0) {
					destroy_block();
				}
			}
		};

		//Control block for no deleter
		template<typename StoredT>
		class shared_block_no_deleter : public shared_control_block_base {

			typedef typename dp::remove_extent<StoredT>::type InputT;

			InputT* m_ptr;

			void destroy_resource() {
				//Spin up a default delete to make sure arrays get deleted with delete[]
				dp::default_delete<StoredT>()(m_ptr);
			}
			void destroy_block() {
				delete this;
			}

		public:

			explicit shared_block_no_deleter(InputT* in) : shared_control_block_base(), m_ptr(in) {}

			shared_block_no_deleter* clone() {
				//I don't really like this design either, but exception safety gotta exception safety.
				InputT* newPtr = NULL;
				shared_block_no_deleter* newBlock = NULL;
				try {
					newPtr = new InputT(*m_ptr);
					newBlock = new shared_block_no_deleter(newPtr);
				}
				catch (...) {
					dp::default_delete<StoredT>()(newPtr);
					delete newBlock;
					throw;
				}
				return newBlock;
			}

			void* get() {
				return m_ptr;
			}

		};

		//Control block with deleter
		template<typename StoredT, typename DelT>
		class shared_block_with_deleter : public shared_control_block_base {

			typedef typename dp::remove_extent<StoredT>::type InputT;

			InputT* m_ptr;
			DelT m_deleter;

			void destroy_resource() {
				m_deleter(m_ptr);
			}
			void destroy_block() {
				delete this;
			}

		public:
			explicit shared_block_with_deleter(InputT* inPtr, DelT inDel) : shared_control_block_base(), m_ptr(inPtr), m_deleter(inDel) {}

			void* get_deleter(const std::type_info& inT) {
				if (inT == typeid(DelT)) return static_cast<void*>(&m_deleter);
				return NULL;
			}

			shared_block_with_deleter* clone() {
				InputT* newPtr = NULL;
				shared_block_with_deleter* newBlock = NULL;
				try {
					newPtr = new InputT(*m_ptr);
					DelT newDeleter(m_deleter);
					newBlock = new shared_block_with_deleter(newPtr, newDeleter);
				}
				catch (...) {
					DelT newDeleter(m_deleter);
					newDeleter(newPtr);
					delete newBlock;
					throw;
				}
				return newBlock;
			}

			void* get() {
				return m_ptr;
			}
		};

		//Control block with deleter and allocator
		template<typename StoredT, typename DelT, typename AllocT>
		class shared_block_with_allocator : public shared_control_block_base {

			typedef typename dp::remove_extent<StoredT>::type InputT;

			InputT* m_ptr;
			DelT m_deleter;
			AllocT m_alloc;

			void destroy_resource() {
				m_deleter(m_ptr);
			}
			void destroy_block() {				
#if !defined(DP_CPP20_OR_HIGHER)
				typename AllocT::rebind<shared_block_with_allocator>::other dealloc;
				dealloc.destroy(this);
				dealloc.deallocate(this, sizeof(shared_block_with_allocator));
#else
				std::allocator_traits<AllocT>::destroy(m_alloc, this);
				std::allocator_traits<AllocT>::deallocate(m_alloc, this, sizeof(shared_block_with_allocator));
#endif
				
			}

		public:
			explicit shared_block_with_allocator(InputT* inPtr, DelT inDel, const AllocT& inAlloc) : shared_control_block_base(), m_ptr(inPtr), m_deleter(inDel), m_alloc(inAlloc) {}

			//Allocator::contruct is non-variadic prior to C++11, which makes it largely useless for constructing things.
			//but we offer allocator support so we provide support for that.
			//We can't call alloc.construct(block_ptr, ptr, deleter, alloc)
			//As such, we need to be able to call alloc.construct(block_ptr, control_block(ptr, deleter, alloc)) which is a copy-construction operation
			shared_block_with_allocator(const shared_block_with_allocator& in) : shared_control_block_base(), m_ptr(in.m_ptr), m_deleter(in.m_deleter), m_alloc(in.m_alloc) {}

			void* get_deleter(const std::type_info& inT) {
				if (inT == typeid(DelT)) return static_cast<void*>(&m_deleter);
				return NULL;
			}

			shared_block_with_allocator* clone() {
				InputT* newPtr = NULL;
				try {
					newPtr = new InputT(*m_ptr);
				}
				catch (...) {
					DelT newDeleter(m_deleter);
					newDeleter(newPtr);
					throw;
				}
				//We separate them because custom deallocation is a little more complex.
#if !defined(DP_CPP20_OR_HIGHER)
				typedef typename AllocT::rebind<shared_block_with_allocator<StoredT, DelT, AllocT> >::other alloc_type;
				alloc_type all;
#endif				
				shared_block_with_allocator* newBlock = NULL;
				
				try {
#if !defined(DP_CPP20_OR_HIGHER)
					newBlock = all.allocate(sizeof(shared_block_with_allocator<StoredT, DelT, AllocT>));
#else
					std::allocator_traits<AllocT>::allocate(m_alloc, sizeof(shared_block_with_allocator<StoredT, DelT, AllocT>));
#endif
				}
				catch (...) {
					DelT newDeleter(m_deleter);
					newDeleter(newPtr);
					throw;
				}
				
				try {
#if !defined(DP_CPP20_OR_HIGHER)
					all.construct(newBlock, shared_block_with_allocator<StoredT, DelT, AllocT>(newPtr, DelT(m_deleter), AllocT(m_alloc)));
#else
					std::allocator_traits<AllocT>::construct(m_alloc, newBlock, newPtr, m_deleter, m_alloc);
#endif
				}
				catch (...) {
#if !defined(DP_CPP20_OR_HIGHER)
					all.deallocate(newBlock, sizeof(shared_block_with_allocator));
#else
					std::allocator_traits<AllocT>::deallocate(m_alloc, newBlock, sizeof(shared_block_with_allocator));
#endif
					DelT newDeleter(m_deleter);
					newDeleter(newPtr);
					throw;
				}
				return newBlock;
			}

			void* get() {
				return m_ptr;
			}
		};


		/*
		*  VALIDITY CHECK FOR "Compatible" conversions
		*/
		#ifndef DP_BORLAND
		template<typename Y, typename T>
		struct compatible_ptr_type {
			static const bool value = dp::is_convertible<Y*, T*>::value ||
					((dp::is_array<T>::value || dp::is_array<Y>::value) &&
					dp::is_same<typename dp::remove_extent<Y>::type, typename dp::remove_cv<typename dp::remove_extent<T>::type>::type>::value);
		};
		#else
        //In borland-land we must be more restrictive, because we have fewer tools.
		template<typename Y, typename T>
		struct compatible_ptr_type{
			static const bool value = dp::is_same<Y,T>::value ||
				((dp::is_array<T>::value || dp::is_array<Y>::value) &&
					dp::is_same<typename dp::remove_extent<Y>::type, typename dp::remove_cv<typename dp::remove_extent<T>::type>::type>::value);
		};
		#endif






	}


}
#endif