#ifndef DP_CPP98_RC_PTR
#define DP_CPP98_RC_PTR

#include "cpp98/rc_base.h"

#include <cassert>


/*
*   A reference counting smart pointer. At the cost of more complex code and little extra allocation,
*   this smart pointer allows us to a lot more freedom in terms of what we can do with it.
*
*   This follows a fairly basic reference-counting, copy-on-write model and all that entails.
*
*   If you're reading this in the far off futuristic world of C++11, disregard this class and swap it out for
*   std::shared_ptr instead. Note that the interface doesn't exactly match because the underlying techniques
*   used for both pointers differ, but it should be simple enough to implement.
*/

namespace dp{

template<typename T>
class rc_ptr{
private:
	//We nest a reference counter as its own class inheriting from the general
	//reference counting base.
	struct Counter : public rc_base{
		T* m_data;
		~Counter() {delete m_data;}
	};

	Counter* m_counter;

	void init(){
		//If the new ptr points to an already existent resource, we make our own.
		if(!m_counter->isShared()){
			T* orig = m_counter->m_data;
			m_counter = new Counter;
			m_counter->m_data = new T(*orig);
		}
		//In all cases we increment the ref ocunter.
		m_counter->addRef();
	}

	//Make this into a full and distinct copy of itself, to allow for COW behaviour.
	void makeCopy(){
		if(m_counter->isShared()){
			T* orig = m_counter->m_data;
			m_counter->removeRef();
			m_counter = new Counter;
			m_counter->m_data = new T(*orig);
			m_counter->addRef();
		}
	}


public:
	rc_ptr(T* in = 0) : m_counter(new Counter) {
		m_counter->m_data = in;
		init();
	}

	rc_ptr(const rc_ptr& in) : m_counter(in.m_counter){
		init();
	}

	~rc_ptr(){
        m_counter->removeRef();
	}

	rc_ptr& operator=(const rc_ptr& in){
		if(m_counter != in.m_counter){
			m_counter->removeRef();
			m_counter = in.m_counter;
			init();
		}
		return *this;
	}

	//Const, non-copying functions
	const T* operator->() const{
		return m_counter->m_data;
	}
	const T& operator*() const{
		return *(m_counter->m_data);
	}
	const T* get()	const{
		return m_counter->m_data;
	}

	//Non-const, COW functions
	T* operator->(){
		makeCopy();
		return m_counter->m_data;
	}
	T& operator*(){
		makeCopy();
		return *(m_counter->m_data);
	}
	T* get(){
		makeCopy();
		return m_counter->m_data;
	}

	//Release ownership and return (a copy of) the data
	T* release(){
		T* temp = new T(m_counter->m_data);
		m_counter->removeRef();
		return temp;
	}

	//Reset the pointer with a new value
	void reset(T* in = 0){
		m_counter->removeRef();
		m_counter = new Counter;
		m_counter->m_data = in;
		m_counter->addRef();
	}

	operator bool(){
		return m_counter->m_data;
	}

};

//"make_ptr" factory functions, designed to be swapped out for std::make_shared as soon as the opportunity arises
template<typename T>
rc_ptr<T> make_rc_ptr(){
	return rc_ptr<T>(new T);
}

template<typename T, typename U>
rc_ptr<T> make_rc_ptr(const U& in){
	return rc_ptr<T>(new T(in));
}


/*
*   ARRAY VERSION
*   Do not mix and match pointers to arrays and pointers to individual values.
*   The language will let you do it, but the results will be undefined.
*/


template<typename T>
class rc_ptr<T[]>{
private:
	struct Counter : public rc_base{
		T* m_data;
		std::size_t m_size;  	//Number of elements in the array
		~Counter(){
			delete[] m_data;
		}
	};

	Counter* m_counter;


	void init(){
		if(!m_counter->isShared()){
			Counter* orig = m_counter;
			m_counter = new Counter;
			m_counter->m_size = orig->m_size;
			m_counter->m_data = new T[m_counter->m_size];
			for(int i = 0; i < m_counter->m_size; ++i){
				m_counter->m_data[i] = orig->m_data[i];
			}
		}

		m_counter->addRef();
	}

	//Make this into a full and distinct copy of itself, to allow for COW behaviour.
	void makeCopy(){
		if(m_counter->isShared()){
			Counter* orig = m_counter;
			m_counter->removeRef();         //We know this won't destroy the counter because of the condition
			m_counter = new Counter;
			m_counter->m_size = orig->m_size;
			m_counter->m_data = new T[m_counter->m_size];
			for(int i = 0; i < m_counter->m_size; ++i){
				m_counter->m_data[i] = orig->m_data[i];
			}
			m_counter->addRef();
		}
	}

	public:
	//Construct a null pointer
	rc_ptr() : m_counter(new Counter){
		m_counter->m_data = NULL;
		m_counter->m_size = 0;
	}


	rc_ptr(T* inData, std::size_t inSize) : m_counter(new Counter){
		m_counter->m_size = inSize;
		m_counter->m_data = inData;
		init();
	}

	rc_ptr(const rc_ptr& in) : m_counter(in.m_counter){
		init();
	}

	~rc_ptr(){
		m_counter->removeRef();
	}

	rc_ptr& operator=(const rc_ptr& in){
		if(m_counter != in.m_counter){
			m_counter->removeRef();
			m_counter = in.m_counter;
			init();
		}
	return *this;
	}

	//Const access
	const T& operator[](std::size_t index) const{
		return (m_counter->m_data)[index];
	}

	const T* get() const{
		return m_counter->m_data;
	}

	//COW access
	T& operator[](std::size_t index){
		makeCopy();
		return (m_counter->m_data)[index];
	}

	T* get(){
		makeCopy();
		return m_counter->m_data;
	}

	//Release ownership and return (a copy of) the data
	T* release(){
		T* temp = new T[m_counter->m_size];
		for(int i = 0; i < m_counter->m_size; ++i){
			temp[i] = m_counter->m_data[i];
		}
		m_counter->removeRef();
		return temp;
	}

	//Reset to a null state
	void reset(){
		m_counter->removeRef();
		m_counter = new Counter;
		m_counter->m_data = NULL;
		m_counter->m_size = 0;
	}

	//Reset the N default-constructed values
	void reset(std::size_t N){
		m_counter->removeRef();
		m_counter = new Counter;
		m_counter->m_data = new T[N];
		m_counter->size = N;
	}

	//Reset the pointer with a new value
	void reset(T* inData, std::size_t inSize){
		m_counter->removeRef();
		m_counter = new Counter;
		m_counter->m_data = inData;
		m_counter->m_size = inSize;
		m_counter->addRef();
	}

	operator bool(){
		return m_counter->m_data;
	}

};

//NOT IMPEMENTED YET DON'T TRY TO MAKE_rc_ptr WITH AN ARRAY

/*
//As with SmrtPtr, this first function is left deliberately undefined
//Don't define it - don't use it. This is here because without it, overload resolution
//might pick a non-array pointer for an array new.
//Use the one which provides a size
template<typename T>
rc_ptr<T[]> make_rc_ptr();

//Due the make_rc_ptr for arrays runs a little differently. It is called like
// make_rc_ptr<int,5> To create an
template<typename T, std::size_t N>
rc_ptr<T[]> make_rc_ptr(){
	return rc_ptr<T[]>(new T[N], N);
}
*/

}

#endif
