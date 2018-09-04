/*********************************************************************************/ /**
 * @file stream.cpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#include <cstring>

#include "scheme.hpp"
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

static std::ostream& operator<<(std::ostream& os, const Symbol& sym)
{
    using string_type = Symbol::value_type;
    const string_type& name = sym.value();

    if (name.find_first_of(' ') != string_type::npos)
        return os << '|' << name << '|';
    else
        return os << name;
}

static std::ostream& operator<<(std::ostream& os, const StringPtr& sptr)
{
    return os << '"' << *sptr << '"';
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
 * @brief Output stream operator for Cell type arguments.
 */
std::ostream& operator<<(std::ostream& os, const Cell& cell)
{
    overloads fun{
        [&os](None) { os << "#<none>"; },
        [&os](Nil) { os << "()"; },
        [&os](Bool arg) { os << (arg ? "#t" : "#f"); },
        [&os](Char arg) { os << "#\\" << arg; },
        [&os](const SymenvPtr& arg) { os << "#<symenv " << arg.get() << '>'; },
        [&os](const FunctionPtr& arg) { os << "#<function " << arg->name() << '>'; },
        [&os](const Port&) { os << "#<port>"; },
        [&os](auto& arg) { os << arg; }
    };
    std::visit(std::move(fun), static_cast<const Cell::base_type&>(cell));
    return os;
}

std::ostream& operator<<(std::ostream& os, const DisplayManip<Cell>& manip)
{
    return std::visit([&os, &manip](auto& val) -> std::ostream& {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, None>)
            return os;
        else if constexpr (std::is_same_v<T, Char>)
            return os << val;
        else if constexpr (std::is_same_v<T, StringPtr>)
            return os << display(val);
        else
            return os << manip.value;
    },
        static_cast<const Cell::base_type&>(manip.value));
}

Port::Port()
    : pstream{ std::make_shared<stream_variant>(std::in_place_type_t<std::iostream>(),
          std::cout.rdbuf()) }
    , mode{ std::ios_base::out }
{
    is_open() || (throw std::invalid_argument("could open standard port"), 0);
}

bool Port::is_strport() const noexcept { return is_type<std::stringstream>(); }

bool Port::is_fileport() const noexcept { return is_type<std::fstream>(); }

bool Port::is_input() const noexcept { return mode & std::ios_base::in; }

bool Port::is_output() const noexcept { return mode & std::ios_base::out; }

bool Port::is_binary() const noexcept { return mode & std::ios_base::binary; }

bool Port::is_open() const noexcept
{
    return std::visit([](auto& os) {
        using S = std::decay_t<decltype(os)>;

        if constexpr (std::is_same_v<S, std::ofstream>)
            return os.is_open();
        else
            return os.good();
    },
        *pstream);
}

void Port::close() const
{
    std::visit([](auto& os) {
        using S = std::decay_t<decltype(os)>;
        os.flush();
        os.clear();
        if constexpr (std::is_same_v<S, std::fstream>) {
            if (os.is_open())
                os.close();
        } else
            os.setstate(std::ios_base::eofbit);
    },
        *pstream);
}

bool Port::open(const std::string& path, std::ios_base::openmode mode)
{
    close();
    this->mode = mode;
    *pstream = std::fstream{ path, mode };
    return is_open();
}

bool Port::open_str(const std::string& str, std::ios_base::openmode mode)
{
    close();
    this->mode = mode;
    *pstream = std::stringstream{ str, mode };
    return is_open();
}

std::string Port::str() const
{
    return std::get<std::stringstream>(*pstream).str();
}

std::iostream& Port::stream()
{
    return std::visit([](auto& os) -> std::iostream& {
        return os;
    },
        *pstream);
}

bool Port::operator!=(const Port& stream) const noexcept
{
    return pstream.get() != stream.pstream.get();
}

bool Port::operator==(const Port& stream) const noexcept
{
    return !(*this != stream);
}
} // namespace pscm
