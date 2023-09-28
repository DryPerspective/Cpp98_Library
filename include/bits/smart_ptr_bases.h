#ifndef DP_CPP98_SMART_POINTER_BASES
#define DP_CPP98_SMART_POINTER_BASES

#include <typeinfo>
#include <memory>

#include "cpp98/type_traits.h"
#include "bits/version_defs.h"

#include "bits/static_assert_no_macro.h"



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
	#ifndef DP_BORLAND
	template<typename T, std::size_t N>
	struct default_delete<T[N]> {
		void operator()(T* in) {
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
				typename AllocT::rebind<shared_block_with_allocator>::other dealloc;
#if !defined(DP_CPP20_OR_HIGHER)
				dealloc.destroy(this);
#else
				this->~shared_block_with_allocator();
#endif
				dealloc.deallocate(this, sizeof(shared_block_with_allocator));
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
#if !defined(DP_CPP20_OR_HIGHER)
					all.construct(newBlock, shared_block_with_allocator<StoredT, DelT, AllocT>(newPtr, DelT(m_deleter), AllocT(m_alloc)));
#else
					::new (newBlock) shared_block_with_allocator<StoredT, DelT, AllocT>(newPtr, DelT(m_deleter), AllocT(m_alloc));
#endif
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


		template<typename StoredT>
		class shared_ownership_base {
		public:
			typedef typename dp::remove_extent<StoredT>::type	element_type;

		protected:

			typedef StoredT stored_type;   //Two-phase lookup fixer

			//NB: Correct constructors before changing order.
			//Other smart pointer ctors rely on this order
			element_type* m_ptr;
			detail::shared_control_block_base* m_control;

			//Aliasing ctor not supported in all derived classes
			template<typename U>
			shared_ownership_base(const shared_ownership_base<U>& inPtr, element_type* r) : m_ptr(r), m_control(inPtr.m_control) {
				if (m_control) m_control->inc_shared();
			}

			//Derived classes of different types should not be able to swap with each other
			void swap(shared_ownership_base& other) {
				using std::swap;
				swap(m_ptr, other.m_ptr);
				swap(m_control, other.m_control);
			}

			shared_ownership_base& operator=(const shared_ownership_base&);

		public:

			shared_ownership_base() : m_ptr(NULL), m_control(NULL) {}

			template<typename U>
			explicit shared_ownership_base(U* inPtr) : m_ptr(inPtr), m_control(new dp::detail::shared_block_no_deleter<stored_type>(inPtr)) {
				dp::static_assert_98<dp::detail::compatible_ptr_type<U, stored_type>::value>();
			}

			template<typename U, typename Deleter>
			shared_ownership_base(U* inPtr, Deleter inDel) : m_ptr(inPtr), m_control(new dp::detail::shared_block_with_deleter<U, Deleter>(inPtr, inDel)) {
				dp::static_assert_98<dp::detail::compatible_ptr_type<U, stored_type>::value>();
			}

			template<typename U, typename Deleter, typename Alloc>
			shared_ownership_base(U* inPtr, Deleter inDel, Alloc inAlloc) : m_ptr(inPtr) {
				dp::static_assert_98<dp::detail::compatible_ptr_type<U, stored_type>::value>();
				typedef typename Alloc::rebind<dp::detail::shared_block_with_allocator<U, Deleter, Alloc> >::other Rebind;
				Rebind rb;
				m_control = rb.allocate(sizeof(dp::detail::shared_block_with_allocator<U, Deleter, Alloc>));
				try {
#if !defined(DP_CPP20_OR_HIGHER)
					dp::detail::shared_block_with_allocator<U, Deleter, Alloc>* block = NULL;
					rb.construct(block, dp::detail::shared_block_with_allocator<U, Deleter, Alloc>(inPtr, inDel, inAlloc));
					m_control = block;
#else
					::new (m_control) dp::detail::shared_block_with_allocator<U, Deleter, Alloc>(inPtr, inDel, inAlloc);
#endif
				}
				catch (...) {
					rb.deallocate(static_cast<dp::detail::shared_block_with_allocator<U, Deleter, Alloc>*>(m_control), sizeof(dp::detail::shared_block_with_allocator<U, Deleter, Alloc>));
					throw;
				}
			}

			//Copy ctor
			shared_ownership_base(const shared_ownership_base& inPtr) : m_ptr(inPtr.m_ptr), m_control(inPtr.m_control) {
				if (m_control) m_control->inc_shared();
			}
			template<typename U>
			shared_ownership_base(const shared_ownership_base<U>& inPtr) : m_ptr(inPtr.get()), m_control(inPtr.m_control) {
				dp::static_assert_98<dp::detail::compatible_ptr_type<U, stored_type>::value>();
				if (m_control) m_control->inc_shared();
			}

#ifndef DP_CPP17_OR_HIGHER
			template<typename U>
			shared_ownership_base(std::auto_ptr<U>& inPtr) : m_ptr(inPtr.get()), m_control(new dp::detail::shared_block_no_deleter<stored_type>(inPtr.release())) {
				dp::static_assert_98<dp::detail::compatible_ptr_type<U, stored_type>::value>();
			}
#endif
			template<typename U, typename Deleter>
			shared_ownership_base(dp::scoped_ptr<U, Deleter>& inPtr) : m_ptr(inPtr.get()), m_control(new dp::detail::shared_block_with_deleter<stored_type, Deleter>(inPtr.release(), inPtr.get_deleter())) {
				dp::static_assert_98<dp::detail::compatible_ptr_type<U, stored_type>::value>();
			}

			template<typename U, typename Deleter>
			shared_ownership_base(dp::lite_ptr<U, Deleter>& inPtr) : m_ptr(inPtr.get()), m_control(new dp::detail::shared_block_with_deleter<stored_type, Deleter>(inPtr.release(), inPtr.get_deleter())) {
				dp::static_assert_98<dp::detail::compatible_ptr_type<U, stored_type>::value>();
			}


			~shared_ownership_base() {
				if (m_control) m_control->dec_shared();
			}


#ifndef DP_CPP17_OR_HIGHER
			template<typename U>
			typename dp::enable_if<dp::detail::compatible_ptr_type<U, stored_type>::value, shared_ownership_base&>::type operator=(std::auto_ptr<U>& inPtr) {
				shared_ownership_base copy(inPtr);
				this->swap(copy);
				return *this;
			}
#endif

			template<typename U, typename DelT>
			typename dp::enable_if<dp::detail::compatible_ptr_type<U, stored_type>::value, shared_ownership_base&>::type operator=(dp::scoped_ptr<U, DelT>& inPtr) {
				shared_ownership_base copy(inPtr);
				this->swap(copy);
				return *this;
			}

			template<typename U, typename DelT>
			typename dp::enable_if<dp::detail::compatible_ptr_type<U, stored_type>::value, shared_ownership_base&>::type operator=(dp::lite_ptr<U, DelT>& inPtr) {
				shared_ownership_base copy(inPtr);
				this->swap(copy);
				return *this;
			}

			void reset() {
				m_control->dec_shared();
				m_control = NULL;
				m_ptr = NULL;
			}

			template<typename U>
			void reset(U* inPtr) {
				dp::static_assert_98<dp::detail::compatible_ptr_type<U, stored_type>::value>();
				//Do the potentially throwing thing first
				dp::detail::shared_control_block_base* newBlock = new dp::detail::shared_block_no_deleter<stored_type>(inPtr);

				//Then, since simple pointer assignment shouldn't throw, we can provide the strong exception guarantee.
				m_control->dec_shared();
				m_ptr = inPtr;
				m_control = newBlock;
			}
			template<typename U, typename Deleter>
			void reset(U* inPtr, Deleter inDel) {
				dp::static_assert_98<dp::detail::compatible_ptr_type<U, stored_type>::value>();
				dp::detail::shared_control_block_base* newBlock = new dp::detail::shared_block_with_deleter<stored_type, Deleter>(inPtr, inDel);

				m_control->dec_shared();
				m_ptr = inPtr;
				m_control = newBlock;
			}
			template<typename U, typename Deleter, typename Alloc>
			void reset(U* inPtr, Deleter inDel, Alloc inAlloc) {
				dp::static_assert_98<dp::detail::compatible_ptr_type<U, stored_type>::value>();
				dp::detail::shared_control_block_base* newBlock = new dp::detail::shared_block_with_allocator<stored_type, Deleter, Alloc>(inPtr, inDel, inAlloc);

				m_control->dec_shared();
				m_ptr = inPtr;
				m_control = newBlock;
			}



			std::size_t use_count() const {
				return m_control->shared_count;
			}

			bool unique() const {
				return use_count() == 1;
			}

			operator bool() const {
				return m_ptr != NULL;
			}


		};





	}


}
#endif