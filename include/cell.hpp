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

#include <iostream> // to be phased out

#include "types.hpp"

namespace pscm {

size_t store_size();

/**
 * @brief A scheme cell is a variant type of all supported scheme types.
 */
struct Cell : Variant {
    using base_type = Variant;
    using Variant::Variant;
    using Variant::operator=;

    /**
     * @brief Type conversion operator to return the value hold by this Cell.
     * @throw std::bad_variant_access
     */
    template <typename T>
    operator T() const { return std::get<T>(static_cast<Variant>(*this)); }
};

static const None none{}; //!< void return symbol
static const Nil nil{}; //!< empty list symbol

constexpr bool is_nil(const Cell& cell) { return is_type<Nil>(cell); }
constexpr bool is_none(const Cell& cell) { return is_type<None>(cell); }
constexpr bool is_bool(const Cell& cell) { return is_type<Bool>(cell); }
constexpr bool is_char(const Cell& cell) { return is_type<Char>(cell); }
constexpr bool is_string(const Cell& cell) { return is_type<String>(cell); }
constexpr bool is_pair(const Cell& cell) { return is_type<Cons*>(cell); }
constexpr bool is_intern(const Cell& cell) { return is_type<Intern>(cell); }
constexpr bool is_symbol(const Cell& cell) { return is_type<Symbol>(cell); }
constexpr bool is_symenv(const Cell& cell) { return is_type<Symenv>(cell); }
constexpr bool is_proc(const Cell& cell) { return is_type<Proc>(cell); }
constexpr bool is_false(const Cell& cell) { return is_type<Bool>(cell) && !static_cast<Bool>(cell); }
constexpr bool is_true(const Cell& cell) { return !is_type<Bool>(cell) || static_cast<Bool>(cell); }

/**
 * @brief Scheme equal? predicate to test two cells for same content.
 *
 * Two lists or vectors are considered equal, if each
 * item is equal. Two strings are equal if each individual
 * character is equal.
 */
inline bool is_equal(const Cell& lhs, const Cell& rhs);

/**
 * @brief Construct a new cons cell-pair from the global cell store and
 *        return a pointer to it. Pointer life time is managed by the
 *        garbage collector.
 *
 * @param  car Cell to assign to Cons->first.
 * @param  cdr Cell to assign to Cons->second.
 * @return Pointer to a new Cons pair.
 */
Cons* cons(Cell&& car, Cell&& cdr);
Cons* cons(Cell&& car, const Cell& cdr);
Cons* cons(const Cell& car, Cell&& cdr);
Cons* cons(const Cell& car, const Cell& cdr);

//! Convenience functions to access a list of Cons cell-pairs.
inline Cell& car(Cons* cons) { return cons->first; }
inline Cell& cdr(Cons* cons) { return cons->second; }
inline Cell& cddr(Cons* cons) { return cdr(cdr(cons)); }
inline Cell& cadr(Cons* cons) { return car(cdr(cons)); }
inline Cell& caddr(Cons* cons) { return car(cddr(cons)); }

//! Set the first cell of a Cons cell-pair.
template <typename T>
void set_car(Cons* cons, T&& t) { cons->first = std::forward<T>(t); }

//! Set the second cell of a Cons cell-pair.
template <typename T>
void set_cdr(Cons* cons, T&& t) { cons->second = std::forward<T>(t); }

//! Predicate return true if cell is a proper nil terminated list or a circular list.
bool is_list(Cell cell);

bool is_list_equal(Cell lhs, Cell rhs);

//! Return the length of a proper list or the period length of a circular list.
Int list_length(Cell list);

//! Return the kth element of a proper or cicular list.
Cell list_ref(Cell list, Int k);

inline Cell port() { return &std::cout; }

//! Build a cons list of all arguments
template <typename T, typename... Args>
Cons* list(T&& t, Args&&... args)
{
    return cons(std::forward<T>(t), list(std::forward<Args>(args)...));
}
//! Recursion base case
inline Cell list() { return nil; }

/**
 * @brief Build a cons list from all arguments directly in
 *        in argument cons cell array.
 *
 * The cons array size must be equal or greater the number of
 * remaining arguments. An insufficient array size is not detected.
 * This array embedded cons-list is used for short temporary
 * argument lists to circumvent to unecessarly fill the
 * global cell store.
  */
template <typename T, typename... Args>
Cons* alist(Cons cons[], T&& t, Args&&... args)
{
    cons->first = std::forward<T>(t);
    cons->second = alist(cons + 1, std::forward<Args>(args)...);
    return cons;
}

//! Recursion base case
inline Cell alist(Cons*) { return nil; }

}; // namespace pscm
#endif // CELL_HPP
