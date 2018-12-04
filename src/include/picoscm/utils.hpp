/********************************************************************************/ /**
 * @file utils.hpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#ifndef UTILS_HPP
#define UTILS_HPP

#include <codecvt>
#include <locale>
#include <type_traits>
#include <utility>
#include <variant>

namespace pscm {

template <typename T>
struct TDEF;

//! Build a vistor type for std::visit, where the function application
//! operator () is overloaded for each template argument type.
template <typename... Ts>
struct overloads : Ts... {
    using Ts::operator()...;
};
template <typename... Ts>
overloads(Ts...)->overloads<Ts...>; //!< construct overloads type

template <class T>
struct always_false : std::false_type {
}; //!< return false for every type

//! Shortcut for std::holds_alternative
template <typename T, typename Variant>
constexpr bool is_type(Variant&& v)
{
    return std::holds_alternative<T>(std::forward<Variant>(v));
}

//! Trait class to retrieve the character type of a string or character buffer.
template <typename T, bool is_class = std::is_class_v<T>>
struct char_traits;

template <typename T> //! partial specialisation for string like classes
struct char_traits<T, true> {
    using char_type = typename std::char_traits<std::remove_const_t<typename T::value_type>>::char_type;
};

template <typename T> //! partial specialisation for null-terminated char[] buffers
struct char_traits<T, false> {
    using char_type = typename std::char_traits<std::remove_const_t<std::remove_pointer_t<std::decay_t<T>>>>::char_type;
};

//! Convert byte encoded utf-8 string of type StringT into a
//! wide character std::basic_string<CharT>.
template <typename CharT, typename StringT,
    typename = std::enable_if_t<std::is_integral_v<CharT>>>
std::basic_string<CharT> s2ws(const StringT& str)
{
    using value_type = typename char_traits<StringT>::char_type;
    static_assert(std::is_same_v<char, value_type>, "not a single byte character type");

    using convert_type = std::codecvt_utf8<CharT>;
    static std::wstring_convert<convert_type, CharT> converter;
    return converter.from_bytes(str);
}

//! Convert a string of type StringT into a byte encoded
//! utf-8 std::basic_string<char>.
template <typename StringT>
std::basic_string<char> ws2s(const StringT& str)
{
    using CharT = typename char_traits<StringT>::char_type;
    using convert_type = std::codecvt_utf8<CharT>;
    static std::wstring_convert<convert_type, CharT> converter;
    return converter.to_bytes(str);
}

//! Convert a string of type StringT into a std::basic_string<CharT>
template <typename CharT, typename StringT, typename = std::enable_if_t<std::is_integral_v<CharT>>>
std::basic_string<CharT> string_convert(const StringT& str)
{
    using value_type = typename char_traits<StringT>::char_type;

    if constexpr (std::is_same_v<CharT, value_type>)
        return str;

    else if constexpr (std::is_same_v<char, value_type>)
        return s2ws<CharT>(str);

    else if constexpr (std::is_same_v<char, CharT>)
        return ws2s(str);
}
} // namespace pscm
#endif // UTILS_HPP
