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

#include <complex>
#include <deque>
#include <functional>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <unordered_set>

#include <variant>
#include <vector>

#include "symbol.hpp"
#include "utils.hpp"

namespace pscm {

struct Cell; //!< union type, forward declaration
//struct Number;

using Int = int64_t;
using Float = double;

/**
 * @brief
 */
using Nil = nullptr_t;
using None = void*;
using Bool = bool;
using Char = char;
//using Complex    = std::complex<Number>;
using Port = std::ostream;
using Cons = std::pair<Cell, Cell>;
using Func = Cell (*)(const Cell&);
using String = std::shared_ptr<std::basic_string<Char>>;
using Vector = std::shared_ptr<std::vector<Cell>>;
using VecInt = std::shared_ptr<std::vector<Int>>;
using VecFloat = std::shared_ptr<std::vector<Float>>;
//using VecComplex = std::shared_ptr<std::vector<Complex>>;

enum class Intern {
    _and,
    _or,
    _cond,
    _define,
    _setb,
    _begin,
    _apply,
    _lambda
};

//struct Number : NumberVariant {
//    using base_type = NumberVariant;
//    using base_type::operator=;
//    using base_type::base_type;
//};

//constexpr Number operator+(const Number& lhs, const Number& rhs)
//{
//    return std::visit([](auto&& lhs, auto&& rhs) {
//        using std::is_same_v;
//        using std::is_pointer_v;

//        using T_lhs = std::decay_t<decltype(lhs)>;
//        using T_rhs = std::decay_t<decltype(rhs)>;

//        if constexpr (is_pointer_v<T_lhs> && is_pointer_v<T_rhs>)
//            return Number{ *lhs + *rhs };
//        else

//            //if constexpr (std::is_same_v<T_lhs, T_rhs>)
//            return Number{ lhs + rhs };
//        //else

//        //return Number{ std::common_type_t<T_lhs, T_rhs>{ lhs + rhs } };
//    },
//        lhs, rhs);
//    //static_cast<Number::base_type>(lhs), static_cast<Number::base_type>(rhs));
//}

using Variant = std::variant<Nil, None, Bool, Int, Float, Intern, Symbol, String, Cons*, Port*, Func>;

struct Cell : Variant {
    using base_type = Variant;
    using Variant::operator=;
    using Variant::Variant;
};

static const Nil nil{}; //!< empty list symbol
static const None none{}; //!< void return symbol

inline Cell car(const Cell& cons) { return std::get<Cons*>(cons)->first; }
inline Cell cdr(const Cell& cons) { return std::get<Cons*>(cons)->second; }
inline Cell cadr(const Cell& cons) { return car(cdr(cons)); }
inline Cell caddr(const Cell& cons) { return car(cdr(cdr(cons))); }

template <typename T>
void set_car(Cell& cons, T&& t) { std::get<Cons*>(cons)->first = std::forward<T>(t); }

template <typename T>
void set_cdr(Cell& cons, T&& t) { std::get<Cons*>(cons)->second = std::forward<T>(t); }

constexpr bool is_nil(const Cell& cell) { return is_type<Nil>(cell); }
constexpr bool is_none(const Cell& cell) { return is_type<None>(cell); }
constexpr bool is_string(const Cell& cell) { return is_type<String>(cell); }
constexpr bool is_pair(const Cell& cell) { return is_type<Cons*>(cell); }
constexpr bool is_intern(const Cell& cell) { return is_type<Intern>(cell); }
constexpr bool is_symbol(const Cell& cell) { return is_type<Symbol>(cell); }

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

Cell sym(const char* s);

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

inline const char* name(Intern i)
{
    static const char* const names[] = {
        "and", "or", "cond", "define", "setb", "begin"
                                               "apply",
        "lambda"
    };
    return names[static_cast<int>(i)];
}

/**
 * @brief Calls a function for each list item.
 * @param args Argument list: (Func list...))
 * @return none symbol
 */
Cell fun_foreach(const Cell& args);

/**
 * @brief Writes an expression to the standard or supplied output port
 * @param args Argument list (expr {port})
 * @return none symbol
 */
Cell fun_write(const Cell& args);
}; // namespace pscm
#endif // CELL_HPP
