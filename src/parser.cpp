/*********************************************************************************/ /**
 * @file parser.cpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#include <algorithm>
#include <cstring>

#include "cell.hpp"
#include "parser.hpp"

namespace pscm {

using std::cout;
using std::endl;

/**
 * @brief Lexical analyse the argument string for an integer, a floating point or complex number.
 *
 * @param str  String to lexical analyse.
 * @param num  Uppon success, return the converted number.
 */
Parser::Token Parser::lex_number(const std::string& str, Number& num)
{
    if (str.empty())
        return Token::Error;

    bool is_flo = false, is_cpx = false;

    num = Int{ 0 };
    Complex z = { 0, 1 };

    auto ic = str.begin();
    size_t pos = 0, ip = 0;

    // Positive or negative imaginary number: +i or -i
    if (strchr("+-", *ic) && strchr("iI", *(ic + 1))) {
        num = *ic != '-' ? z : -z;
        return Token::Number;
    }

    // Sign character of floating point:
    if (strchr("+-.", *ic)) {
        is_flo = *ic == '.';
        ++ic;
        ++ip;
    }

    if (isdigit(*ic)) {

        while (++ic != str.end()) {
            ++ip;

            if (isdigit(*ic))
                continue;

            else if (strchr(".eE", *ic))
                is_flo = true;

            else if (strchr("+-", *ic)) {

                if (!strchr("eE", *(ic - 1))) {
                    is_cpx = true;
                    z.real(std::stod(str.substr(0, pos = ip)));

                    if (*ic != '+')
                        z.imag(-1);
                }
            } else if (strchr("iI", *ic) && &(*ic) == &str.back()) {
                is_cpx = true;

                if (isdigit(str.at(pos)) || pos + 2 < str.size())
                    z.imag(z.imag() >= 0 ? std::stod(str.substr(pos, str.size()))
                                         : -std::stod(str.substr(pos, str.size())));
            } else
                return Token::Error;
        }
        if (is_cpx)
            num = z;
        else if (is_flo)
            num = Number{ std::stod(str) };
        else {
            try {
                num = std::stol(str);
            } catch (std::out_of_range& e) {
                num = Number{ std::stod(str) };
            }
        }
        return Token::Number;
    }
    return Token::Error;
}

Cell Parser::strnum(const std::string& str)
{
    Number num;
    Token tok;

    if (!str.compare(0, 2, "#i"s))
        tok = lex_number(str.substr(2), num);

    else if (!str.compare(0, 2, "#e"s)) {
        tok = lex_number(str.substr(2), num);
        if (tok == Token::Number)
            num = trunc(num);
    } else
        tok = lex_number(str, num);

    if (tok == Token::Error)
        return false;

    return num;
}

/**
 * @brief Read characters from input stream into argument string.
 */
Parser::Token Parser::lex_string(std::string& str, std::istream& in)
{
    str.clear();

    while (in) {
        int c = in.get();

        switch (c) {

        case '"':
            return Token::String;

        case '\\':
            str.push_back('\\');
            c = in.get();
            [[fallthrough]];

        default:
            if (isprint(c))
                str.push_back(static_cast<Char>(c));
            else
                return Token::Error;
        }
    }
    return Token::Error;
}

/**
 * @brief Lexical analyse the argument string for valid scheme
 *        symbol characters.
 */
Parser::Token Parser::lex_symbol(const std::string& str)
{
    if (str.empty() || !is_alpha(str.front()))
        return Token::Error;

    for (auto c : str)
        if (!is_alpha(c) && !isdigit(c))
            return Token::Error;

    return Token::Symbol;
}

Parser::Token Parser::lex_char(const std::string& str, Char& c, std::istream& in)
{
    constexpr struct {
        const char* name;
        char c;
    } stab[]{
        { "#\\alarm", '\a' },
        { "#\\backspace", '\b' },
        { "#\\delete", '\0' },
        { "#\\escape", '\0' },
        { "#\\newline", '\n' },
        { "#\\null", '\0' },
        { "#\\return", '\r' },
        { "#\\space", ' ' },
        { "#\\tab", '\t' },

    };
    constexpr size_t ntab = sizeof(stab) / sizeof(*stab);

    if (str.size() == 2 && (std::isspace(in.peek()) || is_special(in.peek()))) {
        c = in.get();
        return Token::Char;
    }
    if (str.size() == 3) {
        c = str[2];
        return Token::Char;

    } else if (str.size() > 3 && str[2] == 'x') {
        std::string s{ str.substr(1) };
        s[0] = '0';
        c = static_cast<Char>(stoi(s));
        return Token::Char;
    } else {
        std::string name;
        std::transform(str.begin(), str.end(), std::back_inserter(name), ::tolower);

        for (size_t i = 0; i < ntab; ++i)
            if (stab[i].name == name) {
                c = stab[i].c;
                return Token::Char;
            }
    }
    return Token::Error;
}

/**
 * @brief Lexical analyse a special scheme symbol.
 */
Parser::Token Parser::lex_special(const std::string& str, std::istream& in)
{
    if (str == "#")
        return Token::Vector;

    Token tok;

    switch (str.at(1)) {
    case 't':
        if (str == "#t" || str == "#true")
            return Token::True;

    case 'f':
        if (str == "#f" || str == "#false")
            return Token::False;

    case '\\':
        return lex_char(str, chrtok, in);

    case 'e':
        tok = lex_number(str.substr(2), numtok);
        if (tok == Token::Number)
            numtok = trunc(numtok);
        return tok;

    case 'i':
        return lex_number(str.substr(2), numtok);

    default:
        return Token::Error;
    }
}

Parser::Token Parser::lex_unquote(const std::string& str, std::istream& in)
{
    if (str.size() != 1)
        return Token::Error;

    if (in.peek() == '@') {
        in.get();
        return Token::UnquoteSplice;
    }
    return Token::Unquote;
}

/**
 * @brief Skip a comment line.
 */
Parser::Token Parser::skip_comment(std::istream& in) const
{
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return Token::Comment;
}

/**
 * @brief Predicate returns true if the first n characters are digits,
 *        floating point exponent characters (e,E) or imaginary characters (i,I).
 *
 * @param str String to test.
 * @param n   Unless zero, test the first n characters or the whole string
 *            otherwise.
 */
bool Parser::is_digit(const std::string& str, size_t n)
{
    n = n ? std::min(n, str.size()) : str.size();

    bool has_digit = isdigit(str.front()),
         has_sign = strchr("+-", str.front()),
         has_imag = false;

    if (str.empty() || (str.size() == 1 && !has_digit))
        return false;
    else
        for (auto ic = str.begin(), ie = ic + n; ic != ie; ++ic) {
            if (!has_digit)
                has_digit = isdigit(*ic);

            if (!has_imag)
                has_imag = strchr("iI", *ic);

            if (!isdigit(*ic) && !strchr("+-.iIeE", *ic))
                return false;
        }
    return has_digit || (str.size() <= 2 && (has_sign || has_imag));
}

/**
 * @brief Predicate returns true if the argument character is a special
 *        scheme character, starting a new expression, string or comment.
 */
bool Parser::is_special(int c)
{
    return strchr("()\"'`,;", c);
}

/**
 * @brief Predicate return true if argument character is an allowed scheme character.
 */
bool Parser::is_alpha(int c)
{
    return isalpha(c) || strchr("._:?!+-*/<>=^@$%&~", c);
}

Parser::Token Parser::get_token(std::istream& in)
{
    // Check if there is a put-back token available:
    if (put_back != Token::None) {
        Token tok = put_back;
        put_back = Token::None;
        return tok;
    }

    // Ignore all leading whitespaces:
    char c;
    while (in.get(c) && isspace(c))
        ;

    if (!in.good())
        return in.eof() ? Token::Eof : Token::Error;

    strtok.clear();
    strtok.push_back(static_cast<Char>(c));

    // Read chars until a trailing whitespace, a special scheme character or EOF is reached:
    if (!is_special(c)) {
        while (in.get(c) && !isspace(c) && !is_special(c))
            strtok.push_back(static_cast<Char>(c));

        if (!in.good() && !in.eof())
            return Token::Error;

        in.unget();
    }
    // Lexical analyse token string according to the first character:
    switch (c = strtok.front()) {

    case '(':
        return Token::OBrace;

    case ')':
        return Token::CBrace;

    case '\'':
        return Token::Quote;

    case '`':
        return Token::QuasiQuote;

    case ',':
        return lex_unquote(strtok, in);

    case ';':
        return skip_comment(in);

    case '#':
        return lex_special(strtok, in);

    case '"':
        return lex_string(strtok, in);

    case '.':
        if (strtok.size() == 1)
            return Token::Dot;
        [[fallthrough]];
    default:
        if (is_digit(strtok, 2))
            return lex_number(strtok, numtok);
        else
            return lex_symbol(strtok);
    }
}

Cell Parser::read(std::istream& in)
{
    for (;;)
        switch (get_token(in)) {

        case Token::Comment:
            break;

        case Token::True:
            return true;

        case Token::False:
            return false;

        case Token::Char:
            return chrtok;

        case Token::Quote:
            return list(s_quote, read(in));

        case Token::QuasiQuote:
            return list(s_quasiquote, read(in));

        case Token::Unquote:
            return list(s_unquote, read(in));

        case Token::UnquoteSplice:
            return list(s_unquotesplice, read(in));

        case Token::Number:
            return numtok;

        case Token::String:
            return str(strtok.c_str());

        case Token::Symbol:
            return sym(strtok.c_str());

        case Token::Vector:
            return parse_vector(in);

        case Token::OBrace:
            return parse_list(in);

        case Token::Eof:
            return none;

        case Token::Error:
        default:
            throw std::invalid_argument("parse error");
        }
}

Cell Parser::parse_vector(std::istream& in)
{
    VectorPtr vptr = vec(0, none);
    Token tok = get_token(in);

    if (tok == Token::OBrace)
        while (in.good()) {
            switch (tok = get_token(in)) {
            case Token::Comment:
                break;
            case Token::CBrace:
                return vptr;
            case Token::Eof:
            case Token::Error:
                goto error;
            default:
                put_back = tok;
                vptr->push_back(read(in));
            }
        }
error:
    throw std::invalid_argument("error while reading vector");
}

Cell Parser::parse_list(std::istream& in)
{
    Cell list = nil, tail = nil;
    Cell cell;
    Token tok;

    while (in.good()) {
        switch (tok = get_token(in)) {
        case Token::Comment:
            break;
        case Token::CBrace:
            return list;
        case Token::Dot:
            cell = read(in);
            tok = get_token(in);

            if (tok == Token::CBrace) {
                set_cdr(tail, cell);
                return list;
            }
            goto error;

        case Token::Eof:
        case Token::Error:
            goto error;

        default:
            put_back = tok;
            cell = read(in);

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
} // namespace pscm
