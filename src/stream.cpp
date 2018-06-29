/*********************************************************************************/ /**
 * @file stream.cpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#include "stream.hpp"

namespace pscm {

/**
 * @brief Output stream operator for Cons type arguments.
 */
static std::ostream& operator<<(std::ostream& os, Cons* cons)
{
    Cell iter{ cons };

    os << '(' << car(iter);
    iter = cdr(iter);

    for (Cell slow{ iter }; is_pair(iter); iter = cdr(iter), slow = cdr(slow)) {
        os << ' ' << car(iter);

        if (!is_pair(iter = cdr(iter)) || slow == iter) {
            if (slow == iter)
                return os << " ...)"; // circular list detected

            break;
        }
        os << ' ' << car(iter);
    }
    if (is_nil(iter))
        os << ')'; // list end
    else
        os << " . " << iter << ')'; // dotted pair end

    return os;
}

/**
 * @brief Output stream operator for Cell type arguments.
 */
std::ostream& operator<<(std::ostream& os, const Cell& cell)
{
    overloads fun{
        [&os](Nil) { os << "()"; },
        [&os](None) { os << "#none"; },
        [&os](Bool arg) { os << (arg ? "#t" : "#f"); },
        [&os](Number arg) { os << arg; },
        [&os](Intern arg) { os << "<intern " << static_cast<int>(arg) << '>'; },
        [&os](String arg) { os << '"' << *arg << '"'; },
        [&os](Symbol arg) { os << '<' << arg.name() << '>'; },
        [&os](Symenv arg) { os << "<symenv>"; },
        [&os](Proc arg) { os << "<proc>"; },
        [&os](Port*) { os << "port"; },
        [&os](Cons* arg) { os << arg; },

        /* catch missing overloads and emit compile time error message */
        [](auto arg) { static_assert(always_false<decltype(arg)>::value, "callable overload is missing"); }
    };
    std::visit(std::move(fun), static_cast<const Cell::base_type&>(cell));
    return os;
}

}; // namespace pscm
