/*********************************************************************************/ /**
 * @file cell.hpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#ifndef CELL_HPP
#define CELL_HPP

#include <iostream>

#include "types.hpp"

namespace pscm {

struct Cell : Variant {
    using base_type = Variant;
    using Variant::operator=;
    using Variant::Variant;

    template <typename T>
    operator T() const { return std::get<T>(*this); }
};

size_t store_size();

static const None none{}; //!< void return symbol
static const Nil nil{}; //!< empty list symbol

constexpr bool is_nil(const Cell& cell) { return is_type<Nil>(cell); }
constexpr bool is_none(const Cell& cell) { return is_type<None>(cell); }
constexpr bool is_string(const Cell& cell) { return is_type<String>(cell); }
constexpr bool is_pair(const Cell& cell) { return is_type<Cons*>(cell); }
constexpr bool is_intern(const Cell& cell) { return is_type<Intern>(cell); }
constexpr bool is_symbol(const Cell& cell) { return is_type<Symbol>(cell); }
constexpr bool is_symenv(const Cell& cell) { return is_type<Symenv>(cell); }
constexpr bool is_proc(const Cell& cell) { return is_type<Proc>(cell); }
constexpr bool is_false(const Cell& cell) { return is_type<Bool>(cell) && !std::get<Bool>(cell); }
constexpr bool is_true(const Cell& cell) { return !is_type<Bool>(cell) || std::get<Bool>(cell); }

inline Cell car(const Cell& cons) { return std::get<Cons*>(cons)->first; }
inline Cell cdr(const Cell& cons) { return std::get<Cons*>(cons)->second; }
inline Cell cddr(const Cell& cons) { return cdr(cdr(cons)); }
inline Cell cadr(const Cell& cons) { return car(cdr(cons)); }
inline Cell caddr(const Cell& cons) { return car(cddr(cons)); }

template <typename T>
void set_car(Cell& cons, T&& t) { std::get<Cons*>(cons)->first = std::forward<T>(t); }

template <typename T>
void set_cdr(Cell& cons, T&& t) { std::get<Cons*>(cons)->second = std::forward<T>(t); }

//! Predicate return true if cell is a proper nil terminated list or a circular list.
bool is_list(Cell cell);

//! Return the length of a proper list or the period length of a circular list.
Int list_length(Cell list);

//! Return the kth element of a proper or cicular list.
Cell list_ref(Cell list, Int k);

inline Cell port() { return &std::cout; }

//! Build a String shared-pointer from a zero terminiated Char array.
inline Cell str(const Char* s)
{
    return std::make_shared<String::element_type>(s);
}

template <typename T>
inline Cell num(const T& x)
{
    return Number{ x };
}

template <typename RE, typename IM>
inline Cell num(const RE& x, const IM& y)
{
    return Number{ x, y };
}

Cell cons(Cell&& car, Cell&& cdr);
Cell cons(Cell&& car, const Cell& cdr);
Cell cons(const Cell& car, Cell&& cdr);
Cell cons(const Cell& car, const Cell& cdr);

//! Build a list of all arguments
template <typename T, typename... Args>
Cell list(T&& t, Args&&... args)
{
    return cons(std::forward<T>(t), list(std::forward<Args>(args)...));
}

//! Recursion base case
inline Cell list() { return nil; }

}; // namespace pscm
#endif // CELL_HPP
