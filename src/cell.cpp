#include "cell.hpp"

namespace pscm {

Int use_count(const Cell& cell)
{
    static overloads pointer{
        [](const StringPtr& p) -> Int { return p.use_count(); },
        [](const RegexPtr& p) -> Int { return p.use_count(); },
        [](const VectorPtr& p) -> Int { return p.use_count(); },
        [](const SymenvPtr& p) -> Int { return p.use_count(); },
        [](const FunctionPtr& p) -> Int { return p.use_count(); },
        [](auto&) -> Int { return 0; },
    };
    return visit(pointer, static_cast<const Cell::base_type&>(cell));
}

//! Predicate returns true if each list item from both lists is equal according to @ref pscm::is_equal.
static bool is_list_equal(Cell lhs, Cell rhs)
{
    for (/* */; is_pair(lhs) && is_pair(rhs); lhs = cdr(lhs), rhs = cdr(rhs))
        if (!is_equal(car(lhs), car(rhs)))
            return false;

    return is_equal(lhs, rhs);
}

bool is_equal(const Cell& lhs, const Cell& rhs)
{
    if (lhs == rhs)
        return true;

    if (lhs.index() != rhs.index())
        return false;

    // clang-format off
    static overloads test{
        [](Cons* lhs, Cons* rhs)                       -> bool { return is_list_equal(lhs, rhs); },
        [](const StringPtr& lhs, const StringPtr& rhs) -> bool { return *lhs == *rhs; },
        [](const VectorPtr& lhs, const VectorPtr& rhs) -> bool {
            return lhs == rhs
                    || (lhs->size() == rhs->size()
                        && std::equal(lhs->begin(), lhs->end(), rhs->begin(), is_equal));
        },
        [](auto&, auto&) -> bool { return false; }
    }; // clang-format on

    return visit(test, static_cast<const Cell::base_type&>(lhs), static_cast<const Cell::base_type&>(rhs));
}

bool is_list(Cell cell)
{
    for (Cell slow{ cell }; is_pair(cell); cell = cdr(cell), slow = cdr(slow))
        if (!is_pair(cell = cdr(cell)) || cell == slow)
            break;
    return is_nil(cell);
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

    !k || ((void)(throw std::invalid_argument("invalid list index")), 0);
    return car(list);
}
}
