#ifndef CELL_HPP
#define CELL_HPP

#include "number.hpp"
#include "procedure.hpp"
#include "stream.hpp"
#include "types.hpp"
#include "utils.hpp"

namespace pscm {

/**
 * A scheme cell is a variant type of all supported scheme types.
 */
struct Cell : Variant {
    using base_type = Variant;
    using Variant::Variant;
};

/**
 * Exception class to throw an invalid cell variant access error.
 */
template <typename CellType>
struct bad_cell_access : public std::bad_variant_access {
    bad_cell_access() noexcept
        : _reason("invalid type ")
    {
        _reason.append(type_name());
    }
    bad_cell_access(const Cell& cell)
    {
        std::ostringstream os;
        os << "argument " << cell << " must be of type " << type_name();
        _reason = os.str();
    }
    const char* what() const noexcept override
    {
        return _reason.c_str();
    }

private:
    std::string _reason;

    /**
     * Return a textual representation of template argument type.
     */
    constexpr const char* type_name()
    {
        using T = std::decay_t<CellType>;
        if constexpr (std::is_same_v<T, Nil>)
            return "()";
        else if constexpr (std::is_same_v<T, None>)
            return "#<none>";
        else if constexpr (std::is_same_v<T, Intern>)
            return "#<primop>";
        else if constexpr (std::is_same_v<T, Bool>)
            return "#<boolean>";
        else if constexpr (std::is_same_v<T, Char>)
            return "#<character>";
        else if constexpr (std::is_same_v<T, Number>)
            return "#<number>";
        else if constexpr (std::is_same_v<T, Cons*>)
            return "#<cons>";
        else if constexpr (std::is_same_v<T, StringPtr>)
            return "#<string>";
        else if constexpr (std::is_same_v<T, RegexPtr>)
            return "#<regex>";
        else if constexpr (std::is_same_v<T, VectorPtr>)
            return "#<vector>";
        else if constexpr (std::is_same_v<T, FunctionPtr>)
            return "#<function>";
        else if constexpr (std::is_same_v<T, Port>)
            return "#<port>";
        else if constexpr (std::is_same_v<T, Symbol>)
            return "#<symbol>";
        else if constexpr (std::is_same_v<T, SymenvPtr>)
            return "#<environment>";
        else if constexpr (std::is_same_v<T, Procedure>)
            return "#<procedure>";
        else
            return "#<unknown>";
    }
};

template <typename T>
T& get(Cell& cell)
{
    try {
        return std::get<T>(static_cast<Variant&>(cell));

    } catch (std::bad_variant_access&) {
        throw bad_cell_access<T>(cell);
    }
}

template <typename T>
T&& get(Cell&& cell)
{
    try {
        return std::get<T>(static_cast<Variant&&>(std::move(cell)));

    } catch (std::bad_variant_access&) {
        throw bad_cell_access<T>(cell);
    }
}

template <typename T>
const T& get(const Cell& cell)
{
    try {
        return std::get<T>(static_cast<Variant&>(const_cast<Cell&>(cell)));

    } catch (std::bad_variant_access&) {
        throw bad_cell_access<T>(cell);
    }
}

template <typename T>
const T&& get(const Cell&& cell)
{
    try {
        return std::get<T>(static_cast<const Variant&&>(std::move(cell)));

    } catch (std::bad_variant_access&) {
        throw bad_cell_access<T>(cell);
    }
}

//! Return the use count of a shared pointer cell or zero for a value type cell;
Int use_count(const Cell&);

inline bool is_nil(const Cell& cell) { return is_type<Nil>(cell); }
inline bool is_none(const Cell& cell) { return is_type<None>(cell); }
inline bool is_bool(const Cell& cell) { return is_type<Bool>(cell); }
inline bool is_char(const Cell& cell) { return is_type<Char>(cell); }
inline bool is_string(const Cell& cell) { return is_type<StringPtr>(cell); }
inline bool is_regex(const Cell& cell) { return is_type<RegexPtr>(cell); }
inline bool is_pair(const Cell& cell) { return is_type<Cons*>(cell); }
inline bool is_intern(const Cell& cell) { return is_type<Intern>(cell); }
inline bool is_port(const Cell& cell) { return is_type<Port>(cell); }
inline bool is_number(const Cell& cell) { return is_type<Number>(cell); }
inline bool is_symbol(const Cell& cell) { return is_type<Symbol>(cell); }
inline bool is_symenv(const Cell& cell) { return is_type<SymenvPtr>(cell); }
inline bool is_vector(const Cell& cell) { return is_type<VectorPtr>(cell); }
inline bool is_func(const Cell& cell) { return is_type<FunctionPtr>(cell); }
inline bool is_proc(const Cell& cell) { return is_type<Procedure>(cell); }
inline bool is_macro(const Cell& cell) { return is_proc(cell) && get<Procedure>(cell).is_macro(); }
inline bool is_false(const Cell& cell) { return is_type<Bool>(cell) && !get<Bool>(cell); }
inline bool is_true(const Cell& cell) { return !is_type<Bool>(cell) || get<Bool>(cell); }
inline bool is_else(const Cell& cell) { return is_intern(cell) && get<Intern>(cell) == Intern::_else; }
inline bool is_arrow(const Cell& cell) { return is_intern(cell) && get<Intern>(cell) == Intern::_arrow; }
inline bool is_exit(const Cell& cell) { return is_intern(cell) && get<Intern>(cell) == Intern::op_exit; }

/**
 * Scheme equal? predicate to test two cells for same content.
 *
 * Two lists or vectors are considered equal, if each
 * item is equal. Two strings are equal if each individual
 * character is equal.
 */
bool is_equal(const Cell& lhs, const Cell& rhs);

//! Convenience functions to access a list of Cons cell-pairs.
inline const Cell& car(const Cell& cons) { return get<0>(*get<Cons*>(cons)); }
inline const Cell& cdr(const Cell& cons) { return get<1>(*get<Cons*>(cons)); }
inline const Cell& caar(const Cell& cons) { return car(car(cons)); }
inline const Cell& cdar(const Cell& cons) { return cdr(car(cons)); }
inline const Cell& cddr(const Cell& cons) { return cdr(cdr(cons)); }
inline const Cell& cadr(const Cell& cons) { return car(cdr(cons)); }
inline const Cell& caddr(const Cell& cons) { return car(cddr(cons)); }

//! Set the first cell of a Cons cell-pair.
template <typename T>
void set_car(const Cell& cons, T&& t) { get<0>(*std::get<Cons*>(cons)) = std::forward<T>(t); }

//! Set the second cell of a Cons cell-pair.
template <typename T>
void set_cdr(const Cell& cons, T&& t) { get<1>(*std::get<Cons*>(cons)) = std::forward<T>(t); }

//! Return a new cons-cell from the global cons-store
//! A cons-cell is basically a type tagged pointer into the global cons-store.
template <typename Store, typename CAR, typename CDR>
Cons* cons(Store& store, CAR&& car, CDR&& cdr)
{
    return &store.emplace_back(std::forward<CAR>(car), std::forward<CDR>(cdr), false);
}

//! Recursion base case
template <typename Store>
Cell list(Store&) { return nil; }

//! Build a cons list of all arguments
template <typename Store, typename T, typename... Args>
Cons* list(Store& store, T&& t, Args&&... args)
{
    return cons(store, std::forward<T>(t), list(store, std::forward<Args>(args)...));
}

//! Predicate returns true if cell is a proper nil terminated list or a circular list.
bool is_list(Cell cell);

//! Return the length of a proper list or the period length of a circular list.
Int list_length(Cell list);

//! Return the kth element of a proper or cicular list.
Cell list_ref(Cell list, Int k);

/**
 * Build a cons list from all arguments directly in
 * in argument cons cell array.
 *
 * This array embedded cons-list is used for short temporary
 * argument lists to circumvent to unecessarly fill the
 * global cell store.
 *
 * The cons array size must be equal or greater the number of
 * remaining arguments. An insufficient array size is an compile
 * time error.
 */
template <size_t size, typename T, typename... Args>
Cons* alist(Cons (&cons)[size], T&& t, Args&&... args)
{
    get<0>(cons[0]) = std::forward<T>(t);

    if constexpr (size > 1) {
        get<1>(cons[0]) = alist(reinterpret_cast<Cons(&)[size - 1]>(cons[1]), std::forward<Args>(args)...);
    } else
        get<0>(cons[0]) = nil;

    return &cons[0];
}

//! Recursion base case, if list is shorter then the array size.
template <size_t size>
Nil alist(Cons (&)[size]) { return nil; }

//! Error condition if list is longer then the array size.
template <typename T1, typename T2, typename... Args>
Nil alist(Cons (&)[1], T1&&, T2&&, Args&&...)
{
    throw std::invalid_argument("invalid cons array size");
    return nil;
}
}
#endif // CELL_HPP
