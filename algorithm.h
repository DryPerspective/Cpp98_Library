#ifndef DP_CPP98_ALGORITHM
#define DP_CPP98_ALGORITHM

#include <algorithm>
#include <iterator>
#include <functional>

#include "type_traits.h"

/*
*  The algorithms in the <algorithm> header which were added from C++11 onwards, recreated (where possible) here
*/
namespace dp {

	template<typename InputIt, typename T>
	InputIt find_if_not(InputIt begin, InputIt end, const T& value) {
		for (; begin != end; ++begin) {
			if (*begin != value) return begin;
		}
		return end;
	}
	template<typename InputIt, typename UnaryPredicate>
	InputIt find_if_not(InputIt begin, InputIt end, UnaryPredicate pred) {
		for (; begin != end; ++begin) {
			if (!pred(*begin)) return begin;
		}
		return end;
	}

	template<typename InputIt, typename UnaryPredicate>
	bool all_of(InputIt begin, InputIt end, UnaryPredicate pred) {
		return dp::find_if_not(begin, end, pred) == end;
	}
	template<typename InputIt, typename UnaryPredicate>
	bool any_of(InputIt begin, InputIt end, UnaryPredicate pred) {
		return std::find_if(begin, end, pred) != end;
	}
	template<typename InputIt, typename UnaryPredicate>
	bool none_of(InputIt begin, InputIt end, UnaryPredicate pred) {
		return std::find_if(begin, end, pred) == end;
	}
	
	template<typename InputIt, typename Size, typename UnaryFunction>
	InputIt for_each_n(InputIt begin, Size n, UnaryFunction func) {
		for (Size i = 0; i < n; ++begin,  ++i) {
			func(*begin);
		}
		return begin;
	}

	template<typename InputIt, typename OutputIt, typename UnaryPredicate>
	InputIt copy_if(InputIt begin, InputIt end, OutputIt dest_begin, UnaryPredicate pred) {
		for (; begin != end; ++begin) {
			if (pred(begin)) {
				*dest_begin = *begin;
				++dest_begin;
			}
		}
		return dest_begin;
	}

	template<typename InputIt, typename Size, typename OutputIt>
	InputIt copy_n(InputIt begin, Size n, OutputIt dest_begin) {
		if (n > 0) {
			*dest_begin = *begin;
			++dest_begin;
			for (Size i = 1; i != n; ++i, ++dest_begin) {
				*dest_begin = *++begin;
			}
		}
		return dest_begin;
	}



	namespace detail {
		template<typename Iter, typename Tag, bool = dp::is_convertible<typename std::iterator_traits<Iter>::iterator_category, Tag>::value >
		struct is_iter_category : dp::false_type {};

		template<typename Iter, typename Tag>
		struct is_iter_category<Iter, Tag, true> : dp::true_type {};

		template<typename Iter>
		typename dp::enable_if<dp::detail::is_iter_category<Iter, std::bidirectional_iterator_tag>::value, typename std::iterator_traits<Iter>::difference_type>::type 
			bounded_advance(Iter& it, typename std::iterator_traits<Iter>::difference_type n, const Iter bound){
			for (; n < 0 && it != bound; ++n, void(--it));
			for (; n > 0 && it != bound; --n, void(++it));

			return n;
		}

		template<typename Iter>
		typename dp::enable_if<!dp::detail::is_iter_category<Iter, std::bidirectional_iterator_tag>::value, typename std::iterator_traits<Iter>::difference_type>::type
			bounded_advance(Iter& it, typename std::iterator_traits<Iter>::difference_type n, const Iter bound) {
			for (; n > 0 && it != bound; --n, void(++it));

			return n;
		}
	}
	/*
	*  Unlike in C++20, this uses copy semantics, not move semantics. Be warned
	*/
	template<typename ForwardIt>
	ForwardIt shift_left(ForwardIt begin, ForwardIt end, typename std::iterator_traits<ForwardIt>::difference_type n) {
		if (n <= 0) return end;
		ForwardIt copy = begin;
		if (dp::detail::bounded_advance(copy, n, end)) return begin;
		return std::copy(copy, end, begin);
	}
	
	//Easy for bidirectional iterators
	template<typename ForwardIt>
	typename dp::enable_if<dp::detail::is_iter_category<ForwardIt, std::bidirectional_iterator_tag>::value, ForwardIt>::type
		shift_right(ForwardIt begin, ForwardIt end, typename std::iterator_traits<ForwardIt>::difference_type n) {
		if (n <= 0) return begin;
		ForwardIt copy = end;
		if (dp::detail::bounded_advance(copy, -n, begin)) return end;
		return std::copy_backward(begin, copy, end);
	}
	//Less so for forward iterators
	template<typename ForwardIt>
	typename dp::enable_if<!dp::detail::is_iter_category<ForwardIt, std::bidirectional_iterator_tag>::value, ForwardIt>::type
		shift_right(ForwardIt begin, ForwardIt end, typename std::iterator_traits<ForwardIt>::difference_type n) {
		if (n <= 0) return begin;
		ForwardIt copy = begin;
		if (dp::detail::bounded_advance(copy, n, end)) return end;

		ForwardIt lead = copy;
		ForwardIt trail = begin;
		for (; trail != copy; ++lead, ++trail) {
			if (lead == end) {
				// The range looks like:
				//
				//   |-- (n - k) elements --|-- k elements --|-- (n - k) elements --|
				//   ^-begin          trail-^                ^-copy             end-^
				//
				// Note that distance(begin, trail) == distance(result, end)
				std::copy(begin, trail, copy);
				return copy;
			}
		}
		while (true) {
			for (ForwardIt mid = begin; mid != copy; ++lead, ++trail, ++mid) {
				if (lead == end) {
					// The range looks like:
					//
					//   |-- (n - k) elements --|-- k elements --|-- ... --|-- n elements --|
					//   ^-begin            mid-^           copy-^         ^-trail      end-^
					//
					trail = std::copy(mid, copy, trail);
					std::copy(begin, mid, trail);
					return copy;
				}
				std::iter_swap(mid, trail);
			}
		}

	}

	template<typename InputIt, typename UnaryPredicate>
	bool is_partitioned(InputIt begin, InputIt end, UnaryPredicate pred) {
		for (; begin != end; ++begin) {
			if (!pred(*begin)) break;
		}
		for (; begin != end; ++begin) {
			if (pred(*begin)) return false;
		}
		return true;
	}

	template<typename InputIt, typename OutputIt1, typename OutputIt2, typename UnaryPredicate>
	std::pair<OutputIt1, OutputIt2> partition_copy(InputIt begin, InputIt end, OutputIt1 d_begin_true, OutputIt2 d_begin_false, UnaryPredicate pred) {
		for (; begin != end; ++begin) {
			if (pred(*begin)) {
				*d_begin_true = *begin;
				++d_begin_true;
			}
			else {
				*d_begin_false = *begin;
				++d_begin_false;
			}
		}
		return std::pair<OutputIt1, OutputIt2>(d_begin_true, d_begin_false);
	}

    namespace detail{
		//std::next is a C++11 feature, but I've not (yet) reimplemented the entire
		//<iterator> header. For the time being, we keep it here, pending move to
		//its own header when the time comes		
        template<typename InputIt>
			InputIt next(InputIt it, typename std::iterator_traits<InputIt>::difference_type n = 1){
			std::advance(it, n);
			return it;
        }
    }

	template<typename ForwardIt, typename UnaryPredicate>
	ForwardIt partition_point(ForwardIt begin, ForwardIt end, UnaryPredicate pred) {
		typedef typename std::iterator_traits<ForwardIt>::difference_type diff_type;
		for (diff_type length = std::distance(begin, end); length > 0;) {
			diff_type half = length / 2;
			ForwardIt mid = dp::detail::next(begin, half);
			if (pred(*mid)) {
				begin = dp::detail::next(mid);
				length -= (half + 1);
			}
			else {
				length = half;
			}
		}
		return begin;
	}

	template<typename ForwardIt, typename Compare>
	ForwardIt is_sorted_until(ForwardIt begin, ForwardIt end, Compare comp) {
		if (begin != end) {
			ForwardIt next = begin;
			while (++next != end) {
				if (comp(*next, *begin)) return next;
				begin = next;
			}
		}
		return end;
	}
	template<typename ForwardIt>
	ForwardIt is_sorted_until(ForwardIt begin, ForwardIt end) {
		return dp::is_sorted_until(begin, end, std::less<typename std::iterator_traits<ForwardIt>::value_type>());
	}

	template<typename ForwardIt>
	bool is_sorted(ForwardIt begin, ForwardIt end) {
		return dp::is_sorted_until(begin, end) == end;
	}
	template<typename ForwardIt, typename Compare>
	bool is_sorted(ForwardIt begin, ForwardIt end, Compare comp) {
		return dp::is_sorted_until(begin, end, comp) == end;
	}

	template<typename T, typename Compare>
	std::pair<const T&, const T&> minmax(const T& a, const T& b, Compare comp) {
		return comp(a, b) ? std::pair<const T&, const T&>(a, b) : std::pair<const T&, const T&>(b, a);
	}
	template<typename T>
	std::pair<const T&, const T&> minmax(const T& a, const T& b) {
		return dp::minmax(a, b, std::less<T>());
	}

	template<typename ForwardIt, typename Compare>
	std::pair<ForwardIt, ForwardIt> minmax_element(ForwardIt begin, ForwardIt end, Compare comp) {
		ForwardIt min = begin;
		ForwardIt max = begin;
		if (begin == end || ++begin == end) return std::make_pair(min, max);

		if (comp(*begin, *min)) min = begin;
		else					max = begin;

		while (++begin != end) {
			ForwardIt it = begin;
			if (++begin == end) {
				if (comp(*it, *min))	 min = it;
				else if(!comp(*it,*max)) max = it;
				break;
			}
			else {
				if (comp(*begin, *it)) {
					if (comp(*begin, *min)) min = begin;
					if (!comp(*it, *max))	max = it;
				}
				else {
					if (comp(*it, *min))	 min = it;
					if (!comp(*begin, *max)) max = begin;
				}
			}
		}
		return std::make_pair(min, max);
	}
	template<typename ForwardIt>
	std::pair<ForwardIt, ForwardIt> minmax_element(ForwardIt begin, ForwardIt end) {
		return dp::minmax_element(begin, end, std::less<typename std::iterator_traits<ForwardIt>::value_type>());
	}

	template<typename T, typename Compare>
	const T& clamp(const T& value, const T& min, const T& max, Compare comp) {
		return comp(value, min) ? min :
			comp(max, value) ? max : value;
	}
	template<typename T>
	const T& clamp(const T& value, const T& min, const T& max) {
		return dp::clamp(value, min, max, std::less<T>());
	}

	namespace detail {
		/*
		*	To solve the problem of binary predicate -> unary predicate the standard pre-C++11 function was std::bind1st and the extensible functor handle typedefs
		*	These were of course deprecated in C++11 for being a bit crap.
		*	One facet of the goals of this library is that a user, when updating to C++11 and onwards, need only change a bunch of dp:: to a bunch of std:: for their code
		*	to work. Whether we meet that everywhere is a matter for debate, but requiring they use future-deprecated features in their predicates is suboptimal.
		*	For now, we instead reinvent a wheel and reassess that decision later
		*/
		template<typename ForwardIt, typename Value, typename BinaryPredicate>
		typename std::iterator_traits<ForwardIt>::difference_type count_with_pred(ForwardIt begin, ForwardIt end, const Value& val, BinaryPredicate pred){
			typename std::iterator_traits<ForwardIt>::difference_type count = 0;
			for (; begin != end; ++begin) {
				if (pred(val, *begin)) ++count;
			}
			return count;
		}
	}

	template<typename ForwardIt1, typename ForwardIt2, typename BinaryPredicate>
	bool is_permutation(ForwardIt1 begin, ForwardIt1 end, ForwardIt2 dest_begin, ForwardIt2 dest_end, BinaryPredicate pred){
		std::pair<ForwardIt1, ForwardIt2> common_prefix = std::mismatch(begin, end, dest_begin, pred);
		begin = common_prefix.first;
		dest_begin = common_prefix.second;
		if (begin != end) {
			for (ForwardIt1 it = begin; it != end; ++it) {
				//If we've already checked this value, we skip it
				//Hacky analogue of std::find for a binary predicate
				for (ForwardIt1 findIt = begin; findIt != it; ++findIt) {
					if (pred(*findIt, *it)) goto continue_loop;		//May the gods forgive me for using a goto in my code, but it does simplify things quite a bit.	
				}

				{
				//Otherwise make sure this value occurs the same number of times in both remaining ranges
				typedef typename std::iterator_traits<ForwardIt2>::difference_type diff_type;
				diff_type count_in_dest = dp::detail::count_with_pred(dest_begin, dest_end, *it, pred);
				if (count_in_dest == 0 || count_in_dest != dp::detail::count_with_pred(it, end, *it, pred)) return false;
				}
			continue_loop:;
			}			
		}
		return true;	
	}
	template<typename ForwardIt1, typename ForwardIt2, typename BinaryPredicate>
	bool is_permutation(ForwardIt1 begin, ForwardIt1 end, ForwardIt2 dest_begin, BinaryPredicate pred) {
		return dp::is_permutation(begin, end, dest_begin, dp::detail::next(dest_begin, std::distance(begin, end)), pred);
	}

	template<typename ForwardIt1, typename ForwardIt2>
	bool is_permutation(ForwardIt1 begin, ForwardIt1 end, ForwardIt2 dest_begin, ForwardIt2 dest_end) {
		std::pair<ForwardIt1, ForwardIt2> common_prefix = std::mismatch(begin, end, dest_begin);
		begin = common_prefix.first;
        dest_begin = common_prefix.second;
                
        if (begin != end) {
			for (ForwardIt1 it = begin; it != end; ++it) {
				if (it != std::find(begin, it, *it)) continue;

				typedef typename std::iterator_traits<ForwardIt2>::difference_type diff_type;
				diff_type count = std::count(dest_begin, dest_end, *it);
				if (count == 0 || count != std::count(it, end, *it)) return false;

			}
		}
		return true;
	}
	template<typename ForwardIt1, typename ForwardIt2>
	bool is_permutation(ForwardIt1 begin, ForwardIt1 end, ForwardIt2 dest_begin) {
		return dp::is_permutation(begin, end, dest_begin, dp::detail::next(dest_begin, std::distance(begin, end)));
	}



}


#endif