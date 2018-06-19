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
#include <deque>

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
using Store  = std::deque<Cons>;

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

static Store store;

template<typename T, typename ... Args>
T make(Args&& ... args)
{
    if constexpr (std::is_pointer_v<T>)
    {
        return new std::remove_pointer_t<T>{std::forward<Args>(args)...};
    }
    else
    {
        return T{std::forward<Args>(args)...};
    }
};

constexpr bool operator == (None a, None b)
{
    return true;
}

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

constexpr Cell car(const Cell& cons)
{
    return type<Cons*>(cons)->first;
}

constexpr Cell cdr(const Cell& cons)
{
    return type<Cons*>(cons)->second;
}

constexpr Cell cadr(const Cell& cons)
{
    return car(cdr(cons));
}

constexpr void set_car(Cell& cons, const Cell& cell)
{
    type<Cons*>(cons)->first = cell;
}

constexpr void set_cdr(Cell& cons, const Cell& cell)
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
   Cell slow = cell, iter = cell;

   for(; is_pair(iter); iter = cdr(iter), slow = cdr(slow))
       if(!is_pair(iter = cdr(iter)) || iter == slow)
           break;

   return is_nil(iter);
}

inline Cell port()
{
    return &std::cout;
}

inline Cell str(const char *s)
{
    return make<String*>(s);
}

inline Cell cons(const Cell& car, const Cell& cdr)
{
    return &store.emplace_back(car, cdr);
}

template<typename FUN>
Cell func(FUN&& fun)
{
    return make<Func*>(std::forward<FUN>(fun));
}

Cell fun_foreach(const Cell& args);

Cell fun_write(const Cell& args);

}; // namespace pscm
#endif // CELL_HPP
