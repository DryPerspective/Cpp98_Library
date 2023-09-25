#ifndef DP_CPP98_SMART_POINTER_BASES
#define DP_CPP98_SMART_POINTER_BASES

#include <typeinfo>

#include "cpp98/type_traits.h"

namespace dp {
	//Default delete. "Publicly visible" in namespace dp
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
	template<typename T, std::size_t N>
	struct default_delete<T[N]> {
		void operator()(T* in) {
			delete[] in;
		}
	};

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
			shared_control_block_base(const shared_control_block_base&);
			shared_control_block_base& operator=(const shared_control_block_base&);

		protected:
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
				typename AllocT::rebind<shared_block_with_allocator>::other dealloc;
				this->~shared_block_with_allocator();
				dealloc.deallocate(this, sizeof(shared_block_with_allocator));
			}

		public:
			explicit shared_block_with_allocator(InputT* inPtr, DelT inDel, const AllocT& inAlloc) : shared_control_block_base(), m_ptr(inPtr), m_deleter(inDel), m_alloc(inAlloc) {}

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
				typedef typename AllocT::rebind<shared_block_with_allocator<StoredT, DelT, AllocT> >::other alloc_type;
				shared_block_with_allocator* newBlock = NULL;
				alloc_type all;
				try {
					newBlock = all.allocate(sizeof(shared_block_with_allocator<StoredT, DelT, AllocT>));
				}
				catch (...) {
					DelT newDeleter(m_deleter);
					newDeleter(newPtr);
					throw;
				}
				
				try {
					::new (newBlock) shared_block_with_allocator<StoredT, DelT, AllocT>(newPtr, DelT(m_deleter), Alloc(m_alloc));
				}
				catch (...) {
					all.deallocate(newBlock, sizeof(shared_block_with_allocator));
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
		template<typename Y, typename T>
		struct compatible_ptr_type {
			static const bool value = dp::is_convertible<Y*, T*>::value ||
					((dp::is_array<T>::value || dp::is_array<Y>::value) && 
					dp::is_same<typename dp::remove_extent<Y>::type, typename dp::remove_cv<typename dp::remove_extent<T>::type>::type>::value);
		};
		


	}


}
#endif