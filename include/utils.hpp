#ifndef UTILS_HPP
#define UTILS_HPP

#include <utility>
#include <variant>

/**
 * @brief Build a vistor type for std::visit, where the function application
 *        operator () is overloaded for each template argument type.
 */
template <typename... Ts>
struct overloads : Ts... {
    using Ts::operator()...;
};
template <typename... Ts>
overloads(Ts...)->overloads<Ts...>; //!< construct overloads type

template <class T>
struct always_false : std::false_type {
}; //!< return false for every type

template <typename T, typename Variant>
constexpr bool is_type(Variant&& cell)
{
    return std::holds_alternative<T>(std::forward<Variant>(cell));
}

#endif // UTILS_HPP
