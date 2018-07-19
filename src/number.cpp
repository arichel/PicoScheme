/*********************************************************************************/ /**
 * @file number.cpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#include <assert.h>
#include <cmath>
#include <limits>

#include "number.hpp"
#include "utils.hpp"

namespace pscm {

/**
 * @brief Check wheter an integer addition of both argument values would overflow.
 */
constexpr bool overflow_add(Int a, Int b)
{
    constexpr Int min = std::numeric_limits<Int>::min(),
                  max = std::numeric_limits<Int>::max();

    return (b > 0 && a > max - b) || (b < 0 && a < min - b);
}

/**
 * @brief Check wheater integer substraction of both arguments values would overflow.
 */
constexpr bool overflow_sub(Int a, Int b)
{
    constexpr Int min = std::numeric_limits<Int>::min(),
                  max = std::numeric_limits<Int>::max();

    return (b > 0 && a < min + b) || (b < 0 && a > max + b);
}

/**
 * @brief Predicate function to test wheter the argument numbers aren't equal.
 */
bool operator!=(const Number& lhs, const Number& rhs)
{
    using value_type = Complex::value_type;

    return visit([](auto x, auto y) {
        if
            constexpr(std::is_same_v<Complex, decltype(x)> && !std::is_same_v<Complex, decltype(y)>) return x != Complex{ (value_type)y, 0 };

        else if
            constexpr(!std::is_same_v<Complex, decltype(x)> && std::is_same_v<Complex, decltype(y)>) return Complex{ (value_type)x, 0 } != y;

        else {
            using T = std::common_type_t<decltype(x), decltype(y)>;
            return (T)x != (T)y;
        }
    },
        static_cast<const Number::base_type&>(lhs), static_cast<const Number::base_type&>(rhs));
}

/**
 * @brief Predicate function to test wheter the argument numbers are equal.
 */
bool operator==(const Number& lhs, const Number& rhs)
{
    return !(lhs != rhs);
}

bool operator<(const Number& lhs, const Number& rhs)
{
    return visit([](auto x, auto y) {
        if
            constexpr(!std::is_same_v<Complex, decltype(x)> && !std::is_same_v<Complex, decltype(y)>)
            {
                using T = std::common_type_t<decltype(x), decltype(y)>;
                return (T)x < (T)y;
            }
        else
            return (throw std::invalid_argument("uncomparable complex number"), false);
    },
        static_cast<const Number::base_type&>(lhs), static_cast<const Number::base_type&>(rhs));
}

bool operator>(const Number& lhs, const Number& rhs)
{
    return visit([](auto x, auto y) {
        if
            constexpr(!std::is_same_v<Complex, decltype(x)> && !std::is_same_v<Complex, decltype(y)>)
            {
                using T = std::common_type_t<decltype(x), decltype(y)>;
                return (T)x > (T)y;
            }
        else
            return (throw std::invalid_argument("uncomparable complex number"), false);
    },
        static_cast<const Number::base_type&>(lhs), static_cast<const Number::base_type&>(rhs));
}

bool operator<=(const Number& lhs, const Number& rhs)
{
    return visit([](auto x, auto y) {
        if
            constexpr(!std::is_same_v<Complex, decltype(x)> && !std::is_same_v<Complex, decltype(y)>)
            {
                using T = std::common_type_t<decltype(x), decltype(y)>;
                return (T)x <= (T)y;
            }
        else
            return (throw std::invalid_argument("uncomparable complex number"), false);
    },
        static_cast<const Number::base_type&>(lhs), static_cast<const Number::base_type&>(rhs));
}

bool operator>=(const Number& lhs, const Number& rhs)
{
    return visit([](auto x, auto y) {
        if
            constexpr(!std::is_same_v<Complex, decltype(x)> && !std::is_same_v<Complex, decltype(y)>)
            {
                using T = std::common_type_t<decltype(x), decltype(y)>;
                return (T)x >= (T)y;
            }
        else
            return (throw std::invalid_argument("uncomparable complex number"), false);
    },
        static_cast<const Number::base_type&>(lhs), static_cast<const Number::base_type&>(rhs));
}

Number inv(const Number& x)
{
    x != Number{} || (throw std::invalid_argument("divide by zero"), 0);

    return visit([](auto& x) -> Number {
        if
            constexpr(std::is_same_v<const Complex&, decltype(x)>) return 1 / x;
        else
            return 1 / (Float)x;
    },
        static_cast<const Number::base_type&>(x));
}

Number operator-(const Number& x)
{
    return visit([](auto& x) -> Number { return -x; },
        static_cast<const Number::base_type&>(x));
}

/**
 * @brief Number addition operator.
 *
 * In case of an integer overflow, the returned number is converted into a floating point sum.
 */
Number operator+(const Number& lhs, const Number& rhs)
{
    using value_type = Complex::value_type;

    constexpr auto fun = overloads{
        [](const Complex& z0, const Complex& z1) -> Number { return z0 + z1; },
        [](const Complex& z, auto x) -> Number { return z + (value_type)x; },
        [](auto x, const Complex& z) -> Number { return (value_type)x + z; },
        [](Int i0, Int i1) -> Number {
            return overflow_add(i0, i1) ? (value_type)i0 + (value_type)i1 : i0 + i1;
        },
        [](auto x, auto y) -> Number {
            using T = std::common_type_t<decltype(x), decltype(y)>;
            return (T)x + (T)y;
        }
    };
    return visit(std::move(fun),
        static_cast<const Number::base_type&>(lhs),
        static_cast<const Number::base_type&>(rhs));
}

/**
 * @brief Number addition operator.
 *
 * In case of an integer overflow, the returned number is converted into a floating point sum.
 */
Number operator-(const Number& lhs, const Number& rhs)
{
    using value_type = Complex::value_type;

    auto fun = overloads{
        [](const Complex& z0, const Complex& z1) -> Number { return z0 - z1; },
        [](const Complex& z, auto x) -> Number { return z - (value_type)x; },
        [](auto x, const Complex& z) -> Number { return (value_type)x - z; },
        [](Int i0, Int i1) -> Number { return overflow_sub(i0, i1) ? (value_type)i0 - (value_type)i1 : i0 - i1; },
        [](auto x, auto y) -> Number {
            using T = std::common_type_t<decltype(x), decltype(y)>;
            return (T)x - (T)y;
        }
    };
    return visit(std::move(fun),
        static_cast<const Number::base_type&>(lhs),
        static_cast<const Number::base_type&>(rhs));
}

/**
 * @brief Number multiplication operator.
 */
Number operator*(const Number& lhs, const Number& rhs)
{
    using value_type = Complex::value_type;

    auto fun = overloads{
        [](const Complex& z0, const Complex& z1) -> Number { return z0 * z1; },
        [](const Complex& z, auto x) -> Number { return z * (value_type)x; },
        [](auto x, const Complex& z) -> Number { return (value_type)x * z; },
        [](auto x, auto y) -> Number {
            using T = std::common_type_t<decltype(x), decltype(y)>;
            return (T)x * (T)y;
        }
    };
    return visit(std::move(fun),
        static_cast<const Number::base_type&>(lhs),
        static_cast<const Number::base_type&>(rhs));
}

/**
 * @brief Number division operator.
 */
Number operator/(const Number& lhs, const Number& rhs)
{
    using value_type = Complex::value_type;

    rhs != Number{} || (throw std::invalid_argument("divide by zero"), 0);

    auto fun = overloads{
        [](const Complex& z0, const Complex& z1) -> Number { return z0 / z1; },
        [](const Complex& z, auto x) -> Number { return z / (value_type)x; },
        [](auto x, const Complex& z) -> Number { return (value_type)x / z; },
        [](Int x, Int y) -> Number { return x / (value_type)y; },
        [](auto x, auto y) -> Number {
            using T = std::common_type_t<decltype(x), decltype(y)>;
            return (T)x / (T)y;
        }
    };
    return visit(std::move(fun),
        static_cast<const Number::base_type&>(lhs),
        static_cast<const Number::base_type&>(rhs));
}

/**
 * @brief Addition assignment operator.
 */
Number& operator+=(Number& lhs, const Number& rhs)
{
    return lhs = lhs + rhs;
}

/**
 * @brief Addition assignment operator.
 */
Number& operator-=(Number& lhs, const Number& rhs)
{
    return lhs = lhs - rhs;
}

/**
 * @brief Addition assignment operator.
 */
Number& operator*=(Number& lhs, const Number& rhs)
{
    return lhs = lhs * rhs;
}

/**
 * @brief Addition assignment operator.
 */
Number& operator/=(Number& lhs, const Number& rhs)
{
    return lhs = lhs / rhs;
}

/**
 * @brief Computes the sine of a ::Number.
 */
Number sin(const Number& x)
{
    return is_complex(x) ? std::sin(static_cast<const Complex&>(x))
                         : std::sin(static_cast<Float>(x));
}

/**
 * @brief Computes the cosine of a ::Number.
 */
Number cos(const Number& x)
{
    return is_complex(x) ? std::cos(static_cast<const Complex&>(x))
                         : std::cos(static_cast<Float>(x));
}

/**
 * @brief Computes the cosine of a ::Number.
 */
Number tan(const Number& x)
{
    return is_complex(x) ? std::tan(static_cast<const Complex&>(x))
                         : std::tan(static_cast<Float>(x));
}

/**
 * @brief Computes the arcus sinus of a ::Number.
 */
Number asin(const Number& x)
{
    return is_type<Complex>(x) ? std::asin(static_cast<const Complex&>(x))
                               : std::asin(static_cast<Float>(x));
}

/**
 * @brief Computes the arcus sinus of a ::Number.
 */
Number acos(const Number& x)
{
    return is_complex(x) ? std::acos(static_cast<const Complex&>(x))
                         : std::acos(static_cast<Float>(x));
}

/**
 * @brief Computes the arcus sinus of a ::Number.
 */
Number atan(const Number& x)
{
    return is_complex(x) ? std::atan(static_cast<const Complex&>(x))
                         : std::atan(static_cast<Float>(x));
}

/**
 * @brief Computes the arcus sinus of a ::Number.
 */
Number sinh(const Number& x)
{
    return is_complex(x) ? std::sinh(static_cast<const Complex&>(x))
                         : std::sinh(static_cast<Float>(x));
}

/**
 * @brief Computes the arcus sinus of a ::Number.
 */
Number cosh(const Number& x)
{
    return is_complex(x) ? std::cosh(static_cast<const Complex&>(x))
                         : std::cosh(static_cast<Float>(x));
}

/**
 * @brief Computes the arcus sinus of a ::Number.
 */
Number tanh(const Number& x)
{
    return is_complex(x) ? std::tanh(static_cast<const Complex&>(x))
                         : std::tanh(static_cast<Float>(x));
}

/**
 * @brief Computes the arcus sinus of a ::Number.
 */
Number asinh(const Number& x)
{
    return is_complex(x) ? std::asinh(static_cast<const Complex&>(x))
                         : std::asinh(static_cast<Float>(x));
}

/**
 * @brief Computes the arcus sinus of a ::Number.
 */
Number acosh(const Number& x)
{
    return is_complex(x) ? std::acosh(static_cast<const Complex&>(x))
                         : std::acosh(static_cast<Float>(x));
}

/**
 * @brief Computes the arcus sinus of a ::Number.
 */
Number atanh(const Number& x)
{
    return is_complex(x) ? std::atanh(static_cast<const Complex&>(x))
                         : std::atanh(static_cast<Float>(x));
}

/**
 * @brief Compute e exponential of a ::Number.
 */
Number exp(const Number& x)
{
    return is_complex(x) ? std::exp(static_cast<const Complex&>(x))
                         : std::exp(static_cast<Float>(x));
}

/**
 * @brief Compute natural logarithm of a ::Number.
 */
Number log(const Number& x)
{
    return is_complex(x) ? std::log(static_cast<const Complex&>(x))
                         : std::log(static_cast<Float>(x));
}

/**
 * @brief Compute natural logarithm of a ::Number.
 */
Number log10(const Number& x)
{
    return is_complex(x) ? std::log10(static_cast<const Complex&>(x))
                         : std::log10(static_cast<Float>(x));
}

/**
 * @brief Computes the square root of ::Number.
 */
Number sqrt(const Number& x)
{
    return is_complex(x) || x < Number{ 0 } ? std::sqrt(static_cast<const Complex&>(x))
                                            : std::sqrt(static_cast<Float>(x));
}

/**
 * @brief Computes x raised by y.
 */
Number pow(const Number& x, const Number& y)
{
    return is_complex(x) || is_complex(y) ? std::pow(static_cast<const Complex&>(x), static_cast<const Complex&>(y))
                                          : std::pow(static_cast<Float>(x), static_cast<Float>(y));
}

/**
 * @brief Return the real part of a complex number.
 */
Number real(const Number& z)
{
    return static_cast<const Complex&>(z).real();
}

/**
 * @brief Return the imaginary part of a complex number or zero for floating point or
 *        integer arguments.
 */
Number imag(const Number& z)
{
    return static_cast<const Complex&>(z).imag();
}

Number rect(const Number& x, const Number& y)
{
    return { static_cast<Float>(x), static_cast<Float>(y) };
}

/**
 * @brief Construct a complex number from magnitude and phase angle.
 */
Number polar(const Number& r, const Number& theta)
{
    return std::polar(static_cast<Float>(r), static_cast<Float>(theta));
}

Number arg(const Number& z)
{
    return std::arg(static_cast<const Complex&>(z));
}

Number conj(const Number& z)
{
    return std::conj(static_cast<const Complex&>(z));
}

Number abs(const Number& x)
{
    return visit([](const auto& x) -> Number { return std::abs(x); },
        static_cast<const Number::base_type&>(x));
}

} // namespace pscm
