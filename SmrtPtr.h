#ifndef CPP98_SMRTPTR
#define CPP98_SMRTPTR


#include <cctype>
/*
*   A very simple scope-local smart pointer class for C++03. It is in no way as sophisticated as
*   the C++11 smart pointers like unique_ptr and shared_ptr, and should be replaced by std::unique_ptr when the opportunity arises.
*   It does however have the benefit of not having some of the quirks of std::auto_ptr.
*
*   Specifically - this is a non-copyable pointer. Copying is disabled and will lead to UB if you try to force it to happen.
*   While this does significantly limit the scope of possible use-cases (e.g. easy passing around, polymorphism), it side-steps
*   the issue with auto_ptr and pre-11 C++'s lack of defined move semantics.
*
*   It is safe enough for its common enough uses, but it is not safe in a multithreaded environment or some other features.
*   I recommend this as a band-aid solution to prevent leaks, and not a building block to be used everywhere and thrown around
*   into all sorts of new code. As always, understand the implications of what you're doing before you do it.
*   Another limitation is that its member and factory functions can only initialize data with 0 or 1 constructor arguments.
*   This is simply because we dont have variadic templates at our disposal and handwriting every permutation is silly.
*/

namespace dp{

template<typename T>
class SmrtPtr{

	T* m_data;


	//We explicitly forbid copying.
	SmrtPtr(const SmrtPtr&);
	SmrtPtr& operator=(const SmrtPtr&);

	public:

    typedef T element_type;
	typedef T* pointer;


	SmrtPtr(T* in = NULL)	: m_data(in) {}

	SmrtPtr& operator=(T* in){
		this->reset(in);
		return *this;
	}

	template<typename U>
    explicit SmrtPtr(const U& in) : m_data(new T(in)) {}

	~SmrtPtr()  {delete m_data;}

	void swap(SmrtPtr& other){
		T* temp = m_data;
		m_data = other.m_data;
		other.m_data = temp;
	}

	const T& operator*() const  {return *m_data;}
	T& operator*()        		{return *m_data;}
	const T* operator->() const	{return m_data;}
	T* operator->()             {return m_data;}
    const T* get() const        {return m_data;}
	T* get()	        		{return m_data;}


	//Release ownership of the resource and return the raw pointer to it.
	T* release(){
		T* temp = m_data;
		m_data = NULL;
		return temp;
	}

	//Replace the resource we currently own with a new one
	void reset(T* in = NULL){
		if(m_data != in){
			delete m_data;
			m_data = in;
		}
	}


	operator bool() const {
		return m_data;
	}

};

//Freestanding make_smrt functions, mirroring std::make_unique and std::make_shared from C++11
//Set up so that updating to those pointers is a simple find and replace
//These make the allocation of a new resource and the binding it to a Smrt_Ptr an atomic operation to the caller
//Meaning that we should not leak even if the eval order messes around.
template<typename T>
SmrtPtr<T> make_Smrt(){
	return SmrtPtr<T>(new T);
}

template<typename T, typename U>
SmrtPtr<T> make_Smrt(const U& in){
	return SmrtPtr<T>(new T(in));
}



/*
*   ARRAY VERSION
*   Don't mix and match array and non-array pointers, dumb or smart. Calling the wrong delete variant will cause UB.
*   There's no way for me to enforce that in this interface, so just don't be an idiot.
*/


template<typename T>
class SmrtPtr<T[]>{

	T* m_data;

	//Again we forbid copying
	SmrtPtr(const SmrtPtr&);
	SmrtPtr& operator=(const SmrtPtr&);

	public:

	typedef T element_type;
	typedef T* pointer;

	SmrtPtr(T* in = NULL) : m_data(in) {}
	explicit SmrtPtr(std::size_t in) : m_data(new T[in]) {}

	//We use the correct delete operator.
	~SmrtPtr(){
        delete[] m_data;
	}

	SmrtPtr& operator=(T* in){
		this->reset(in);
		return *this;
	}

	void swap(SmrtPtr& other){
		T* temp = m_data;
		m_data = other.m_data;
		other.m_data = temp;
	}

	//Rather than dereference and pointer access operators, we provide array access
	const T& operator[](std::size_t N) const{
		return m_data[N];
	}

	T& operator[](std::size_t N){
		return m_data[N];
	}

	//As before, offer functions to get, reset, and release
	const T* get() const{
		return m_data;
	}

	T* get(){
		return m_data;
	}

	T* release(){
		T* temp = m_data;
		m_data = NULL;
		return temp;
	}

	void reset(T* in = NULL){
		if(m_data != in){
			delete[] m_data;
			m_data = in;
		}
	}


	operator bool() const{
		return m_data;
	}

};

template<typename T>
void swap(SmrtPtr<T>& lhs, SmrtPtr<T>& rhs){
    lhs.swap(rhs);
}

//Make_smrt functions for the array version

//Intentionally left undefined to prevent overload resolution picking the single-object
//version and trying to store an array there. Would =delete but that's a C++11 feature
//If you get an error from trying to call this function then your code is wrong and
//you should change it - most likely you need to provide a size of the array.
template<typename T>
SmrtPtr<T[]> make_Smrt();

template<typename T>
SmrtPtr<T[]> make_Smrt(std::size_t N){
	return SmrtPtr<T[]>(new T[N]);
}

}

#endif
