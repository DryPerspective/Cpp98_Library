#ifndef DP_CPP98_SPAN
#define DP_CPP98_SPAN

#include <cstddef> //std::size_t
#include <iterator>

#include "cpp98/type_traits.h"
#include "cpp98/iterator.h"
#include "bits/misc_memory_functions.h"
#include "bits/fat_pointer.h"
#include "bits/type_traits_ns.h"

#include "bits/version_defs.h"

//We need to play this silly game because Borland can't handle default args correctly.
#ifdef DP_BORLAND
#include "bits/ignore.h"
#define DP_ENABLE_TYPE dp::ignore
#else
#define DP_ENABLE_TYPE bool
#endif





namespace dp {

	template<typename T, std::size_t N>
	struct array;

	template<typename T, std::size_t N>
	class span;

	static const std::size_t dynamic_extent = -1;

	namespace detail {

		template<typename T>
		struct is_special_of_span : dp::false_type {};

		template<typename T, std::size_t N>
		struct is_special_of_span<dp::span<T, N> > : dp::true_type {};

		template<typename T>
		struct is_special_of_array : dp::false_type {};

		template<typename T, std::size_t N>
		struct is_special_of_array<dp::array<T, N> > : dp::true_type {};





		template<typename R>
		struct is_constructible_span_range {
			static const bool value = !is_special_of_span<R>::value &&
				!is_special_of_array<R>::value &&
				!dp::is_array<typename dp::remove_cvref<R>::type>::value;

		};

		//Hacky way around the template limitations we're working with
		//if constexpr would make this shockingly easier
		template<typename T, bool b = T::extent == dp::dynamic_extent>
		struct assign_storage_impl {
			dp::static_assert_98<is_special_of_span<T>::value> assertion;

			void operator()(T& sp, const typename T::value_type* inPtr, std::size_t inSize) {
				sp.m_data.begin = const_cast<typename T::value_type*>(inPtr);
				sp.m_data.size = inSize;
			}
		};

		template<typename T>
		struct assign_storage_impl<T,false> {
			dp::static_assert_98<is_special_of_span<T>::value> assertion;

			void operator()(T& sp, const typename T::value_type* inPtr, std::size_t) {
				sp.m_data = const_cast<typename T::value_type*>(inPtr);
			}
		};

		template<typename T>
		struct assign_storage : assign_storage_impl<T> {};

		template<typename T, bool b = T::extent == dp::dynamic_extent>
		struct span_size_impl {
			dp::static_assert_98<is_special_of_span<T>::value> assertion;

			std::size_t operator()(const T& sp) {
				return sp.m_data.size;
			}
		};

		template<typename T>
		struct span_size_impl<T, false> {
			dp::static_assert_98<is_special_of_span<T>::value> assertion;

			std::size_t operator()(const T& sp) {
				return sp.extent;
			}
		};

		template<typename T>
		struct span_size : span_size_impl<T> {};

		template<typename T>
		struct not_size_t {
#ifndef DP_BORLAND
			static const bool value = !dp::is_convertible<T, std::size_t>::value;
#else
			static const bool value = !dp::is_arithmetic<T>::value;
#endif
		};


#ifdef DP_BORLAND
		//Yes we need this for Borland because Borland can't process too many conditions in one line, unless it's a trait
		template<typename SpanT, typename ArrT>
		struct span_array_check {
			dp::static_assert_98<is_special_of_span<SpanT>::value && is_special_of_array<ArrT>::value > assertion;


			typedef typename dp::param_types<SpanT>::first_param_type span_type;
			typedef typename dp::param_types<ArrT>::first_param_type array_type;

			static const bool value = (SpanT::extent == dp::dynamic_extent || SpanT::extent == dp::param_types<ArrT>::size_type) &&
				dp::is_qualification_conversion<array_type, span_type>::value;
		};
#endif


	}



	template<typename StoredT, std::size_t Extent = dp::dynamic_extent>
	class span {
	public:

		typedef StoredT										element_type;
		typedef typename dp::remove_cv<element_type>::type	value_type;
		typedef std::size_t									size_type;
		typedef std::ptrdiff_t								difference_type;
		typedef element_type*								pointer;
		typedef const element_type*							const_pointer;
		typedef element_type&								reference;
		typedef const element_type&							const_reference;

		typedef value_type*									iterator;
		typedef const value_type*							const_iterator;
		typedef std::reverse_iterator<iterator>				reverse_iterator;
		typedef std::reverse_iterator<const_iterator>		const_reverse_iterator;

		static const std::size_t extent = Extent;


		//It's not so easy to exclude constructors from overload resolution in C++98, so we fall back to assertions
		//To clarify, the three usual options are not viable here:
		//Can't enable_if a template parameter (no default template params for functions)
		//Can't use a dummy enable_if function parameter for non-template ctors (class template params being checked)
		//Can't requires (obviously)
		span() {
			dp::static_assert_98<Extent == 0 || Extent == dp::dynamic_extent>();
			assign_contents(NULL, 0);
		}

		template<typename It>
		span(It begin, std::size_t count) {
			assign_contents(dp::addressof(*begin), count);
		}

		//From here on out we can use dummy enable_if variables to control overload resolution
		//You can manually set them to true/false in the calling code if you *really* want to, but it has no effect.
		template<typename It, typename End>
		span(It begin, End end, typename dp::enable_if<dp::detail::not_size_t<End>::value, bool>::type = true) {
			assign_contents(dp::addressof(*begin), end - begin);
		}

		template<std::size_t N>
		span(typename dp::type_identity<element_type>::type(&arr)[N], typename dp::enable_if<Extent == dp::dynamic_extent || Extent == N, DP_ENABLE_TYPE>::type = true) {
			assign_contents(dp::data(arr), dp::size(arr));
		}

#ifndef DP_BORLAND
		template<typename U, std::size_t N>
		span(dp::array<U,N>& arr, typename dp::enable_if<Extent == dp::dynamic_extent || Extent == N && dp::is_qualification_conversion<U, element_type>::value, DP_ENABLE_TYPE>::type = true){
			assign_contents(dp::data(arr), dp::size(arr));
		}

		template<typename U, std::size_t N>
		span(const dp::array<U, N>& arr, typename dp::enable_if<Extent == dp::dynamic_extent || Extent == N && dp::is_qualification_conversion<U, element_type>::value, DP_ENABLE_TYPE>::type = true) {
			assign_contents(dp::data(arr), dp::size(arr));
		}
#else
		template<typename U, std::size_t N>
		span(dp::array<U,N>& arr, typename dp::enable_if<dp::detail::span_array_check<span<element_type, Extent>,dp::array<U,N> >::value, DP_ENABLE_TYPE>::type = true){
			assign_contents(dp::data(arr), dp::size(arr));
		}
		template<typename U, std::size_t N>
		span(const dp::array<U, N>& arr, typename dp::enable_if<dp::detail::span_array_check<span<element_type, Extent>, dp::array<U, N> >::value, DP_ENABLE_TYPE>::type = true) {
			assign_contents(dp::data(arr), dp::size(arr));
		}
#endif

		template<typename Range>
		span(const Range& r, typename dp::enable_if<detail::is_constructible_span_range<Range>::value, DP_ENABLE_TYPE>::type = true) {
			assign_contents(dp::data(r), dp::size(r));
		}

		template<typename U, std::size_t Ext>
		span(const span<U, Ext>& in, typename dp::enable_if< Extent == dp::dynamic_extent || Ext == dp::dynamic_extent || Ext == Extent, DP_ENABLE_TYPE>::type = true) {
			assign_contents(in.data(), in.size());
		}

		span(const span& other) : m_data(other.m_data){}

		span& operator=(const span& other) {
			m_data = other.m_data;
			return *this;
		}

		iterator begin() const {
		#ifndef DP_BORLAND
			//Curse you quirky const-correctness. Now I need to use a const_cast
			return static_cast<iterator>(const_cast<pointer_type&>(m_data));
		#else
			//Borland doesn't respect the const_cast and will still error.
			//So we take the long way round.
			const_iterator begin(static_cast<const_iterator>(m_data));
			iterator res = const_cast<iterator&>(begin);
			return res;
		#endif
		}

		const_iterator cbegin() const {
			return static_cast<const_iterator>(m_data);
		}

		iterator end() const {
			return begin() + size();
		}

		const_iterator cend() const {
			return cbegin() + size();
		}

		reverse_iterator rbegin() const {
			return reverse_iterator(end());
		}

		const_reverse_iterator crbegin() const {
			return rbegin();
		}

		reverse_iterator rend() const {
			return reverse_iterator(this->begin());
		}

		const_reverse_iterator crend() const {
			return rend();
		}

		reference operator[](std::size_t index) const {
#ifndef DP_BORLAND
			return const_cast<pointer_type&>(m_data)[index];
#else
			const_reference cref = (m_data)[index];
			reference ref = const_cast<reference>(cref);
            return ref;
#endif
		}

		reference front() {
			return *this[0];
		}
		const_reference front() const {
			return *this[0];
		}
		reference back() {
			return *this[size() - 1];
		}
		const_reference back() const {
			return *this[size() - 1];
		}

		pointer data() const {
			return begin();
		}


		std::size_t size() const {
			return detail::span_size<span>()(*this);
		}

		std::size_t size_bytes() const {
			return size() * sizeof(element_type);
		}

		bool empty() const {
			return size() == 0;
		}

		template<std::size_t Count>
		span<element_type, Count> first() const {
			dp::static_assert_98<Extent >= Count>();
			return span<element_type, Count>(begin(), Count);
		}
		span<element_type, dp::dynamic_extent> first(std::size_t Count) const {
			return span<element_type, dp::dynamic_extent>(begin(), Count);
		}

		template<std::size_t Count>
		span<element_type, Count> last() const {
			dp::static_assert_98<Extent >= Count>();
			return span<element_type, Count>(this->data() + (this->size() - Count), Count);
		}
		span<element_type, dp::dynamic_extent> last(std::size_t Count) const {
			return span<element_type, dp::dynamic_extent>(this->data() + (this->size() - Count), Count);
		}

		template<std::size_t Offset, std::size_t Count>
		span<element_type, Count> subspan() const {
			dp::static_assert_98<Offset <= Extent && (Count == dp::dynamic_extent || Count < Extent - Offset)>();
			return span<element_type, Count>(this->data() + Offset, Count);
		}
		span<element_type, dp::dynamic_extent> subspan(std::size_t Offset, std::size_t Count = dp::dynamic_extent) const {
			return span<element_type, dp::dynamic_extent>(this->data() + Offset, std::min(Count, this->size() - Offset));
		}

	private:



		typedef StoredT stored_type;	//Two-phase lookup fixer



		typedef typename dp::conditional<Extent == dp::dynamic_extent, dp::fat_pointer<stored_type>, stored_type*>::type pointer_type;

		pointer_type m_data;


		template<typename T, bool>
		friend struct detail::assign_storage_impl;

		template<typename, bool>
		friend struct detail::span_size_impl;

		void assign_contents(const value_type* inPtr, std::size_t inSize) {
			detail::assign_storage<span>()(*this, inPtr, inSize);
		}

	};

	namespace detail {
		template<typename T, std::size_t N>
		struct span_bytes {
			static const std::size_t size = sizeof(T) * N;
		};
		template<typename T>
		struct span_bytes<T, dp::dynamic_extent> {
			static const std::size_t size = dp::dynamic_extent;
		};
	}

	template<typename T, std::size_t N>
	dp::span<const unsigned char, detail::span_bytes<T, N>::size> as_bytes(dp::span<T, N> s) {
		return dp::span<const unsigned char, detail::span_bytes<T, N>::size>(reinterpret_cast<unsigned char*>(s.data()), s.size_bytes());
	}

	template<typename T, std::size_t N>
	dp::span<unsigned char, detail::span_bytes<T, N>::size> as_writeable_bytes(dp::span<T, N> s) {
		return dp::span<unsigned char, detail::span_bytes<T, N>::size>(reinterpret_cast<unsigned char*>(s.data()), s.size_bytes());
	}





}


#undef DP_ENABLE_TYPE


#endif
