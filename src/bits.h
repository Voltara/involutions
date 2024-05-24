#ifndef INVL_BITS_H
#define INVL_BITS_H

#include <numeric>

template<typename T>
struct bits {
	T mask;
	bits(T mask) : mask(mask)               { }
	bits & operator++ ()                    { mask &= mask - 1; return *this; }
	bool operator!= (const bits &rhs) const { return mask != rhs.mask; }
	int operator* () const                  { return std::countr_zero(mask); }
	bits begin() const                      { return mask; }
	bits end() const                        { return T(0); }
};

#endif
