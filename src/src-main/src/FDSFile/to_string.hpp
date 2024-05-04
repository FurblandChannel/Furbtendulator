// Replacement for broken to_string function in MingW
#ifndef INCLUDED_TOSTRING_HPP
#define INCLUDED_TOSTRING_HPP 1

#ifdef _GLIBCXX_HAVE_BROKEN_VSWPRINTF

#include <string>
#include <sstream>
namespace std {
    template < typename T > std::string to_string( const T& n ) {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}
#endif // ifdef _GLIBCXX_HAVE_BROKEN_VSWPRINTF
#endif // #ifndef INCLUDED_TOSTRING_HPP