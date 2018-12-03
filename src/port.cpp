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
static std::wostream& operator<<(std::wostream& os, Cons* cons)
{
    Cell iter{ cons };

    os << L'(' << car(iter);
    iter = cdr(iter);

    for (Cell slow{ iter }; is_pair(iter); iter = cdr(iter), slow = cdr(slow)) {
        os << L' ' << car(iter);

        if (!is_pair(iter = cdr(iter)) || slow == iter) {
            if (slow == iter)
                return os << L" ...)"; // circular list detected

            break;
        }
        os << L' ' << car(iter);
    }
    if (is_nil(iter))
        os << L')'; // list end
    else
        os << L" . " << iter << L')'; // dotted pair end

    return os;
}

/**
 * Output stream operator for Symbols.
 */
static std::wostream& operator<<(std::wostream& os, const Symbol& sym)
{
    const String& name = sym.value();

    if (name.find_first_of(' ') != String::npos)
        return os << L'|' << name << L'|';
    else
        return os << name;
}

static std::wostream& operator<<(std::wostream& os, const DisplayManip<StringPtr>& manip)
{
    const StringPtr::element_type& str = *manip.value;

    for (auto cp = str.begin(), end = str.end(); cp != end; ++cp)
        if (*cp == L'\\' && cp + 1 < end)
            switch (*(++cp)) {
            case L'a':
                os << L'\a';
                break;
            case L'b':
                os << L'\b';
                break;
            case L'n':
                os << L'\n';
                break;
            case L'r':
                os << L'\r';
                break;
            case L't':
                os << L'\t';
                break;
            default:
                os << *cp;
            }
        else
            os << *cp;
    return os;
}

static std::wostream& operator<<(std::wostream& os, const VectorPtr& vptr)
{
    if (vptr->size()) {
        os << L"#(" << vptr->front();
        for (auto ip = vptr->begin() + 1, ie = vptr->end(); ip != ie; ++ip)
            os << L' ' << *ip;
        return os << L')';
    } else
        return os << L"#()";
}

std::wostream& operator<<(std::wostream& os, Intern opcode)
{
    switch (opcode) {
    case Intern::_or:
        return os << L"or";
    case Intern::_and:
        return os << L"and";
    case Intern::_if:
        return os << L"if";
    case Intern::_cond:
        return os << L"cond";
    case Intern::_else:
        return os << L"else";
    case Intern::_arrow:
        return os << L"=>";
    case Intern::_when:
        return os << L"when";
    case Intern::_unless:
        return os << L"unless";
    case Intern::_define:
        return os << L"define";
    case Intern::_setb:
        return os << L"set!";
    case Intern::_begin:
        return os << L"begin";
    case Intern::_lambda:
        return os << L"lambda";
    case Intern::_macro:
        return os << L"define-macro";
    case Intern::_apply:
        return os << L"apply";
    case Intern::_quote:
        return os << L"quote";
    case Intern::_quasiquote:
        return os << L"quasiquote";
    case Intern::_unquote:
        return os << L"unquote";
    case Intern::_unquotesplice:
        return os << L"unquote-splicing";
    default:
        return os << L"#<primop>";
    }
}

static std::wostream& operator<<(std::wostream& os, const Procedure& proc)
{
    return proc.is_macro() ? os << L"#<macro>" : os << L"#<clojure>";
}

/**
 * Output stream operator for Cell type arguments.
 */
std::wostream& operator<<(std::wostream& os, const Cell& cell)
{
    // clang-format off
    overloads stream{
        [&os](None)                   -> std::wostream& { return os << L"#<none>"; },
        [&os](Nil)                    -> std::wostream& { return os << L"()"; },
        [&os](Bool arg)               -> std::wostream& { return os << (arg ? L"#t" : L"#f"); },
        [&os](Char arg)               -> std::wostream& { return os << L"#\\" << arg; },
        [&os](const StringPtr& arg)   -> std::wostream& { return os << L'"' << *arg << L'"';},
        [&os](const RegexPtr&)        -> std::wostream& { return os << L"#<regex>"; },
        [&os](const SymenvPtr& arg)   -> std::wostream& { return os << L"#<symenv " << arg.get() << '>'; },
        [&os](const FunctionPtr& arg) -> std::wostream& { return os << L"#<function " << arg->name() << '>'; },
        [&os](const PortPtr&)         -> std::wostream& { return os << L"#<port>"; },
        [&os](auto& arg)              -> std::wostream& { return os << arg; }
    }; // clang-format on

    return std::visit(std::move(stream), static_cast<const Cell::base_type&>(cell));
}

/**
 * Overloaded output stream operator for Cell types as scheme (display <expr>) function
 * representation.
 */
std::wostream& operator<<(std::wostream& os, DisplayManip<Cell> manip)
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
