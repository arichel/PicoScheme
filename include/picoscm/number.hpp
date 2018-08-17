/*********************************************************************************/ /**
 * @file number.hpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#ifndef NUMBER_HPP
#define NUMBER_HPP

#include <complex>
#include <iostream>
#include <variant>

#include "utils.hpp"

namespace pscm {

using Int = int64_t;
using Float = double;
using Complex = std::complex<double>;

template <typename T>
struct Type;

/**
 * @brief Number struct as union of integer, floating point and complex numbers.
 *
 * A floating point number it converted into an integer if it is
 * exact representable as an integer. If the imaginary part of a complex
 * number is zero, only a number representing the real part is
 * constructed.
 */
struct Number : std::variant<Int, Float, Complex> {
    using base_type = std::variant<Int, Float, Complex>;
    using base_type::operator=;

    constexpr Number()
        : base_type{ Int{ 0 } }
    {
    }

    Number(const Number&) = default;
    Number& operator=(const Number&) = default;
    Number& operator=(Number&&) = default;

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    constexpr Number(T x)
        : base_type{ static_cast<Int>(x) }
    {
    }
    /**
     * @brief Converting constructor for arithmetic type arguments.
     */
    constexpr Number(Float x)
    {
        if (x > floor(x) || x < ceil(x) || std::abs(x) > static_cast<Float>(std::numeric_limits<Int>::max()))
            *this = base_type{ x };
        else
            *this = static_cast<Int>(x);
    }
    /**
     * @brief Converting constructor for complex type arguments.
     */
    constexpr Number(const Complex& z)
        : Number{ z.real(), z.imag() }
    {
    }

    template <typename RE, typename IM>
    constexpr Number(RE x, IM y)
    {
        if (y > IM{ 0 } || y < IM{ 0 })
            *this = base_type{ Complex{ static_cast<Float>(x), static_cast<Float>(y) } };
        else
            *this = Number{ x };
    }

    /**
     * @brief Conversion operator to convert a Number type to the requested arithmetic or complex type.
     */
    template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T> || std::is_same_v<T, Complex>>>
    explicit constexpr operator T() const noexcept
    {
        auto fun = [](auto& num) -> T {
            using TT = std::decay_t<decltype(num)>;

            if
                constexpr(std::is_same_v<TT, Int>)
                {
                    if
                        constexpr(std::is_integral_v<T>) return static_cast<T>(num);

                    else if
                        constexpr(std::is_floating_point_v<T>) return static_cast<T>(num);

                    else // T is Complex:
                        return static_cast<T>(static_cast<typename T::value_type>(num));
                }
            else if
                constexpr(std::is_same_v<TT, Float>)
                {
                    if
                        constexpr(std::is_arithmetic_v<T>) return static_cast<T>(num);

                    else // T is Complex
                        return static_cast<T>(static_cast<typename T::value_type>(num));
                }
            else if
                constexpr(std::is_same_v<TT, Complex>)
                {
                    if
                        constexpr(std::is_arithmetic_v<T>) return static_cast<T>(std::abs(num));

                    else
                        return static_cast<T>(num);
                }
            else
                static_assert(always_false<TT>::value, "invalid variant");
        };

        return std::visit(std::move(fun), static_cast<const base_type&>(*this));
    }
};

template <typename T>
Number num(const T& x) { return { x }; }

template <typename RE, typename IM>
Number num(const RE& x, const IM& y) { return { x, y }; }

constexpr bool is_int(const Number& num) { return is_type<Int>(num); }
constexpr bool is_float(const Number& num) { return is_type<Float>(num); }
constexpr bool is_complex(const Number& num) { return is_type<Complex>(num); }

bool is_integer(const Number& num);
bool is_odd(const Number& num);

constexpr inline Number operator""_int(unsigned long long val) { return static_cast<Int>(val); }
constexpr inline Number operator""_flo(unsigned long long val) { return static_cast<Int>(val); }
constexpr inline Number operator""_cpx(unsigned long long val) { return Number{ 0., static_cast<Float>(val) }; }
constexpr inline Number operator""_flo(long double val) { return static_cast<Float>(val); }
constexpr inline Number operator""_cpx(long double val) { return Number{ 0., static_cast<Float>(val) }; }

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const Complex& z)
{
    if (auto im = z.imag(); im > 0 || im < 0) {
        if (im < 0) {
            os << z.real();
            if (im > -1 || im < -1)
                return os << '-' << im << 'i';
            else
                return os << "-i";

        } else if (im > 1 || im < 1)
            return os << z.real() << '+' << im << 'i';
        else
            return os << z.real() << "+i";
    } else
        return os << z.real();
}

/**
 * @brief Out stream operator for a ::Number argument value.
 */
template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const Number& num)
{
    return std::visit([&os](auto x) -> decltype(os) { return os << x; },
        static_cast<Number::base_type>(num));
}

bool operator!=(const Number& lhs, const Number& rhs);
bool operator==(const Number& lhs, const Number& rhs);
bool operator>(const Number& lhs, const Number& rhs);
bool operator<(const Number& lhs, const Number& rhs);
bool operator>=(const Number& lhs, const Number& rhs);
bool operator<=(const Number& lhs, const Number& rhs);

inline bool is_zero(const Number& x) { return !(x != Number{ 0 }); }
inline bool is_negative(const Number& x) { return x < Number{ 0 }; }
inline bool is_positive(const Number& x) { return x > Number{ 0 }; }

inline Number min(const Number& lhs, const Number& rhs)
{
    return rhs < lhs ? rhs : lhs;
}

inline Number max(const Number& lhs, const Number& rhs)
{
    return rhs > lhs ? rhs : lhs;
}

Number inv(const Number& x);

Number operator-(const Number& x);
Number operator+(const Number& lhs, const Number& rhs);
Number operator-(const Number& lhs, const Number& rhs);
Number operator*(const Number& lhs, const Number& rhs);
Number operator/(const Number& lhs, const Number& rhs);
Number operator%(const Number& lhs, const Number& rhs);

Number& operator+=(Number& lhs, const Number& rhs);
Number& operator-=(Number& lhs, const Number& rhs);
Number& operator*=(Number& lhs, const Number& rhs);
Number& operator/=(Number& lhs, const Number& rhs);

Number round(const Number& x);
Number floor(const Number& x);
Number ceil(const Number& x);
Number trunc(const Number& x);
Number remainder(const Number& lhs, const Number& rhs);
Number quotient(const Number& lhs, const Number& rhs);

Number sin(const Number& x);
Number cos(const Number& x);
Number tan(const Number& x);
Number asin(const Number& x);
Number acos(const Number& x);
Number atan(const Number& x);
Number sinh(const Number& x);
Number cosh(const Number& x);
Number tanh(const Number& x);
Number asinh(const Number& x);
Number acosh(const Number& x);
Number atanh(const Number& x);

Number exp(const Number& x);
Number log(const Number& x);
Number log10(const Number& x);
Number sqrt(const Number& x);
Number cbrt(const Number& x);
Number pow(const Number& x, const Number& y);

Number abs(const Number& x);
Number real(const Number& z);
Number imag(const Number& z);
Number arg(const Number& z);
Number conj(const Number& z);
Number rect(const Number& x, const Number& y);
Number polar(const Number& r, const Number& theta);
Number hypot(const Number& x, const Number& y);
Number hypot(const Number& x, const Number& y, const Number& z);

} // namspace pscm
#endif // NUMBER_HPP
