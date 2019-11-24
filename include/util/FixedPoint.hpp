/*
** Copyright 2019 Jakob Kellendonk
**
** Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <type_traits>
#include <cmath>

template<int exponent, class StoreType = unsigned int>
class FixedPoint
{
  template<int shift, class Type>
  static constexpr Type logicalLShift(Type val) noexcept
  {
    if constexpr (std::is_signed_v<Type>)
      {
	if (val > 0)
	  return logicalLShift<shift>(std::make_unsigned_t<Type>(val));
	else
	  return -logicalLShift<shift>(std::make_unsigned_t<Type>(-val));
      }
    else
      {
	if constexpr (shift < 0)
          return val >> -shift;
	else
	  return val << shift;
      }
  }
  
public:
  StoreType value;

  static constexpr const FixedPoint One{logicalLShift<-exponent>(1u)};
  static constexpr const FixedPoint Zero{0};

  constexpr FixedPoint() noexcept
  : value(0)
  {
  }

  explicit constexpr FixedPoint(StoreType value) noexcept
  : value(value)
  {
  }

  template<int otherExponent, class OtherStoreType>
  constexpr FixedPoint(FixedPoint<otherExponent, OtherStoreType> other) noexcept
  : value(logicalLShift<otherExponent - exponent>(other.value))
  {
    static_assert(otherExponent - exponent <= 16 && exponent - otherExponent <= 16); // sanity to not lose to much precision
  }

  // constexpr FixedPoint &operator*=(FixedPoint<0, StoreType> const &other) noexcept
  // {
  //   value *= other.value;
  //   return *this;
  // }

  template<int otherExponent, class OtherType>
  constexpr auto operator*(FixedPoint<otherExponent, OtherType> const &other) const noexcept
  {	
    return FixedPoint<exponent + otherExponent, decltype(value * other.value)>(value * other.value);
  }

  template<int otherExponent, class OtherType>
  constexpr FixedPoint &operator*=(FixedPoint<otherExponent, OtherType> const &other) noexcept
  {
    return *this = FixedPoint(*this * other);
  }

  template<int otherExponent, class OtherType>
  constexpr FixedPoint &operator/=(FixedPoint<0, OtherType> const &other) noexcept
  {
    value /= other.value;
    return *this;
  }

  template<int otherExponent, class OtherType>
  constexpr FixedPoint<exponent - otherExponent, StoreType> operator/(FixedPoint<otherExponent, OtherType> const &other) const noexcept
  {
    return FixedPoint<exponent - otherExponent>(value / other.value);
  }

  template<int otherExponent, class OtherType>
  constexpr FixedPoint &operator/=(FixedPoint<otherExponent, OtherType> const &other) noexcept
  {
    return *this = FixedPoint(*this / other);
  }

  template<int otherExponent, class OtherType>
  constexpr FixedPoint &operator-=(FixedPoint<otherExponent, OtherType> const &other) noexcept
  {
    value -= FixedPoint<exponent, OtherType>(other).value;
    return *this;
  }

  template<class OtherType>
  constexpr FixedPoint operator-(FixedPoint<exponent, OtherType> const &other) const noexcept
  {
    return FixedPoint(value - other.value);
  }

  template<int otherExponent, class OtherType>
  constexpr FixedPoint &operator+=(FixedPoint<otherExponent, OtherType> const &other) noexcept
  {
    value += FixedPoint<exponent, OtherType>(other).value;
    return *this;
  }

  template<class OtherType>
  constexpr FixedPoint operator+(FixedPoint<exponent, OtherType> const &other) const noexcept
  {
    return FixedPoint(value + other.value);
  }

  template<class OtherType>
  constexpr FixedPoint operator%(FixedPoint<exponent, OtherType> const &other) const noexcept
  {
    return FixedPoint(value % other.value);
  }

  constexpr FixedPoint operator-() const noexcept
  {
    return FixedPoint(-value);
  }

  constexpr FixedPoint floor() const noexcept
  {
    return FixedPoint<0, StoreType>(*this);
  }

  constexpr bool isPositive() const noexcept
  {
    return value > 0;
  }

  constexpr bool isNegative() const noexcept
  {
    return value < 0;
  }

  constexpr bool operator!() const noexcept
  {
    return !value;
  }

#define FIXEDPOINT_RELTATIVE_OP(OP)					\
  template<int otherExponent, class OtherType>				\
  constexpr bool operator OP (FixedPoint<otherExponent, OtherType> const &other) const noexcept \
  {									\
    if constexpr (otherExponent == exponent)				\
      return value OP other.value;					\
    else if constexpr (exponent > otherExponent)			\
      {									\
	if (value != (other.value >> (exponent - otherExponent)))	\
	  return value OP (other.value >> (exponent - otherExponent));	\
	return (value << (exponent - otherExponent)) OP other.value;	\
      }									\
    else								\
      {									\
	static_assert(otherExponent > exponent);			\
	if ((value >> (otherExponent - exponent)) != other.value)	\
	  return (value >> (otherExponent - exponent)) OP other.value;	\
	return value OP (other.value << (otherExponent - exponent));	\
}									\
  }

  FIXEDPOINT_RELTATIVE_OP(==);
  FIXEDPOINT_RELTATIVE_OP(!=);
  FIXEDPOINT_RELTATIVE_OP(<=);
  FIXEDPOINT_RELTATIVE_OP(>=);
  FIXEDPOINT_RELTATIVE_OP(>);
  FIXEDPOINT_RELTATIVE_OP(<);

  constexpr float getFloatValue() const noexcept
  {
    return float(std::ldexp(float(value), exponent));
  }

  constexpr double getDoubleValue() const noexcept
  {
    return double(std::ldexp(double(value), exponent));
  }
};

template<class Type>
constexpr FixedPoint<0, Type> makeFixedPoint(Type &&type)
{
  return FixedPoint<0, Type>(type);
}


constexpr FixedPoint<0> operator ""_uFP(long long unsigned int value) noexcept
{
  return FixedPoint<0>(unsigned(value));
}

constexpr FixedPoint<0, int> operator ""_FP(long long unsigned int value) noexcept
{
  return FixedPoint<0, int>(int(value));
}
