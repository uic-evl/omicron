#ifndef BOOST_SMART_PTR_INTRUSIVE_PTR_HPP_INCLUDED
#define BOOST_SMART_PTR_INTRUSIVE_PTR_HPP_INCLUDED

//
//  Ref.hpp
//
//  Copyright (c) 2001, 2002 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/smart_ptr/intrusive_ptr.html for documentation.
//

#include <boost/config.hpp>

#ifdef BOOST_MSVC  // moved here to work around VC++ compiler crash
# pragma warning(push)
# pragma warning(disable:4284) // odd return type for operator->
#endif

#include <boost/assert.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/smart_ptr/detail/sp_convertible.hpp>

#include <boost/config/no_tr1/functional.hpp>           // for std::less

#if !defined(BOOST_NO_IOSTREAM)
#if !defined(BOOST_NO_IOSFWD)
#include <iosfwd>               // for std::basic_ostream
#else
#include <ostream>
#endif
#endif


namespace omicron
{

//
//  intrusive_ptr
//
//  A smart pointer that uses intrusive reference counting.
//
//  Relies on unqualified calls to
//  
//      void intrusive_ptr_add_ref(T * p);
//      void intrusive_ptr_release(T * p);
//
//          (p != 0)
//
//  The object is responsible for destroying itself.
//

template<class T> class Ref
{
private:

    typedef Ref this_type;

public:

    typedef T element_type;

    Ref(): px( 0 )
    {
    }

    Ref( T * p, bool add_ref = true ): px( p )
    {
        if( px != 0 && add_ref ) intrusive_ptr_add_ref( px );
    }

#if !defined(BOOST_NO_MEMBER_TEMPLATES) || defined(BOOST_MSVC6_MEMBER_TEMPLATES)

    template<class U>
#if !defined( BOOST_SP_NO_SP_CONVERTIBLE )

    Ref( Ref<U> const & rhs, typename boost::detail::sp_enable_if_convertible<U,T>::type = boost::detail::sp_empty() )

#else

    intrusive_ptr( intrusive_ptr<U> const & rhs )

#endif
    : px( rhs.get() )
    {
        if( px != 0 ) intrusive_ptr_add_ref( px );
    }

#endif

    Ref(Ref const & rhs): px( rhs.px )
    {
        if( px != 0 ) intrusive_ptr_add_ref( px );
    }

    ~Ref()
    {
        if( px != 0 ) intrusive_ptr_release( px );
    }

#if !defined(BOOST_NO_MEMBER_TEMPLATES) || defined(BOOST_MSVC6_MEMBER_TEMPLATES)

    template<class U> Ref & operator=(Ref<U> const & rhs)
    {
        this_type(rhs).swap(*this);
        return *this;
    }

#endif

// Move support

#if defined( BOOST_HAS_RVALUE_REFS )

    Ref(Ref && rhs): px( rhs.px )
    {
        rhs.px = 0;
    }

    Ref & operator=(Ref && rhs)
    {
        this_type( static_cast< Ref && >( rhs ) ).swap(*this);
        return *this;
    }

#endif

    Ref & operator=(Ref const & rhs)
    {
        this_type(rhs).swap(*this);
        return *this;
    }

    Ref & operator=(T * rhs)
    {
        this_type(rhs).swap(*this);
        return *this;
    }

    void reset()
    {
        this_type().swap( *this );
    }

    void reset( T * rhs )
    {
        this_type( rhs ).swap( *this );
    }

    T * get() const
    {
        return px;
    }

	bool isNull() const { return px == NULL; }

 /*   T & operator*() const
    {
        BOOST_ASSERT( px != 0 );
        return *px;
    }
*/
    T * operator->() const
    {
        BOOST_ASSERT( px != 0 );
        return px;
    }

	operator T*() const { return px; }

// implicit conversion to "bool"
//#include <boost/smart_ptr/detail/operator_bool.hpp>

    void swap(Ref & rhs)
    {
        T * tmp = px;
        px = rhs.px;
        rhs.px = tmp;
    }

private:

    T * px;
};

// We use the implicit pointer conversion operator instead of these.
//template<class T, class U> inline bool operator==(Ref<T> const & a, Ref<U> const & b)
//{
//    return a.get() == b.get();
//}
//
//template<class T, class U> inline bool operator!=(Ref<T> const & a, Ref<U> const & b)
//{
//    return a.get() != b.get();
//}
//
//template<class T, class U> inline bool operator==(Ref<T> const & a, U * b)
//{
//    return a.get() == b;
//}
//
//template<class T, class U> inline bool operator!=(Ref<T> const & a, U * b)
//{
//    return a.get() != b;
//}
//
//template<class T, class U> inline bool operator==(T * a, Ref<U> const & b)
//{
//    return a == b.get();
//}

template<class T, class U> inline bool operator!=(T * a, Ref<U> const & b)
{
    return a != b.get();
}

#if __GNUC__ == 2 && __GNUC_MINOR__ <= 96

// Resolve the ambiguity between our op!= and the one in rel_ops

template<class T> inline bool operator!=(Ref<T> const & a, Ref<T> const & b)
{
    return a.get() != b.get();
}

#endif

template<class T> inline bool operator<(Ref<T> const & a, Ref<T> const & b)
{
    return std::less<T *>()(a.get(), b.get());
}

template<class T> void swap(Ref<T> & lhs, Ref<T> & rhs)
{
    lhs.swap(rhs);
}

// mem_fn support

template<class T> T * get_pointer(Ref<T> const & p)
{
    return p.get();
}

template<class T, class U> Ref<T> static_pointer_cast(Ref<U> const & p)
{
    return static_cast<T *>(p.get());
}

template<class T, class U> Ref<T> const_pointer_cast(Ref<U> const & p)
{
    return const_cast<T *>(p.get());
}

template<class T, class U> Ref<T> dynamic_pointer_cast(Ref<U> const & p)
{
    return dynamic_cast<T *>(p.get());
}

// operator<<

#if !defined(BOOST_NO_IOSTREAM)

#if defined(BOOST_NO_TEMPLATED_IOSTREAMS) || ( defined(__GNUC__) &&  (__GNUC__ < 3) )

template<class Y> std::ostream & operator<< (std::ostream & os, Ref<Y> const & p)
{
    os << p.get();
    return os;
}

#else

// in STLport's no-iostreams mode no iostream symbols can be used
#ifndef _STLP_NO_IOSTREAMS

# if defined(BOOST_MSVC) && BOOST_WORKAROUND(BOOST_MSVC, < 1300 && __SGI_STL_PORT)
// MSVC6 has problems finding std::basic_ostream through the using declaration in namespace _STL
using std::basic_ostream;
template<class E, class T, class Y> basic_ostream<E, T> & operator<< (basic_ostream<E, T> & os, Ref<Y> const & p)
# else
template<class E, class T, class Y> std::basic_ostream<E, T> & operator<< (std::basic_ostream<E, T> & os, Ref<Y> const & p)
# endif 
{
    os << p.get();
    return os;
}

#endif // _STLP_NO_IOSTREAMS

#endif // __GNUC__ < 3

#endif // !defined(BOOST_NO_IOSTREAM)

} // namespace boost

#ifdef BOOST_MSVC
# pragma warning(pop)
#endif    

#endif  // #ifndef BOOST_SMART_PTR_INTRUSIVE_PTR_HPP_INCLUDED
