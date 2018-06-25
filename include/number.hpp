#ifndef NUMBER_HPP
#define NUMBER_HPP

#include "utils.hpp"

#include <cmath>
#include <complex>
#include <utility>
#include <variant>

using Int = int64_t;
using Float = double;
using Complex = std::complex<double>;

struct Number : std::variant<Int, Float, Complex> {
    using base_type = std::variant<Int, Float, Complex>;
    using base_type::operator=;
    using base_type::base_type;

    //operator base_type() const { return static_cast<base_type>(*this); }

    template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
    constexpr operator T() const
    {
        return std::visit([](const auto& num) -> T {
            using TT = std::decay_t<decltype(num)>;

            if constexpr (std::is_same_v<TT, Int>) {
                if constexpr (std::is_integral_v<T>)
                    return T(num);
                else
                    return T(Float(num));

            } else if constexpr (std::is_same_v<TT, Float>)
                return T(num);

            else if constexpr (std::is_same_v<TT, Complex>) {
                if constexpr (std::is_same_v<T, Complex>)
                    return num;
                else
                    return T(std::abs(num));
            } else
                static_assert(always_false<TT>::value, "invalid variant");
        },
            static_cast<const base_type&>(*this));
    }

    //    constexpr Number()
    //        : base_type{ Int{ 0 } }
    //    {
    //    }

    //    constexpr Number(Int i)
    //        : base_type{ i }
    //    {
    //    }
    //    constexpr explicit Number(const Complex& z)
    //        : Number{ z.real(), z.imag() }
    //    {
    //    }
    //    explicit Number(Float x)
    //    {
    //        if (x != std::trunc(x))
    //            *this = x;
    //        else
    //            *this = static_cast<Int>(x);
    //    }
    //    template <typename RE, typename IM>
    //    constexpr Number(const RE& x, const IM& y)
    //    {
    //        using T = Complex::value_type;

    //        if (y != IM{ 0 })
    //            *this =   { (T)x, (T)y };
    //        else
    //            *this = Number{ x };
    //    }
};

namespace std {

constexpr Number operator+(const Number& lhs, const Number& rhs)
{
    return visit([](const auto& lhs, const auto& rhs) -> Number { return lhs + rhs; },
        static_cast<const Number::base_type&>(lhs),
        static_cast<const Number::base_type&>(rhs));
}

constexpr Number& operator+=(Number& lhs, const Number& rhs)
{
    return lhs = lhs + rhs;
}

constexpr Number operator-(const Number& lhs, const Number& rhs)
{
    return visit([](const auto& lhs, const auto& rhs) -> Number { return lhs - rhs; },
        static_cast<const Number::base_type&>(lhs),
        static_cast<const Number::base_type&>(rhs));
}

constexpr Number operator*(const Number& lhs, const Number& rhs)
{
    return visit([](const auto& lhs, const auto& rhs) -> Number { return lhs * rhs; },
        static_cast<const Number::base_type&>(lhs),
        static_cast<const Number::base_type&>(rhs));
}

constexpr Number operator/(const Number& lhs, const Number& rhs)
{
    return visit([](const auto& lhs, const auto& rhs) -> Number { return lhs / rhs; },
        static_cast<const Number::base_type&>(lhs),
        static_cast<const Number::base_type&>(rhs));
}

Number sin(const Number& num)
{
    return is_type<Complex>(num) ? sin(static_cast<Complex>(num)) : sin(static_cast<Float>(num));
}

Number cos(const Number& num)
{
    return is_type<Complex>(num) ? cos(static_cast<Complex>(num)) : cos(static_cast<Float>(num));
}

Number real(const Number& num)
{
    return is_type<Complex>(num) ? static_cast<Complex>(num).real() : num;
}

Number imag(const Number& num)
{
    return is_type<Complex>(num) ? static_cast<Complex>(num).imag() : Number{};
}

Number polar(const Number& r, const Number& theta = {})
{
    return std::polar(static_cast<Float>(r), static_cast<Float>(theta));
}

}; // namspace std

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const Number& num)
{
    return (std::visit([&os](auto arg) { os << arg; }, static_cast<const Number::base_type&>(num)), os);
}

//template <typename CharT, typename Traits>
//std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const Number& num)
//{
//    overloads fun{
//        [&os](Atom num) { os << num; },
//        [&os](Complex num) { os << num.real() << '+' << num.imag() << 'i'; },

//        /* catch missing overloads and emit compile time error message */
//        [](auto arg) { static_assert(always_false<decltype(arg)>::value,
//                           "callable overload is missing"); }
//    };
//    std::visit(std::move(fun), num);
//    return os;
//}

#endif // NUMBER_HPP
