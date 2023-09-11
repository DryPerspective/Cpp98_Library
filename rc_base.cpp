#include "rc_base.h"


namespace dp{

rc_base& rc_base::operator=(const rc_base&){
	return *this;
}

rc_base::~rc_base(){}

void rc_base::addRef(){
	++m_RefCount;
}

void rc_base::removeRef(){
	if(--m_RefCount == 0) delete this;
}

void rc_base::makeUnshareable(){
	m_Shareable = false;
}

bool rc_base::isShared() const{
    return m_RefCount > 0;
}

}
