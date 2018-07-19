/*********************************************************************************/ /**
 * @file cell.cpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#include <assert.h>
#include <deque>

#include "cell.hpp"
#include "proc.hpp"
#include "stream.hpp"
#include "utils.hpp"

namespace pscm {

//! Global cons type store
static std::deque<Cons> store;

size_t store_size()
{
    return store.size();
}

//! Return a new cons-cell from the global cons-store
//! A cons-cell is basically a type tagged pointer into the global cons-store.
template <typename CAR, typename CDR, typename Store = std::deque<Cons>>
static Cons* cons(Store& store, CAR&& car, CDR&& cdr)
{
    return &store.emplace_back(std::forward<CAR>(car), std::forward<CDR>(cdr));
}

Cons* cons(Cell&& car, Cell&& cdr) { return cons(store, std::move(car), std::move(cdr)); }
Cons* cons(Cell&& car, const Cell& cdr) { return cons(store, std::move(car), cdr); }
Cons* cons(const Cell& car, Cell&& cdr) { return cons(store, car, std::move(cdr)); }
Cons* cons(const Cell& car, const Cell& cdr) { return cons(store, car, cdr); }

bool is_equal(const Cell& lhs, const Cell& rhs)
{
    if (lhs == rhs)
        return true;

    if (lhs.index() != rhs.index())
        return false;

    auto test = overloads{
        [](const String& lhs, const String& rhs) -> bool { return *lhs == *rhs; },
        [](const VectorPtr& lhs, const VectorPtr& rhs) -> bool { return lhs == rhs || *lhs == *rhs; },
        [](Cons* lhs, Cons* rhs) -> bool { return is_list_equal(lhs, rhs); },
        [](auto&, auto&) -> bool { return false; }
    };
    return visit(std::move(test),
        static_cast<const Cell::base_type&>(lhs),
        static_cast<const Cell::base_type&>(rhs));
}

bool is_list(Cell cell)
{
    for (Cell slow{ cell }; is_pair(cell); cell = cdr(cell), slow = cdr(slow))
        if (!is_pair(cell = cdr(cell)) || cell == slow)
            break;
    return is_nil(cell);
}

bool is_list_equal(Cell lhs, Cell rhs)
{
    for (/* */; is_pair(lhs) && is_pair(rhs); lhs = cdr(lhs), rhs = cdr(rhs))
        if (!is_equal(car(lhs), car(rhs)))
            return false;

    return is_equal(lhs, rhs);
}

Int list_length(Cell list)
{
    Int len = 0, slw = 0;

    for (Cell slow{ list }; is_pair(list); list = cdr(list), slow = cdr(slow), ++len, ++slw) {
        ++len;
        if (!is_pair(list = cdr(list)))
            break;

        if (list == slow)
            return ++slw;
    }
    return len;
}

Cell list_ref(Cell list, Int k)
{
    for (/* */; k > 0 && is_pair(list); list = cdr(list), --k)
        ;

    !k || (throw std::invalid_argument("invalid list length"), 0);
    return car(list);
}
}
