#include "RCBase.h"


namespace dp{

RCBase& RCBase::operator=(const RCBase&){
	return *this;
}

RCBase::~RCBase(){}

void RCBase::addRef(){
	++m_RefCount;
}

void RCBase::removeRef(){
	if(--m_RefCount == 0) delete this;
}

void RCBase::makeUnshareable(){
	m_Shareable = false;
}

bool RCBase::isShared() const{
    return m_RefCount > 0;
}

}
