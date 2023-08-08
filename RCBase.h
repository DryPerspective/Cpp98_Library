//---------------------------------------------------------------------------

#ifndef RCBaseH
#define RCBaseH

#include <cstddef>  //For formal declaration of the std::size_t typedef
//---------------------------------------------------------------------------

//A general "reference-counting" base for use both in RCPtr and when writing
//ref counted classes

namespace dp{

class RCBase{

	std::size_t m_RefCount;
	bool m_Shareable;

	protected:
	RCBase() : m_RefCount(0), m_Shareable(true) {}
	RCBase(const RCBase&) : m_RefCount(0), m_Shareable(true) {}
	RCBase& operator=(const RCBase&);
	virtual ~RCBase() = 0;

	public:
	void addRef();
	void removeRef();

	void makeUnshareable();
	bool isShared() const;
};

}

#endif
