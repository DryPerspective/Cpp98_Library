

#ifndef CPP98_RC_BASE
#define CPP98_RC_BASE

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
	rc_base& operator=(const rc_base&);
	virtual ~rc_base() = 0;

	public:
	void addRef();
	void removeRef();

	void makeUnshareable();
	bool isShared() const;
};

}

#endif
