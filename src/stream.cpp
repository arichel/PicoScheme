/*********************************************************************************/ /**
 * @file stream.cpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#include <cstring>

#include "cell.hpp"
#include "stream.hpp"

namespace pscm {

using std::cout;
using std::endl;

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

Parser::Token Parser::lex_number(std::istream& in)
{
    strtok.clear();
    int c = in.get();

    if (strchr("+-.", c)) {
        strtok.push_back(c);
        c = in.get();
    }
    if (isdigit(c)) {
        strtok.push_back(c);

        while (in.good()) {
            if (isdigit(c = in.get()) || strchr(".eE", c))
                strtok.push_back(c);
            else {
                in.unget();
                break;
            }
        }
        return Token::Number;
    }
    in.unget();
    return Token::Error;
}

Parser::Token Parser::lex_string(std::istream& in)
{
    strtok.clear();

    while (in.good()) {
        int c = in.get();

        switch (c) {
        case '"':
            return Token::String;

        default:
            if (isprint(c))
                strtok.push_back(c);
            else
                return Token::Error;
        }
    }
    return Token::Error;
}

Parser::Token Parser::lex_symbol(std::istream& in)
{
    strtok.clear();
    int c = in.get();

    if (is_alpha(c)) {
        strtok.push_back(c);

        while (in.good()) {
            c = in.get();

            if (is_alpha(c) || isdigit(c))
                strtok.push_back(c);

            else {
                in.unget();
                return Token::Symbol;
            }
        }
    }
    return Token::Error;
}

Parser::Token Parser::lex_special(std::istream& in)
{
    int c = in.get();

    switch (c) {
    case 't':
        if (isspace(in.peek()) || is_special(in.peek()))
            return Token::True;

    case 'f':
        if (isspace(in.peek()) || is_special(in.peek()))
            return Token::False;

    default:
        return Token::Error;
    }
}

bool Parser::is_special(int c)
{
    return strchr("()\"',", c);
}

bool Parser::is_digit(std::istream& in, int c)
{
    return isdigit(c) || (strchr("+-.", c) && isdigit(in.peek()));
}

bool Parser::is_alpha(int c)
{
    return isalpha(c) || strchr("_?!+-*/:@", c);
}

Parser::Token Parser::get_token(std::istream& in)
{
    if (put_back != Token::None) {
        Token tok = Token::None;
        std::swap(tok, put_back);
        return tok;
    }
    while (in.good()) {
        int c = in.get();

        if (isspace(c))
            continue;

        switch (c) {

        case '(':
            return Token::OBrace;

        case ')':
            return Token::CBrace;

        case '.':
            if (!isdigit(in.peek()))
                return Token::Dot;

        case '\'':
            return Token::Quote;

        case ';':
            std::getline(in, strtok);
            return Token::Comment;

        case '#':
            return lex_special(in);

        case '"':
            return lex_string(in);

        case EOF:
            return Token::Eof;

        default:
            if (is_digit(in, c))
                return lex_number(in.unget());

            if (is_alpha(c))
                return lex_symbol(in.unget());

            return Token::Error;
        }
    }
    return Token::Error;
}

Cell Parser::parse(std::istream& in)
{
    Token tok;

    while (in.good()) {
        switch (tok = get_token(in)) {

        case Token::True:
            return true;

        case Token::False:
            return false;

        case Token::Quote:
            return list(Intern::_quote, parse(in));

        case Token::Number:
            return num(std::stol(strtok));

        case Token::String:
            return str(strtok.c_str());

        case Token::Symbol:
            return sym(strtok.c_str());

        case Token::OBrace:
            return parse_list(in);

        default:
            throw std::invalid_argument("parse error");
        }
    }
    return none;
}

Cell Parser::parse_list(std::istream& in)
{
    Cell list = nil, tail = nil;
    Cell cell;
    Token tok;

    while (in.good()) {
        switch (tok = get_token(in)) {

        case Token::Dot:
            cell = parse(in);
            tok = get_token(in);

            if (tok == Token::CBrace) {
                set_cdr(tail, cell);
                return list;
            }
            goto error;

        case Token::CBrace:
            return list;

        case Token::Error:
            goto error;

        default:
            put_back = tok;
            cell = parse(in);

            if (is_pair(tail)) {
                set_cdr(tail, cons(cell, nil));
                tail = cdr(tail);
            } else {
                list = cons(cell, nil);
                tail = list;
            }
        }
    }
error:
    throw std::invalid_argument("error parsing list");
}

}; // namespace pscm
