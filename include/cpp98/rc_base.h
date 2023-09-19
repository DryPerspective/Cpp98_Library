

#ifndef DP_CPP98_RC_BASE
#define DP_CPP98_RC_BASE

#include <cstddef>  //For formal declaration of the std::size_t typedef


//A general "reference-counting" base for use both in RCPtr and when writing
//ref counted classes

namespace dp{

class rc_base{

	std::size_t m_RefCount;
	bool		m_Shareable;

	protected:
	rc_base() : m_RefCount(0), m_Shareable(true) {}
	rc_base(const rc_base&) : m_RefCount(0), m_Shareable(true) {}
	inline rc_base& operator=(const rc_base&) {
		return *this;
	}
	virtual ~rc_base() = 0;

	public:
	inline void addRef() {
			++m_RefCount;
	}
	inline void removeRef() {
		if (--m_RefCount == 0) delete this;
	}

	inline void makeUnshareable() {
		m_Shareable = false;
	}
	inline bool isShared() const {
		return m_RefCount > 0;
	}
};

}

#endif
