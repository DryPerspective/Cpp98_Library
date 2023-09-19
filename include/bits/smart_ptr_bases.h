#ifndef DP_CPP98_SMART_POINTER_BASES
#define DP_CPP98_SMART_POINTER_BASES

#include "cpp98/type_traits.h"

namespace dp{
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
}

}

#endif