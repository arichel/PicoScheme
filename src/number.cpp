/*********************************************************************************/ /**
 * @file number.cpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#include "number.hpp"
#include "utils.hpp"
#include <assert.h>
#include <cmath>
#include <limits>

namespace pscm {

/**
 * @brief Check wheter an integer addition of both argument values would overflow.
 */
static bool overflow_add(Int a, Int b)
{
    static constexpr Int min = std::numeric_limits<Int>::min(),
                         max = std::numeric_limits<Int>::max();

    return (b > 0 && a > max - b) || (b < 0 && a < min - b);
}

/**
 * @brief Check wheater integer substraction of both arguments values would overflow.
 */
static bool overflow_sub(Int a, Int b)
{
    static constexpr Int min = std::numeric_limits<Int>::min(),
                         max = std::numeric_limits<Int>::max();

    return (b > 0 && a < min + b) || (b < 0 && a > max + b);
}

/**
 * @brief Predicate function to test wheter the argument numbers aren't equal.
 */
bool operator!=(const Number& lhs, const Number& rhs)
{
    using value_type = Complex::value_type;

    auto fun = overloads{
        [](const Complex& z0, const Complex& z1) -> bool { return z0 != z1; },
        [](const Complex& z, auto x) -> bool { return z != (value_type)x; },
        [](auto x, const Complex& z) -> bool { return (value_type)x != z; },
        [](auto x, auto y) -> bool {
            using T = std::common_type_t<decltype(x), decltype(y)>;
            return (T)x != (T)y;
        }
    };
    return visit(std::move(fun),
        static_cast<const Number::base_type&>(lhs),
        static_cast<const Number::base_type&>(rhs));
}

/**
 * @brief Predicate function to test wheter the argument numbers are equal.
 */
bool operator==(const Number& lhs, const Number& rhs)
{
    return !(lhs != rhs);
}

/**
 * @brief Number addition operator.
 *
 * In case of an integer overflow, the returned number is converted into a floating point sum.
 */
Number operator+(const Number& lhs, const Number& rhs)
{
    using value_type = Complex::value_type;

    auto fun = overloads{
        [](const Complex& z0, const Complex& z1) -> Number { return z0 + z1; },
        [](const Complex& z, auto x) -> Number { return z + (value_type)x; },
        [](auto x, const Complex& z) -> Number { return (value_type)x + z; },
        [](Int i0, Int i1) -> Number {
            return overflow_add(i0, i1) ? (value_type)i0 - (value_type)i1 : i0 + i1;
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
        [](Int i0, Int i1) -> Number {
            return overflow_sub(i0, i1) ? (value_type)i0 - (value_type)i1 : i0 - i1;
        },
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

    assert(rhs != Number{});

    auto fun = overloads{
        [](const Complex& z0, const Complex& z1) -> Number { return z0 / z1; },
        [](const Complex& z, auto x) -> Number { return z / (value_type)x; },
        [](auto x, const Complex& z) -> Number { return (value_type)x / z; },
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
    return is_type<Complex>(x) ? std::sin(static_cast<Complex>(x)) : std::sin(static_cast<Float>(x));
}

/**
 * @brief Computes the cosine of a ::Number.
 */
Number cos(const Number& x)
{
    return is_type<Complex>(x) ? std::cos(static_cast<Complex>(x)) : std::cos(static_cast<Float>(x));
}

/**
 * @brief Computes the cosine of a ::Number.
 */
Number tan(const Number& x)
{
    return is_type<Complex>(x) ? std::tan(static_cast<Complex>(x)) : std::tan(static_cast<Float>(x));
}

/**
 * @brief Computes the arcus sinus of a ::Number.
 */
Number asin(const Number& x)
{
    return is_type<Complex>(x) ? std::asin(static_cast<Complex>(x)) : std::asin(static_cast<Float>(x));
}

/**
 * @brief Computes the arcus sinus of a ::Number.
 */
Number acos(const Number& x)
{
    return is_type<Complex>(x) ? std::acos(static_cast<Complex>(x)) : std::acos(static_cast<Float>(x));
}

/**
 * @brief Computes the arcus sinus of a ::Number.
 */
Number atan(const Number& x)
{
    return is_type<Complex>(x) ? std::atan(static_cast<Complex>(x)) : std::atan(static_cast<Float>(x));
}

/**
 * @brief Computes the arcus sinus of a ::Number.
 */
Number sinh(const Number& x)
{
    return is_type<Complex>(x) ? std::sinh(static_cast<Complex>(x)) : std::sinh(static_cast<Float>(x));
}

/**
 * @brief Computes the arcus sinus of a ::Number.
 */
Number cosh(const Number& x)
{
    return is_type<Complex>(x) ? std::cosh(static_cast<Complex>(x)) : std::cosh(static_cast<Float>(x));
}

/**
 * @brief Computes the arcus sinus of a ::Number.
 */
Number tanh(const Number& x)
{
    return is_type<Complex>(x) ? std::tanh(static_cast<Complex>(x)) : std::tanh(static_cast<Float>(x));
}

/**
 * @brief Computes the arcus sinus of a ::Number.
 */
Number asinh(const Number& x)
{
    return is_type<Complex>(x) ? std::asinh(static_cast<Complex>(x)) : std::asinh(static_cast<Float>(x));
}

/**
 * @brief Computes the arcus sinus of a ::Number.
 */
Number acosh(const Number& x)
{
    return is_type<Complex>(x) ? std::acosh(static_cast<Complex>(x)) : std::acosh(static_cast<Float>(x));
}

/**
 * @brief Computes the arcus sinus of a ::Number.
 */
Number atanh(const Number& x)
{
    return is_type<Complex>(x) ? std::atanh(static_cast<Complex>(x)) : std::atanh(static_cast<Float>(x));
}

/**
 * @brief Compute e exponential of a ::Number.
 */
Number exp(const Number& x)
{
    return is_type<Complex>(x) ? std::exp(static_cast<Complex>(x)) : std::exp(static_cast<Float>(x));
}

/**
 * @brief Compute natural logarithm of a ::Number.
 */
Number log(const Number& x)
{
    return is_type<Complex>(x) ? std::log(static_cast<Complex>(x)) : std::log(static_cast<Float>(x));
}

/**
 * @brief Compute natural logarithm of a ::Number.
 */
Number log10(const Number& x)
{
    return is_type<Complex>(x) ? std::log10(static_cast<Complex>(x)) : std::log10(static_cast<Float>(x));
}

/**
 * @brief Computes the square root of ::Number.
 */
Number sqrt(const Number& x, const Number& y)
{
    return is_type<Complex>(x) ? std::sqrt(static_cast<Complex>(x)) : std::sqrt(static_cast<Float>(x));
}

/**
 * @brief Computes x raised by y.
 */
Number pow(const Number& x, const Number& y)
{
    return is_type<Complex>(x) || is_type<Complex>(y) ? std::pow(static_cast<Complex>(x), static_cast<Complex>(y))
                                                      : std::pow(static_cast<Float>(x), static_cast<Float>(y));
}

/**
 * @brief Return the real part of a complex number.
 */
Number real(const Number& z)
{
    return static_cast<Complex>(z).real();
}

/**
 * @brief Return the imaginary part of a complex number or zero for floating point or
 *        integer arguments.
 */
Number imag(const Number& z)
{
    return static_cast<Complex>(z).imag();
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
    return std::arg(static_cast<Complex>(z));
}

Number conj(const Number& z)
{
    return std::conj(static_cast<Complex>(z));
}

Number abs(const Number& x)
{
    return visit([](const auto& x) -> Number { return std::abs(x); },
        static_cast<const Number::base_type&>(x));
}

} // namespace pscm
