#ifndef DP_CPP98_EXPECTED
#define DP_CPP98_EXPECTED

#include <exception>

#include "bits/optional_expected_base.h"
#include "bits/type_traits_ns.h"

namespace dp{
    template<typename ErrT>
    struct bad_expected_access;
    template<>
    struct bad_expected_access<void> : std::exception{
        const char* what() const throw(){
            return "Bad expected access";
        }
    };    
    template<typename ErrT>
    struct bad_expected_access : bad_expected_access<void>{
        ErrT stored_error;

        bad_expected_access(const ErrT& E) : stored_error(E) {}
        const ErrT& error() const {return stored_error;}
        ErrT& error(){return stored_error;}
    };

    //Unexpected. For construction and holding of an unexpected value
    template<typename ErrT>
    class unexpected{
        ErrT stored_value;

    public:
        unexpected(const dp::unexpected<ErrT>& other) : stored_value(other.stored_value) {}
        template<typename U>
        unexpected(const U& other) : stored_value(other) {}

        const ErrT& error() const{ return stored_value; }
        ErrT& error(){ return stored_value; }

        void swap(dp::unexpected<ErrT>& other){
            using std::swap;
            swap(stored_value, other.stored_value);
        }

        template<typename U>
        friend bool operator==(dp::unexpected<ErrT>& lhs, dp::unexpected<U>& rhs){
            return lhs.stored_value == rhs.stored_value;
        }
    };

    template<typename ErrT>
    void swap(dp::unexpected<ErrT>& lhs, dp::unexpected<ErrT>& rhs){
        lhs.swap(rhs);
    }

    //And our expected object
    //Inheritance from the shared base, using storage allocated for whichever is larger
    //of the value type or the error type.
    template<typename ValT, typename ErrT>
    class expected : public dp::detail::opt_exp_base<ValT, dp::type_sizes<ValT,ErrT>::larger_size>{
        typedef typename dp::detail::opt_exp_base<ValT, dp::type_sizes<ValT,ErrT>::larger_size> Base;

        inline ErrT& storedError(){
            return *reinterpret_cast<ErrT*>(Base::m_Storage);
        }

        inline const ErrT& storedError() const{
            return *reinterpret_cast<const ErrT*>(Base::m_Storage);
        }



    public:
        typedef ValT                    value_type;
        typedef ErrT                    error_type;
        typedef dp::unexpected<ErrT>    unexpected_type;
        //No using alias so we make do
        template<typename U>
        struct rebind{
            typedef typename dp::expected<U, error_type> type;
        };

        expected() : Base(true) {}

        template<typename U>
        expected(const U& in) : Base(in) {}

        template<typename U, typename G>
        expected(const dp::expected<U,G>& other) : Base::m_HasValue(other.has_value()){
            if(this->has_value()){
                new(Base::m_Storage) ValT(*other);
            }
            else{
                new(Base::m_Storage) ErrT(other.error());
            }
        }
        template<typename U>
        expected(const dp::unexpected<U>& other) : Base(false){
            new(Base::m_Storage) ErrT(other.error());
        }

        ~expected(){
            this->reset();
        }

        template<typename U>
        expected& operator=(const U& in){
            //If we have a value already we can use the existing assignment
            //Otherwise we need to worry about destructing our error type first
            //No strong exception guarantee on this one.
            if(!this->has_value()) this->storedError().~ErrT();
            new (Base::m_Storage) ValT(in);
            this->m_HasValue = true;
            return *this;
        }

        template<typename U, typename G>
        expected& operator=(const dp::expected<U,G>& in){
            if(this->has_value() && in.has_value()){
                this->storedObject() = in.storedObject();
            }
            else if(this->has_value()){
                this->storedObject().~ValT();
                new (Base::m_Storage) ErrT(in.error());
                this->m_HasValue = false;
            }
            else if(in.has_value()){
                this->storedError().~ErrT();
                new (Base::m_Storage) ValT(in.value());
                this->m_HasValue = true;                
            }
            else{
                this->storedError() = in.error();
            }
            return *this;
        }

        template<typename U>
        expected& operator=(const dp::unexpected<U>& in){
            if(this->has_value()){
                this->storedObject().~ValT();
                new (Base::m_Storage) ErrT(in.error());
                this->m_HasValue = false;
            }
            else{
                this->storedError() = in.error();
            }
            return *this;
        }

        //Not shared because of differing exceptions thrown.
	    ValT& value() {
		    if (!this->has_value()) throw dp::bad_expected_access<ErrT>(this->error());
		    return this->storedObject();
	    }
	    const ValT& value() const {
		    if (!this->has_value()) throw dp::bad_expected_access<ErrT>(this->error());
		    return this->storedObject();
	    }

        ErrT& error(){
            return this->storedError();
        }

        const ErrT& error() const{
            return this->storedError();
        }

        void reset(){
            if(this->has_value()) this->storedObject().~ValT();
            else storedError().~ErrT();
            this->m_HasValue = false;
        }

        void swap(expected<ValT,ErrT>& other){
            using std::swap;
            if(this->has_value() && other.has_value()){
                swap(this->storedObject(), other.storedObject());
            }
            else if(this->has_value()){
                ValT temp(this->storedObject());
                *this = dp::unexpected<ErrT>(other.error());
                other = temp;
            }
            else if(other.has_value()){
                other.swap(*this);
            }
            else{
                swap(this->storedError(), other.storedError());
            }

        }


    };

    template<typename ValT, typename ErrT>
    void swap(dp::expected<ValT,ErrT>& lhs, dp::expected<ValT,ErrT>& rhs){
        lhs.swap(rhs);
    }










}

#endif