/*********************************************************************************/ /**
 * @file stream.cpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#include <cstring>

#include "port.hpp"
#include "scheme.hpp"
#include "utils.hpp"

namespace pscm {

/**
 * Output stream operator for Cons cell lists.
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
 * Output stream operator for Symbols.
 */
static std::ostream& operator<<(std::ostream& os, const Symbol& sym)
{
    const String& name = sym.value();

    if (name.find_first_of(' ') != String::npos)
        return os << '|' << name << '|';
    else
        return os << name;
}

static std::ostream& operator<<(std::ostream& os, const DisplayManip<StringPtr>& manip)
{
    const StringPtr::element_type& str = *manip.value;

    for (auto cp = str.begin(), end = str.end(); cp != end; ++cp)
        if (*cp == '\\' && cp + 1 < end)
            switch (*(++cp)) {
            case 'a':
                os << '\a';
                break;
            case 'b':
                os << '\b';
                break;
            case 'n':
                os << '\n';
                break;
            case 'r':
                os << '\r';
                break;
            case 't':
                os << '\t';
                break;
            default:
                os << *cp;
            }
        else
            os << *cp;
    return os;
}

static std::ostream& operator<<(std::ostream& os, const VectorPtr& vptr)
{
    if (vptr->size()) {
        os << "#(" << vptr->front();
        for (auto ip = vptr->begin() + 1, ie = vptr->end(); ip != ie; ++ip)
            os << ' ' << *ip;
        return os << ')';
    } else
        return os << "#()";
}

std::ostream& operator<<(std::ostream& os, Intern opcode)
{
    switch (opcode) {
    case Intern::_or:
        return os << "or";
    case Intern::_and:
        return os << "and";
    case Intern::_if:
        return os << "if";
    case Intern::_cond:
        return os << "cond";
    case Intern::_else:
        return os << "else";
    case Intern::_arrow:
        return os << "=>";
    case Intern::_when:
        return os << "when";
    case Intern::_unless:
        return os << "unless";
    case Intern::_define:
        return os << "define";
    case Intern::_setb:
        return os << "set!";
    case Intern::_begin:
        return os << "begin";
    case Intern::_lambda:
        return os << "lambda";
    case Intern::_macro:
        return os << "define-macro";
    case Intern::_apply:
        return os << "apply";
    case Intern::_quote:
        return os << "quote";
    case Intern::_quasiquote:
        return os << "quasiquote";
    case Intern::_unquote:
        return os << "unquote";
    case Intern::_unquotesplice:
        return os << "unquote-splicing";
    default:
        return os << "#<primop>";
    }
}

static std::ostream& operator<<(std::ostream& os, Procedure proc)
{
    return proc.is_macro() ? os << "#<macro>" : os << "#<clojure>";
}

/**
 * Output stream operator for Cell type arguments.
 */
std::ostream& operator<<(std::ostream& os, const Cell& cell)
{
    // clang-format off
    overloads stream{
        [&os](None)                   -> std::ostream& { return os << "#<none>"; },
        [&os](Nil)                    -> std::ostream& { return os << "()"; },
        [&os](Bool arg)               -> std::ostream& { return os << (arg ? "#t" : "#f"); },
        [&os](Char arg)               -> std::ostream& { return os << "#\\" << arg; },
        [&os](const StringPtr& arg)   -> std::ostream& { return os << '"' << *arg << '"';},
        [&os](const RegexPtr&)        -> std::ostream& { return os << "#<regex>"; },
        [&os](const SymenvPtr& arg)   -> std::ostream& { return os << "#<symenv " << arg.get() << '>'; },
        [&os](const FunctionPtr& arg) -> std::ostream& { return os << "#<function " << arg->name() << '>'; },
        [&os](const PortPtr&)         -> std::ostream& { return os << "#<port>"; },
        [&os](auto& arg)              -> std::ostream& { return os << arg; }
    }; // clang-format on

    return std::visit(std::move(stream), static_cast<const Cell::base_type&>(cell));
}

/**
 * Overloaded output stream operator for Cell types as scheme (display <expr>) function
 * representation.
 */
std::ostream& operator<<(std::ostream& os, DisplayManip<Cell> manip)
{
    // clang-format off
    overloads stream{
        [](None)                    { },
        [&os](Char arg)             { os << arg; },
        [&os](const StringPtr& arg) { os << display(arg);},

        // For all other types call normal cell-stream overloaded operator:
        [&os, &manip](auto&)        { os << manip.value; }
    }; // clang-format on

    std::visit(std::move(stream), static_cast<const Cell::base_type&>(manip.value));
    return os;
}
} // namespace pscm
