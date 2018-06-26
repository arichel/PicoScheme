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
#include <variant>

namespace pscm {

using Int = int64_t;
using Float = double;
using Complex = std::complex<double>;

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

    /**
     * @brief Converting constructor for arithmetic type arguments.
     */
    template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
    Number(T x)
    {
        if constexpr (std::is_integral_v<T>)
            *this = static_cast<Int>(x);
        else {
            if (x != std::trunc(x))
                *this = static_cast<Float>(x);
            else
                *this = static_cast<Int>(x);
        }
    }

    /**
     * @brief Converting constructor for complex type arguments.
     */
    Number(const Complex& z)
        : Number{ z.real(), z.imag() }
    {
    }

    template <typename RE, typename IM>
    Number(RE x, IM y)
    {
        if (y != Float{ 0 })
            *this = Complex{ static_cast<Float>(x), static_cast<Float>(y) };
        else
            *this = Number{ x };
    }

    /**
     * @brief Conversion operator to convert a Number type to the requested arithmetic or complex type.
     */
    template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T> || std::is_convertible_v<T, Complex>>>
    constexpr operator T() const
    {
        auto fun = [](const auto& num) -> T {
            using TT = std::decay_t<decltype(num)>;

            if constexpr (std::is_same_v<TT, Int>) {
                if constexpr (std::is_integral_v<T>)
                    return static_cast<T>(num);
                else if constexpr (std::is_floating_point_v<T>)
                    return static_cast<T>(num);
                else
                    return static_cast<T>((typename T::value_type)num);

            } else if constexpr (std::is_same_v<TT, Float>) {
                if constexpr (std::is_arithmetic_v<T>)
                    return static_cast<T>(num);
                else
                    return static_cast<T>((typename T::value_type)num);

            } else if constexpr (std::is_same_v<TT, Complex>) {
                if constexpr (std::is_arithmetic_v<T>)
                    return static_cast<T>(std::abs(num));
                else
                    return static_cast<T>(num);

            } else
                static_assert(always_false<TT>::value, "invalid variant");
        };

        return std::visit(std::move(fun), static_cast<const base_type&>(*this));
    }
};

/**
 * @brief Out stream operator for a ::Number argument value.
 */
template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const Number& num)
{
    overloads fun{
        [&os](const Complex& z) { os << z.real() << '+' << z.imag() << 'i'; },
        [&os](auto x) { os << x; }
    };
    std::visit(std::move(fun), static_cast<const Number::base_type&>(num));
    return os;
}

bool operator!=(const Number& lhs, const Number& rhs);
bool operator==(const Number& lhs, const Number& rhs);

Number operator+(const Number& lhs, const Number& rhs);
Number operator-(const Number& lhs, const Number& rhs);
Number operator*(const Number& lhs, const Number& rhs);
Number operator/(const Number& lhs, const Number& rhs);

Number& operator+=(Number& lhs, const Number& rhs);
Number& operator-=(Number& lhs, const Number& rhs);
Number& operator*=(Number& lhs, const Number& rhs);
Number& operator/=(Number& lhs, const Number& rhs);

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
Number pow(const Number& x, const Number& y);

Number abs(const Number& x);
Number real(const Number& z);
Number imag(const Number& z);
Number arg(const Number& z);
Number conj(const Number& z);
Number polar(const Number& r, const Number& theta);

}; // namspace pscm
#endif // NUMBER_HPP
