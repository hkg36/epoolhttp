/*
 * stdext.h
 *
 *  Created on: Apr 16, 2012
 *      Author: amen
 */
#include <string>
#include <string.h>
#include <set>
#include <list>
#include <map>
#include <queue>
#include "ByteStream.h"
#include "CBuffer.h"
#include "IPtr.h"
#include "locks.h"
#ifndef STDEXT_H_
#define STDEXT_H_
#define UPROUND(n,base) (((n)+((base)-1))/(base)*(base))
#define _countof(x) (sizeof(x)/sizeof(x[0]))
struct string_less_nocase {
    bool operator() ( const std::string& a, const std::string& b ) const {
        return strcasecmp ( a.c_str(),b.c_str() ) <0;
    }
};
namespace std
{
template<typename _ForwardIterator, typename _Tp, typename _ComparerLess>
_ForwardIterator lower_bound ( _ForwardIterator __first, _ForwardIterator __last,
                               const _Tp& __val, const _ComparerLess& cmp )
{
    typedef typename iterator_traits<_ForwardIterator>::value_type _ValueType;
    typedef typename iterator_traits<_ForwardIterator>::difference_type _DistanceType;

    // concept requirements
    __glibcxx_function_requires ( _ForwardIteratorConcept<_ForwardIterator> )
    __glibcxx_function_requires ( _LessThanOpConcept<_ValueType, _Tp> )
    __glibcxx_requires_partitioned_lower ( __first, __last, __val );

    _DistanceType __len = std::distance ( __first, __last );

    while ( __len > 0 ) {
        _DistanceType __half = __len >> 1;
        _ForwardIterator __middle = __first;
        std::advance ( __middle, __half );
        if ( cmp ( *__middle, __val ) ) {
            __first = __middle;
            ++__first;
            __len = __len - __half - 1;
        } else {
            __len = __half;
        }
    }
    return __first;
}
}

#endif /* STDEXT_H_ */
