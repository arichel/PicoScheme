/********************************************************************************//**
 * @file cell.hpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#ifndef CELL_HPP
#define CELL_HPP

#include <utility>
#include <variant>
#include <string>
#include <memory>
#include <functional>
#include <iostream>

namespace pscm {

struct Cell;
struct Symbol;
struct Record;
struct Vector;
struct Dict;

using Nil    = nullptr_t;
using None   = struct{};
using Bool   = bool;
using Int    = int64_t;
using Float  = double;
using String = std::string;
using Port   = std::ostream;
using Cons   = std::pair<Cell, Cell>;
using Func   = std::function<Cell(const Cell&)>;

static const Nil  nil {};
static const None none {};

using Variant = std::variant<Nil, None, Bool, Int, Float, String*, Port*, Cons*, Func*>;
enum struct Type : size_t {Nil, None, Bool, Int, Float, String, Cons, Lambda};

struct Cell : Variant
{
    using base_type = Variant;
    using Variant::operator=;
    using Variant::Variant;
};

template<typename T, typename ... Args> T make(Args&& ... args)
{
    if constexpr (std::is_pointer_v<T>)
        return new std::remove_pointer_t<T>{std::forward<Args>(args)...};
    else
        return T{std::forward<Args>(args)...};
};

template<typename T>
constexpr T type(const Cell& cell)
{
    return std::get<T>(static_cast<Cell::base_type>(cell));
}

template<typename T>
constexpr bool is_type(const Cell& cell)
{
    return std::holds_alternative<T>(cell);
}

Cell car(const Cell& cons)
{
    return type<Cons*>(cons)->first;
}

Cell cdr(const Cell& cons)
{
    return type<Cons*>(cons)->second;
}

Cell cadr(const Cell& cons)
{
    return car(cdr(cons));
}

void set_car(Cell& cons, const Cell& cell)
{
    type<Cons*>(cons)->first = cell;
}

void set_cdr(Cell& cons, const Cell& cell)
{
    type<Cons*>(cons)->second = cell;
}

constexpr bool is_nil(const Cell& cell)
{
    return is_type<Nil>(cell);
}

constexpr bool is_atom(const Cell& cell)
{
    return is_type<Bool>(cell)
            || is_type<Int>(cell)
            || is_type<Float>(cell);
}

constexpr bool is_string(const Cell& cell)
{
    return is_type<String*>(cell);
}

constexpr bool is_pair(const Cell& cell)
{
    return is_type<Cons*>(cell);
}

constexpr bool is_list(const Cell& cell)
{
    return is_nil(cell) || (is_pair(cell) && (is_pair(cdr(cell)) || is_nil(cdr(cell))));
}

Cell port()
{
    return &std::cout;
}

Cell str(const char *s)
{
    return make<String*>(s);
}

Cell cons(const Cell& car, const Cell& cdr)
{
    return make<Cons*>(car, cdr);
}

template<typename FUN>
Cell func(FUN&& fun)
{
    return make<Func*>(std::forward<FUN>(fun));
}

template<typename Fun>
void foreach(Fun&& fun, Cell& list)
{
    while(!is_nil(list))
    {
        fun(car(list));
        list = cdr(list);
    }
}

Cell fun_foreach(const Cell& args)
{
    auto fun = type<Func*>(car(args));

    foreach(*fun, cdr(args));
    return none;
}

}; // namespace pscm
#endif // CELL_HPP
