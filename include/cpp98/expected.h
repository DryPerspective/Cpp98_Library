#ifndef DP_CPP98_EXPECTED
#define DP_CPP98_EXPECTED

#ifdef DP_CPP17_EXPECTED
#error "Both C++98 and C++17 dp::expected detected. Only use one or the other"
#endif

#include <exception>

#include "bits/unbound_storage.h"
#include "bits/type_traits_ns.h"

#include "cpp98/reference_wrapper.h"
#include "cpp98/optional.h"

namespace dp{

    template<typename ErrT>
    struct bad_expected_access;
#ifndef DP_BORLAND_EXCEPTIONS
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
#else
    template<>
    struct bad_expected_access<void> : System::Sysutils::Exception {
        bad_expected_access() : System::Sysutils::Exception(L"Bad expected access") {}
    };
    template<typename ErrT>
    struct bad_expected_access : bad_expected_access<void> {
        ErrT stored_error;

        bad_expected_access(const ErrT& E) : stored_error(E) {}
        const ErrT& error() const { return stored_error; }
        ErrT& error() { return stored_error; }

    };
#endif

    namespace detail {
        //An "unexpected reference" type, to allow us to save a copy of an expensive type
        template<typename ErrT>
        class unexpect_ref {
            dp::reference_wrapper<ErrT> m_data;

        public:
            
            explicit unexpect_ref(dp::reference_wrapper<ErrT> in) : m_data(in) {}

            ErrT& value() const {
                return m_data.get();
            }
        };
    }

    //Unexpected. For construction and holding of an unexpected value
    template<typename ErrT>
    class unexpected{
        ErrT stored_value;

    public:
        unexpected(const dp::unexpected<ErrT>& other) : stored_value(other.stored_value) {}
        template<typename U>
        unexpected(const U& other) : stored_value(other) {}

        template<typename U>
        unexpected(dp::detail::unexpect_ref<U> other) : stored_value(other.value()) {}

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

    struct unexpect_t {
        unexpect_t() {}
    };

    static const unexpect_t unexpect;




    template<typename ValT, typename ErrT>
    class expected {
    public:

        typedef ValT                    value_type;
        typedef ErrT                    error_type;
        typedef dp::unexpected<ErrT>    unexpected_type;
        //No using alias so we make do
        template<typename U>
        struct rebind {
            typedef typename dp::expected<U, error_type> type;
        };

        expected() : m_storage(value_type()), m_holds_value_type(true) {}

        expected(const expected& other) : m_holds_value_type(other.has_value()) {
            if (has_value()) m_storage.template construct<value_type>(*other);
            else m_storage.template construct<error_type>(other.error());
        }

        template<typename U, typename G>
        explicit expected(const dp::expected<U, G>& other) : m_holds_value_type(other.has_value()) {
            if (has_value()) m_storage.template construct<value_type>(*other);
            else m_storage.template construct<error_type>(other.error());
        }

        template<typename U>
        expected(const U& in) : m_holds_value_type(true) {
            m_storage.template construct<value_type>(in);
        }

        template<typename U>
        expected(const dp::unexpected<U>& other) : m_holds_value_type(false) {
            m_storage.template construct<error_type>(other.error());
        }

        template<typename U>
        expected(dp::detail::unexpect_ref<U> other) : m_holds_value_type(false) {
            m_storage.template construct<error_type>(other.value());
        }

        expected(dp::unexpect_t, const error_type& other) : m_holds_value_type(false) {
            m_storage.template construct<error_type>(other);
        }

        ~expected() {
            if (has_value()) m_storage.template destroy<value_type>();
            else m_storage.template destroy<error_type>();
        }

        expected& operator=(const expected& other) {
            expected copy(other);
            this->swap(copy);
            return *this;
        }

        template<typename U>
        expected& operator=(const U& in) {
            expected copy(in);
            this->swap(copy);
            return *this;
        }

        void swap(expected& other) {
            using std::swap;
            if (this->has_value() && other.has_value()) {
                m_storage.template swap<value_type>(other.m_storage);
            }
            else if (other.has_value()) {
                //Exception safety! My old nemesis.
                //This is the type we use to store data. Guaranteed to be big enough to fit
                typedef typename dp::unbound_storage<dp::type_sizes<value_type, error_type>::larger_size> storage_type;

                storage_type temp_val(*other);
                storage_type temp_err(this->error());

                //Assume a non-throwing destructor.
                this->m_storage.template destroy<error_type>();
                other.m_storage.template destroy<value_type>();

                //And nothrow swap them in.
                this->m_storage.template swap<value_type>(temp_val);
                other.m_storage.template swap<error_type>(temp_err);

                swap(m_holds_value_type, other.m_holds_value_type);
            }
            else if (this->has_value()) {
                other.swap(*this);
            }
            else {
                m_storage.template swap<error_type>(other.m_storage);
            }        
        }

        value_type& operator*() {
            return get();
        }
        const value_type& operator*() const {
            return get();
        }

        value_type* operator->() {
            return &get();
        }
        const value_type* operator->() const {
            return &get();
        }

        value_type& value() {
            if (!has_value()) throw dp::bad_expected_access<error_type>(this->error());
            return get();
        }
        const value_type& value() const {
            if (!has_value()) throw dp::bad_expected_access<error_type>(this->error());
            return get();
        }

        template<typename U>
        value_type value_or(const U& in) const {
            if (!has_value()) return in;
            return get();
        }

        error_type& error() {
            return m_storage.template get<error_type>();
        }
        const error_type& error() const {
            return m_storage.template get<error_type>();
        }

        template<typename U>
        error_type error_or(const U& in) const {
            if (has_value()) return in;
            return error();
        }

        bool has_value() const {
            return m_holds_value_type;
        }

#if defined(DP_BORLAND) && __BORLANDC__ >= 0x0730
        explicit
#endif
        operator bool() const {
            return has_value();
        }



    private:

        dp::unbound_storage<dp::type_sizes<value_type, error_type>::larger_size> m_storage;
        bool m_holds_value_type;

        inline value_type& get() {
            return m_storage.template get<value_type>();
        }
        inline const value_type& get() const {
            return m_storage.template get<value_type>();
        }
    };

    //Partial specialisation for void
    template<typename ErrT>
    class expected<void, ErrT> {
    public:
        //In terms of implementation, this can be a little different. A void expected is in many ways just an optional where the optional is full in an error case
        //This makes for an easier implementation, and means we can rule-of-zero our copy operations

        typedef void                    value_type;
        typedef ErrT                    error_type;
        typedef dp::unexpected<ErrT>    unexpected_type;
        template<typename U>
        struct rebind {
            typedef typename dp::expected<U, error_type> type;
        };

        expected() {}

        template<typename U, typename G>
        explicit expected(const dp::expected<U, G>& other) {
            //Avoid the formal UB trap. Capture the other's error if it has one
            if (!other.has_value()) m_error = static_cast<ErrT>(other.error());
        }

        template<typename U>
        expected(const dp::unexpected<U>& other) : m_error(other.value()) {}

        template<typename U>
        expected(dp::detail::unexpect_ref<U> other) : m_error(other.value()) {}        

        expected(dp::unexpect_t, const error_type& other) : m_error(other) {}

        template<typename U>
        expected& operator=(const U& in) {
            expected copy(in);
            this->swap(copy);
            return *this;
        }

        void swap(expected& other) {
            m_error.swap(other.m_error);
        }

        void operator*() const {}

        void value() const {
            if (!has_value()) throw dp::bad_expected_access<error_type>(this->error());
        }

        error_type& error() {
            return *m_error;
        }
        const error_type& error() const {
            return *m_error;
        }

        template<typename U>
        error_type error_or(const U& in) {
            if (has_value()) return in;
            return error();
        }

        bool has_value() const {
            return !m_error.has_value();
        }

#if defined(DP_BORLAND) && __BORLANDC__ >= 0x0730
        explicit
#endif
        operator bool() const {
            return has_value();
        }


    private:

        dp::optional<ErrT> m_error;


    };



    //Undefined to prevent use
    template<typename T, typename ErrT>
    class expected<dp::unexpected<T>, ErrT>;

    template<typename ValT, typename ErrT>
    class expected<ValT&, ErrT>;

    template<typename ValT, typename ErrT>
    class expected<ValT, ErrT&>;

    template<typename ValT, typename ErrT>
    class expected<ValT&, ErrT&>;

    template<typename ValT>
    class expected<ValT, void>;


    //Swapping
    template<typename ValT, typename ErrT>
    void swap(dp::expected<ValT,ErrT>& lhs, dp::expected<ValT,ErrT>& rhs){
        lhs.swap(rhs);
    }

    //Equality comparison
    template<typename V1, typename E1, typename V2, typename E2>
    bool operator==(const expected<V1, E1>& lhs, const expected<V2, E2>& rhs) {
        return lhs.has_value() && rhs.has_value() && *lhs == *rhs;
    }

    template<typename V1, typename E1, typename V2>
    bool operator==(const expected<V1, E1>& lhs, const V2& rhs) {
        return lhs.has_value() && *lhs == rhs;
    }

    template<typename V1, typename E1, typename E2>
    bool operator==(const expected<V1, E1>& lhs, const unexpected<E2>& rhs) {
        return !lhs.has_value() && lhs.error() == rhs.error();
    }


    //An alternative to CTAD 
    //We can't just return a reference to in, as that will be construed as a success type
    //We need a named "error" type and want to avoid constructing an unexpected, copying it out the function, then copying the value into the expected
    //Yes, I miss move semantics and mandatory copy elision too
    template<typename ErrT>
    typename dp::detail::unexpect_ref<ErrT> unex(const ErrT& in) {
        return dp::detail::unexpect_ref<ErrT>(dp::ref(const_cast<ErrT&>(in)));
    }

}

#endif