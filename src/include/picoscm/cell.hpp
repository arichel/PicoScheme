/**
 * @file cell.hpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 */
#ifndef CELL_HPP
#define CELL_HPP

#include <functional>

#include "clock.hpp"
#include "number.hpp"
#include "port.hpp"
#include "procedure.hpp"
#include "types.hpp"

namespace pscm {

//! A scheme Cell is a Variant type of all supported scheme types.
struct Cell : Variant {
    using base_type = Variant;
    using Variant::Variant;
};

template <typename CellType>
struct bad_cell_access;

//! Wrappers around std::get to access the Variant type and rethrow
//! a more descriptive pscm::bad_cell_access exception in case of
//! an invalid type access atempt.
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

template <typename Cell>
struct hash {
    using argument_type = Cell;
    using result_type = std::size_t;

    result_type operator()(const Cell& cell) const
    {
        // clang-format off
        static overloads hash{
            [](None)                 -> result_type { return 0; },
            [](Nil)                  -> result_type { return 0; },
            [](Bool arg)             -> result_type { return static_cast<result_type>(arg); },
            [](Char arg)             -> result_type { return std::hash<Char>{}(arg); },
            [](Intern arg)           -> result_type { return std::hash<Intern>{}(arg); },
            [](Number arg)           -> result_type { return Number::hash{}(arg); },
            [](const Procedure& arg) -> result_type { return Procedure::hash{}(arg); },
            [](const Symbol& arg)    -> result_type { return Symbol::hash{}(arg); },
            [](const StringPtr& arg) -> result_type { return std::hash<String>{}(*arg);},
            [](auto& arg)            -> result_type { return std::hash<std::decay_t<decltype(arg)>>{}(arg); },
        }; // clang-format on
        return std::visit(hash, static_cast<const typename Cell::base_type&>(cell));
    }
};

//! Return the use count of a shared pointer cell or zero for a value type cell;
Int use_count(const Cell&);

/**
 * Conveniance predicates to test the Cell type.
 */
// clang-format off
inline bool is_nil    (const Cell& cell) { return is_type<Nil>(cell); }
inline bool is_none   (const Cell& cell) { return is_type<None>(cell); }
inline bool is_bool   (const Cell& cell) { return is_type<Bool>(cell); }
inline bool is_char   (const Cell& cell) { return is_type<Char>(cell); }
inline bool is_string (const Cell& cell) { return is_type<StringPtr>(cell); }
inline bool is_regex  (const Cell& cell) { return is_type<RegexPtr>(cell); }
inline bool is_dict   (const Cell& cell) { return is_type<MapPtr>(cell); }
inline bool is_pair   (const Cell& cell) { return is_type<Cons*>(cell); }
inline bool is_intern (const Cell& cell) { return is_type<Intern>(cell); }
inline bool is_port   (const Cell& cell) { return is_type<PortPtr>(cell); }
inline bool is_clock  (const Cell& cell) { return is_type<ClockPtr>(cell); }
inline bool is_number (const Cell& cell) { return is_type<Number>(cell); }
inline bool is_symbol (const Cell& cell) { return is_type<Symbol>(cell); }
inline bool is_symenv (const Cell& cell) { return is_type<SymenvPtr>(cell); }
inline bool is_vector (const Cell& cell) { return is_type<VectorPtr>(cell); }
inline bool is_func   (const Cell& cell) { return is_type<FunctionPtr>(cell); }
inline bool is_proc   (const Cell& cell) { return is_type<Procedure>(cell); }
inline bool is_macro  (const Cell& cell) { return is_proc(cell) && get<Procedure>(cell).is_macro(); }
inline bool is_false  (const Cell& cell) { return is_type<Bool>(cell) && !get<Bool>(cell); }
inline bool is_true   (const Cell& cell) { return !is_type<Bool>(cell) || get<Bool>(cell); }
inline bool is_else   (const Cell& cell) { return is_intern(cell) && get<Intern>(cell) == Intern::_else; }
inline bool is_arrow  (const Cell& cell) { return is_intern(cell) && get<Intern>(cell) == Intern::_arrow; }
inline bool is_exit   (const Cell& cell) { return is_intern(cell) && get<Intern>(cell) == Intern::op_exit; }
// clang-format on

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

//! Predicate returns true if cell is a proper, nil terminated Cons-cell list or a circular list.
bool is_list(Cell cell);

//! Return the length of a proper Cons-cell list or the period length of a circular list.
Int list_length(Cell list);

//! Return the kth element of a proper or cicular list.
Cell list_ref(Cell list, Int k);

/**
 * Create a new Cons-cell, directly at the provided cons-store container and
 * return a pointer to this new cons-cell.
 *
 * This new Cons-cell is initialized with car and cdr argument values.
 * Store must be a container like std::deque or std::list, where mutating update
 * operations don't invalidate pointers to previously inserted elements.
 */
template <typename StoreT, typename CAR, typename CDR>
Cons* cons(StoreT& store, CAR&& car, CDR&& cdr)
{
    return &store.emplace_back(std::forward<CAR>(car), std::forward<CDR>(cdr), /*gc-flag*/ false);
}

//! Build an embedded cons-list of all arguments on the provided Cons-cell store
//! container and return a pointer to the list head.
template <typename Store, typename T, typename... Args>
Cons* list(Store& store, T&& t, Args&&... args)
{
    return cons(store, std::forward<T>(t), list(store, std::forward<Args>(args)...));
}

//! Recursion base case
template <typename Store>
Cell list(Store&) { return nil; }

/**
 * Build an array embedded cons-list of all arguments directly in
 * in the provided cons-cell array.
 *
 * This array embedded cons-list is used for short temporary
 * argument lists, to circumvent filling the global cell store
 * unecessarly. The cons array size must be equal or greater then
 * the number of remaining arguments. An insufficient array size
 * is an compile time error.
 */
template <size_t size, typename T, typename... Args>
Cons* list(Cons (&cons)[size], T&& t, Args&&... args)
{
    get<0>(cons[0]) = std::forward<T>(t);

    if constexpr (size > 1) {
        get<1>(cons[0]) = list(reinterpret_cast<Cons(&)[size - 1]>(cons[1]),
            std::forward<Args>(args)...);
    } else
        get<1>(cons[0]) = nil;

    return &cons[0];
}

//! Recursion base case, if list is shorter then the array size.
template <size_t size>
Nil list(Cons (&)[size]) { return nil; }

//! Error condition if list is longer then the array size.
template <typename T1, typename T2, typename... Args>
Nil list(Cons (&)[1], T1&&, T2&&, Args&&...)
{
    throw std::invalid_argument("invalid cons array size");
    return nil;
}

//! Create a new scheme string and initialize it with a copy of the argument string.
template <typename StringT>
StringPtr str(const StringT& str)
{
    return std::make_shared<String>(string_convert<Char>(str));
}

//! Create a new scheme vector of argument size and initial value.
template <typename T>
VectorPtr vec(size_t size, T&& val)
{
    return std::make_shared<VectorPtr::element_type>(size, std::forward<T>(val));
}

//! Create a new scheme regular-expression object.
template <typename StringT>
RegexPtr regex(const StringT& str)
{
    using regex = RegexPtr::element_type;
    regex::flag_type flags = regex::ECMAScript | regex::icase;
    return std::make_shared<RegexPtr::element_type>(string_convert<Char>(str), flags);
}

template <typename Scheme, typename Symenv, typename... Args>
Cell apply2(Scheme& scm, const Symenv& env, const Cell& proc, Args&&... args)
{
    Cons lst[3], arg[2];
    std::vector<Cons> vec;

    Cell expr = pscm::list(lst, Intern::_apply, proc,
        pscm::list(arg, Intern::_quote, pscm::list(vec, std::forward<Args>(args)...)));
    //    std::wcout << expr << std::endl;
    return scm.eval(env, expr);
}

template <typename Cell>
struct less {

    template <typename Scheme, typename Symenv>
    less(Scheme& scm, const Symenv& env, const Cell& comp)
        : compare{ [&scm, env, comp](const Cell& lhs, const Cell& rhs) -> bool {
            Cons cns[5];
            return !is_false(scm.eval(env, list(cns, Intern::_apply, comp, lhs, rhs, nil)));
        } }
    {
    }

    less()
    {
        // clang-format off
        static overloads comp{
            [](Bool lhs, Bool rhs)                         -> bool { return lhs < rhs; },
            [](Char lhs, Char rhs)                         -> bool { return lhs < rhs; },
            [](Intern lhs, Intern rhs)                     -> bool { return lhs < rhs; },
            [](Number lhs, Number rhs)                     -> bool { return lhs < rhs; },
            [](const Symbol& lhs, const Symbol& rhs)       -> bool { return lhs.value() < rhs.value(); },
            [](const StringPtr& lhs, const StringPtr& rhs) -> bool { return *lhs < *rhs;},
            [](const ClockPtr& lhs, const ClockPtr& rhs)   -> bool { return lhs->toc() < rhs->toc();},
            [](auto&, auto&)                               -> bool { throw std::invalid_argument("undefined < comparision operator"); },
        }; // clang-format on

        compare = [](const Cell& lhs, const Cell& rhs) -> bool {
            return std::visit(comp,
                static_cast<const typename Cell::base_type&>(lhs),
                static_cast<const typename Cell::base_type&>(rhs));
        };
    }
    bool operator()(const Cell& lhs, const Cell& rhs) const { return compare(lhs, rhs); }

private:
    std::function<bool(const Cell&, const Cell&)> compare;
};

//! Exception class to throw an invalid cell variant access error with
//! descriptive error message.
template <typename CellType>
struct bad_cell_access : public std::bad_variant_access {
    bad_cell_access() noexcept
        : _reason("invalid type ")
    {
        _reason.append(type_name());
    }
    bad_cell_access(const Cell& cell)
    {
        using Port = StringPort<Char>;

        Port os{ Port::out };
        os << cell;

        _reason = std::string{ "argument " }
                      .append(string_convert<char>(os.str()))
                      .append(" is not a ")
                      .append(type_name());
    }
    const char* what() const noexcept override { return _reason.c_str(); }

private:
    std::string _reason;

    //! Return a textual representation of the template argument type.
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
        else if constexpr (std::is_same_v<T, ClockPtr>)
            return "#<clock>";
        else if constexpr (std::is_same_v<T, RegexPtr>)
            return "#<regex>";
        else if constexpr (std::is_same_v<T, MapPtr>)
            return "#<dict>";
        else if constexpr (std::is_same_v<T, VectorPtr>)
            return "#<vector>";
        else if constexpr (std::is_same_v<T, FunctionPtr>)
            return "#<function>";
        else if constexpr (std::is_same_v<T, PortPtr>)
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
} // namespace pscm

namespace std {

} // namespace std

#endif // CELL_HPP
