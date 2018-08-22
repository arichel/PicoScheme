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

bool is_integer(const Number& num)
{
    auto fun = overloads{
        [](Int i) -> bool { return true; },
        [](Float x) -> bool {
            return !(x > floor(x)
                || x < ceil(x)
                || std::abs(x) > static_cast<Float>(std::numeric_limits<Int>::max()));
        },
        [](const Complex& z) -> bool {
            return imag(z) < 0 || imag(z) > 0 ? false : is_integer(real(z));
        },
    };
    return visit(std::move(fun), static_cast<const Number::base_type&>(num));
}

bool is_odd(const Number& num)
{
    auto fun = overloads{
        [](Int i) -> bool { return std::abs(i) % Int{ 2 }; },
        [](Float x) -> bool { return fmod(x, 2.); },
        [](const Complex& z) -> bool { return imag(z) < 0 || imag(z) > 0 ? true : fmod(real(z), 2.); },
    };
    return visit(std::move(fun), static_cast<const Number::base_type&>(num));
}

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

    auto fun = overloads{
        [](const Complex& z0, const Complex& z1) -> bool {
            return z0 != z1;
        },
        [](const Complex& z, Float x) -> bool {
            return z != Complex{ static_cast<value_type>(x), 0 };
        },
        [](Float x, const Complex& z) -> bool {
            return z != Complex{ static_cast<value_type>(x), 0 };
        },
        [](Float x, Float y) -> bool {
            return x != y;
        },
        [](Int x, Int y) -> bool {
            return x != y;
        },
        [](auto x, auto y) -> bool {
            return true;
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

bool operator<(const Number& lhs, const Number& rhs)
{
    return visit([](auto x, auto y) {
        if constexpr (!std::is_same_v<Complex, decltype(x)> && !std::is_same_v<Complex, decltype(y)>) {
            using T = std::common_type_t<decltype(x), decltype(y)>;
            return static_cast<T>(x) < static_cast<T>(y);
        } else
            return ((void)(throw std::invalid_argument("uncomparable complex number")), false);
    },
        static_cast<const Number::base_type&>(lhs), static_cast<const Number::base_type&>(rhs));
}

bool operator>(const Number& lhs, const Number& rhs)
{
    return visit([](auto x, auto y) {
        if constexpr (!std::is_same_v<Complex, decltype(x)> && !std::is_same_v<Complex, decltype(y)>) {
            using T = std::common_type_t<decltype(x), decltype(y)>;
            return static_cast<T>(x) > static_cast<T>(y);
        } else
            return ((void)(throw std::invalid_argument("uncomparable complex number")), false);
    },
        static_cast<const Number::base_type&>(lhs), static_cast<const Number::base_type&>(rhs));
}

bool operator<=(const Number& lhs, const Number& rhs)
{
    return visit([](auto x, auto y) {
        if constexpr (!std::is_same_v<Complex, decltype(x)> && !std::is_same_v<Complex, decltype(y)>) {
            using T = std::common_type_t<decltype(x), decltype(y)>;
            return static_cast<T>(x) <= static_cast<T>(y);
        } else
            return ((void)(throw std::invalid_argument("uncomparable complex number")), false);
    },
        static_cast<const Number::base_type&>(lhs), static_cast<const Number::base_type&>(rhs));
}

bool operator>=(const Number& lhs, const Number& rhs)
{
    return visit([](auto x, auto y) {
        if constexpr (!std::is_same_v<Complex, decltype(x)> && !std::is_same_v<Complex, decltype(y)>) {
            using T = std::common_type_t<decltype(x), decltype(y)>;
            return static_cast<T>(x) >= static_cast<T>(y);
        } else
            return ((void)(throw std::invalid_argument("uncomparable complex number")), false);
    },
        static_cast<const Number::base_type&>(lhs), static_cast<const Number::base_type&>(rhs));
}

Number min(const Number& lhs, const Number& rhs)
{
    return visit([](auto& x, auto& y) -> Number {
        using T = std::common_type_t<decltype(x), decltype(y)>;
        return y < x ? static_cast<T>(y) : static_cast<T>(x);
    },
        static_cast<const Number::base_type&>(lhs), static_cast<const Number::base_type&>(rhs));
}

Number max(const Number& lhs, const Number& rhs)
{
    return visit([](auto& x, auto& y) -> Number {
        using T = std::common_type_t<decltype(x), decltype(y)>;
        return y > x ? static_cast<T>(y) : static_cast<T>(x);
    },
        static_cast<const Number::base_type&>(lhs), static_cast<const Number::base_type&>(rhs));
}

Number inv(const Number& x)
{
    x != Number{ 0 } || ((void)(throw std::invalid_argument("divide by zero")), 0);

    return visit([](auto& x) -> Number {
        if constexpr (std::is_same_v<const Complex&, decltype(x)>)
            return 1 / x;
        else
            return 1 / static_cast<Float>(x);
    },
        static_cast<const Number::base_type&>(x));
}

Number operator-(const Number& x)
{
    return visit([](auto& x) -> Number { return -x; },
        static_cast<const Number::base_type&>(x));
}

Number operator%(const Number& lhs, const Number& rhs)
{
    auto fun = overloads{
        [](const Complex& z1, const Complex& z2) -> Number { return ((void)(throw std::invalid_argument("modulo - not definied for complex numbers")), 0); },
        [](const Complex& z, auto x) -> Number { return ((void)(throw std::invalid_argument("modulo - not definied for complex numbers")), 0); },
        [](auto x, const Complex& z) -> Number { return ((void)(throw std::invalid_argument("modulo - not definied for complex numbers")), 0); },
        [](Int i0, Int i1) -> Number { return (i1 + i0 % i1) % i1; },
        [](auto x, auto y) -> Number { return fmod((y + fmod(x, y)), y); }
    };
    return visit(std::move(fun),
        static_cast<const Number::base_type&>(lhs),
        static_cast<const Number::base_type&>(rhs));
}

Number remainder(const Number& lhs, const Number& rhs)
{
    constexpr auto fun = overloads{
        [](const Complex& z1, const Complex& z2) -> Number { return ((void)(throw std::invalid_argument("modulo - not definied for complex numbers")), 0); },
        [](const Complex& z, auto x) -> Number { return ((void)(throw std::invalid_argument("remainder - not definied for complex numbers")), 0); },
        [](auto x, const Complex& z) -> Number { return ((void)(throw std::invalid_argument("remainder - not definied for complex numbers")), 0); },
        [](Int i0, Int i1) -> Number { return static_cast<Int>(std::remainder(i0, i1)); },
        [](auto x, auto y) -> Number { return std::remainder(x, y); }
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
Number operator+(const Number& lhs, const Number& rhs)
{
    using value_type = Complex::value_type;

    auto fun = overloads{
        [](const Complex& z0, const Complex& z1) -> Number { return z0 + z1; },
        [](const Complex& z, auto x) -> Number { return z + static_cast<value_type>(x); },
        [](auto x, const Complex& z) -> Number { return static_cast<value_type>(x) + z; },
        [](Int i0, Int i1) -> Number {
            if (overflow_add(i0, i1))
                return static_cast<value_type>(i0) + static_cast<value_type>(i1);
            else
                return i0 + i1;
        },
        [](auto x, auto y) -> Number {
            using T = std::common_type_t<decltype(x), decltype(y)>;
            return static_cast<T>(x) + static_cast<T>(y);
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
        [](const Complex& z, auto x) -> Number { return z - static_cast<value_type>(x); },
        [](auto x, const Complex& z) -> Number { return static_cast<value_type>(x) - z; },
        [](Int i0, Int i1) -> Number {
            if (overflow_sub(i0, i1))
                return static_cast<value_type>(i0) - static_cast<value_type>(i1);
            else
                return i0 - i1;
        },
        [](auto x, auto y) -> Number {
            using T = std::common_type_t<decltype(x), decltype(y)>;
            return static_cast<T>(x) - static_cast<T>(y);
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
        [](const Complex& z, auto x) -> Number { return z * static_cast<value_type>(x); },
        [](auto x, const Complex& z) -> Number { return static_cast<value_type>(x) * z; },
        [](auto x, auto y) -> Number {
            using T = std::common_type_t<decltype(x), decltype(y)>;
            return static_cast<T>(x) * static_cast<T>(y);
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

    rhs != Number{ 0 } || ((void)(throw std::invalid_argument("divide by zero")), 0);

    auto fun = overloads{
        [](const Complex& z0, const Complex& z1) -> Number { return z0 / z1; },
        [](const Complex& z, auto x) -> Number { return z / static_cast<value_type>(x); },
        [](auto x, const Complex& z) -> Number { return static_cast<value_type>(x) / z; },
        [](Int x, Int y) -> Number {
            if (std::remainder(x, y) != Int{ 0 })
                return x / static_cast<value_type>(y);
            else
                return x / y;
        },
        [](auto x, auto y) -> Number {
            using T = std::common_type_t<decltype(x), decltype(y)>;
            return static_cast<T>(x) / static_cast<T>(y);
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

template <typename T>
static T round_even(T x)
{
    return x - std::remainder(x, T{ 1 });
}

/**
 * @brief Round a number to the nearest integer.
 */
Number round(const Number& x)
{
    auto fun = overloads{
        [](Int i) -> Number {
            return i;
        },
        [](Float x) -> Number {
            return round_even(x);
        },
        [](const Complex& z) -> Number {
            return { round_even(z.real()), round_even(z.imag()) };
        }
    };
    return visit(std::move(fun), static_cast<const Number::base_type&>(x));
}

/**
 * @brief Largest integer not greator then x.
 */
Number floor(const Number& x)
{
    if (is_complex(x)) {
        const auto& z = static_cast<Complex>(x);
        return { std::floor(z.real()), std::floor(z.imag()) };
    } else
        return std::floor(static_cast<Float>(x));
}

/**
 * @brief Largest integer not greator then x.
 */
Number ceil(const Number& x)
{
    if (is_complex(x)) {
        const auto& z = static_cast<Complex>(x);
        return { std::ceil(z.real()), std::ceil(z.imag()) };
    } else
        return std::ceil(static_cast<Float>(x));
}

Number quotient(const Number& lhs, const Number& rhs)
{
    Number res = lhs / rhs;

    if (is_int(res))
        return res;

    return is_positive(res) ? floor(res) : ceil(res);
}

/**
 * @brief Largest integer not greator then x.
 */
Number trunc(const Number& x)
{
    return is_negative(x) ? ceil(x) : floor(x);
}

/**
 * @brief Computes the sine of a ::Number.
 */
Number sin(const Number& x)
{
    return is_complex(x) ? std::sin(static_cast<Complex>(x))
                         : std::sin(static_cast<Float>(x));
}

/**
 * @brief Computes the cosine of a ::Number.
 */
Number cos(const Number& x)
{
    return is_complex(x) ? std::cos(static_cast<Complex>(x))
                         : std::cos(static_cast<Float>(x));
}

/**
 * @brief Computes the cosine of a ::Number.
 */
Number tan(const Number& x)
{
    return is_complex(x) ? std::tan(static_cast<Complex>(x))
                         : std::tan(static_cast<Float>(x));
}

/**
 * @brief Computes the arcus sinus of a ::Number.
 */
Number asin(const Number& x)
{
    return is_type<Complex>(x) ? std::asin(static_cast<Complex>(x))
                               : std::asin(static_cast<Float>(x));
}

/**
 * @brief Computes the arcus sinus of a ::Number.
 */
Number acos(const Number& x)
{
    return is_complex(x) ? std::acos(static_cast<Complex>(x))
                         : std::acos(static_cast<Float>(x));
}

/**
 * @brief Computes the arcus sinus of a ::Number.
 */
Number atan(const Number& x)
{
    return is_complex(x) ? std::atan(static_cast<Complex>(x))
                         : std::atan(static_cast<Float>(x));
}

/**
 * @brief Computes the arcus sinus of a ::Number.
 */
Number sinh(const Number& x)
{
    return is_complex(x) ? std::sinh(static_cast<Complex>(x))
                         : std::sinh(static_cast<Float>(x));
}

/**
 * @brief Computes the arcus sinus of a ::Number.
 */
Number cosh(const Number& x)
{
    return is_complex(x) ? std::cosh(static_cast<Complex>(x))
                         : std::cosh(static_cast<Float>(x));
}

/**
 * @brief Computes the arcus sinus of a ::Number.
 */
Number tanh(const Number& x)
{
    return is_complex(x) ? std::tanh(static_cast<Complex>(x))
                         : std::tanh(static_cast<Float>(x));
}

/**
 * @brief Computes the arcus sinus of a ::Number.
 */
Number asinh(const Number& x)
{
    return is_complex(x) ? std::asinh(static_cast<Complex>(x))
                         : std::asinh(static_cast<Float>(x));
}

/**
 * @brief Computes the arcus sinus of a ::Number.
 */
Number acosh(const Number& x)
{
    return is_complex(x) ? std::acosh(static_cast<Complex>(x))
                         : std::acosh(static_cast<Float>(x));
}

/**
 * @brief Computes the arcus sinus of a ::Number.
 */
Number atanh(const Number& x)
{
    return is_complex(x) ? std::atanh(static_cast<Complex>(x))
                         : std::atanh(static_cast<Float>(x));
}

/**
 * @brief Compute e exponential of a ::Number.
 */
Number exp(const Number& x)
{
    return is_complex(x) ? std::exp(static_cast<Complex>(x))
                         : std::exp(static_cast<Float>(x));
}

/**
 * @brief Compute natural logarithm of a ::Number.
 */
Number log(const Number& x)
{
    return is_complex(x) ? std::log(static_cast<Complex>(x))
                         : std::log(static_cast<Float>(x));
}

/**
 * @brief Compute natural logarithm of a ::Number.
 */
Number log10(const Number& x)
{
    return is_complex(x) ? std::log10(static_cast<Complex>(x))
                         : std::log10(static_cast<Float>(x));
}

/**
 * @brief Computes the square root of ::Number.
 */
Number sqrt(const Number& x)
{
    return is_complex(x) || x < Number{ 0 } ? std::sqrt(static_cast<Complex>(x))
                                            : std::sqrt(static_cast<Float>(x));
}

/**
 * @brief Computes the cubic root of ::Number.
 */
Number cbrt(const Number& x)
{
    return is_complex(x) || x < Number{ 0 } ? std::pow(static_cast<Complex>(x), static_cast<Complex>(-3))
                                            : std::cbrt(static_cast<Float>(x));
}

/**
 * @brief Computes x raised by y.
 */
Number hypot(const Number& x, const Number& y)
{
    return is_complex(x) || is_complex(y) ? sqrt(static_cast<Complex>(x)
                                                    * static_cast<Complex>(x)
                                                + static_cast<Complex>(y)
                                                    * static_cast<Complex>(y))
                                          : std::hypot(static_cast<Float>(x),
                                                static_cast<Float>(y));
}

Number hypot(const Number& x, const Number& y, const Number& z)
{
    return is_complex(x) || is_complex(y) ? sqrt(static_cast<Complex>(x)
                                                    * static_cast<Complex>(x)
                                                + static_cast<Complex>(y)
                                                    * static_cast<Complex>(y)
                                                + static_cast<Complex>(z)
                                                    * static_cast<Complex>(z))

                                          : std::hypot(static_cast<Float>(x),
                                                static_cast<Float>(y),
                                                static_cast<Float>(z));
}

/**
 * @brief Computes x raised by y.
 */
Number pow(const Number& x, const Number& y)
{
    if (is_zero(x))
        return is_zero(y) ? Int{ 1 } : Int{ 0 };

    auto fun = overloads{
        [](const Complex& x, const Complex& y) -> Number {
            return std::pow(x, y);
        },
        [](const Complex& z, auto& x) -> Number {
            return std::pow(z, static_cast<Complex>(x));
        },
        [](auto& x, const Complex& z) -> Number {
            return std::pow(static_cast<Complex>(x), z);
        },
        [](Int x, Int y) -> Number {
            constexpr Int min = std::numeric_limits<Int>::min(),
                          max = std::numeric_limits<Int>::max();

            auto res = std::pow(static_cast<Float>(x), static_cast<Float>(y));
            if (res < min || res > max)
                return res;
            return static_cast<Int>(res);
        },
        [](auto x, auto y) -> Number {
            return std::pow(static_cast<Float>(x), static_cast<Float>(y));
        }
    };
    return visit(std::move(fun), static_cast<const Number::base_type&>(x),
        static_cast<const Number::base_type&>(y));
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
