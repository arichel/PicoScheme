/*********************************************************************************/ /**
 * @file parser.cpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#include <algorithm>
#include <cctype>
#include <cstring>

#include "parser.hpp"

namespace pscm {

using namespace std::string_literals;

double str2double(const wchar_t* str, std::size_t* pos = nullptr)
{
    errno = 0;
    wchar_t* end;
    double x = std::wcstod(str, &end);

    if (errno == ERANGE) { // Ignore it for denormals
        if (!(x != 0 && x > -HUGE_VAL && x < HUGE_VAL))
            throw std::out_of_range("strtod: ERANGE");

    } else if (errno)
        throw std::invalid_argument("strtod failed");

    if (pos)
        *pos = end - str;

    return x;
}

/**
 * Lexical analyse the argument string for an integer, a floating point or complex number.
 *
 * @param str  String to analyse.
 * @param num  Uppon success, return the converted number.
 */
Parser::Token Parser::lex_number(const String& str, Number& num)
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

    if (iswdigit(*ic)) {

        while (++ic != str.end()) {
            ++ip;

            if (iswdigit(*ic))
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

                if (iswdigit(str.at(pos)) || pos + 2 < str.size())
                    z.imag(z.imag() >= 0 ? std::stod(str.substr(pos, str.size()))
                                         : -std::stod(str.substr(pos, str.size())));
            } else
                return Token::Error;
        }
        if (is_cpx)
            num = z;

        else if (is_flo) {
            num = str2double(str.c_str());

            //   num = Number{ std::stod(str) };
        } else {
            try {
                num = std::stol(str);
            } catch (std::out_of_range&) {
                num = Number{ std::stod(str) };
            }
        }
        return Token::Number;
    }
    return Token::Error;
}

Cell Parser::strnum(const String& str)
{
    Number num;
    Token tok;

    if (!str.compare(0, 2, L"#i"))
        tok = lex_number(str.substr(2), num);

    else if (!str.compare(0, 2, L"#e")) {
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
Parser::Token Parser::lex_string(String& str, istream_type& in)
{
    str.clear();
    Char c;

    while (in) {
        in >> std::noskipws >> c;
        switch (c) {

        case '"':
            return Token::String;

        case '\\':
            str.push_back('\\');
            in >> c;
            [[fallthrough]];

        default:
            if (iswprint(c))
                str.push_back(static_cast<Char>(c));
            else
                return Token::Error;
        }
    }
    return Token::Error;
}

Parser::Token Parser::lex_regex(String& str, istream_type& in)
{
    if (str != L"#re" || in.get() != '\"')
        return Token::Error;

    if (lex_string(str, in) != Token::String)
        return Token::Error;

    return Token::Regex;
}

/**
     * @brief Lexical analyse the argument string for valid scheme
     *        symbol characters.
     */
Parser::Token Parser::lex_symbol(const String& str)
{
    if (str.empty() || !is_alpha(str.front()))
        return Token::Error;

    for (auto c : str)
        if (!is_alpha(c) && !iswdigit(c))
            return Token::Error;

    return Token::Symbol;
}

Parser::Token Parser::lex_char(const String& str, Char& c, istream_type& in)
{
    constexpr struct {
        const Char* name;
        Int c;
    } stab[]{
        // clang-format off
            { L"#\\eof",        EOF},
            { L"#\\alarm",     '\a'},
            { L"#\\backspace", '\b'},
            { L"#\\delete",    '\0'},
            { L"#\\escape",    '\0'},
            { L"#\\newline",   '\n'},
            { L"#\\null",      '\0'},
            { L"#\\return",    '\r'},
            { L"#\\space",     ' ' },
            { L"#\\tab",       '\t'},
            { L"#\\ae",        L'ä'},  { L"#\\AE",        L'Ä'},
            { L"#\\ue",        L'ü'},  { L"#\\UE",        L'Ü'},
            { L"#\\oe",        L'ö'},  { L"#\\OE",        L'Ö'},
            { L"#\\ss",        L'ß'},
            { L"#\\_0",        L'₀'},  { L"#\\^0",        L'⁰'},
            { L"#\\_1",        L'₁'},  { L"#\\^1",        L'¹'},
            { L"#\\_2",        L'₂'},  { L"#\\^2",        L'²'},
            { L"#\\_3",        L'₃'},  { L"#\\^3",        L'³'},
            { L"#\\_4",        L'₄'},  { L"#\\^4",        L'⁴'},
            { L"#\\_5",        L'₅'},  { L"#\\^5",        L'⁵'},
            { L"#\\_6",        L'₆'},  { L"#\\^6",        L'⁶'},
            { L"#\\_7",        L'₇'},  { L"#\\^7",        L'⁷'},
            { L"#\\_8",        L'₈'},  { L"#\\^8",        L'⁸'},
            { L"#\\_9",        L'₉'},  { L"#\\^9",        L'⁹'},
            { L"#\\alpha",     L'α'},
            { L"#\\beta",      L'β'},
            { L"#\\gamma",     L'γ'},  { L"#\\Gamma",     L'Γ'},
            { L"#\\delta",     L'δ'},  { L"#\\Delta",     L'Δ'},
            { L"#\\epsilon",   L'ε'},
            { L"#\\zeta",      L'ζ'},
            { L"#\\eta",       L'η'},
            { L"#\\theta",     L'θ'},
            { L"#\\iota",      L'ι'},
            { L"#\\kappa",     L'κ'},
            { L"#\\lambda",    L'λ'},
            { L"#\\mu",        L'μ'},
            { L"#\\nu",        L'ν'},
            { L"#\\xi",        L'ξ'},  { L"#\\Xi",       L'Ξ'},
            { L"#\\omicron",   L'ο'},
            { L"#\\pi",        L'π'},  { L"#\\Pi",       L'Π'},
            { L"#\\rho",       L'ρ'},
            { L"#\\tau",       L'τ'},
            { L"#\\sigma",     L'σ'},  { L"#\\Sigma",    L'Σ'},
            { L"#\\upsilon",   L'υ'},
            { L"#\\phi",       L'φ'},  { L"#\\Phi",      L'Φ'},
            { L"#\\chi",       L'χ'},
            { L"#\\psi",       L'ψ'},  { L"#\\Psi",      L'Ψ'},
            { L"#\\omega",     L'ω'},  { L"#\\Omega",    L'Ω'},
            { L"#\\le",        L'≤'},
            { L"#\\ge",        L'≥'},
            { L"#\\sim",       L'∼'},
            { L"#\\simeq",     L'≃'},
            { L"#\\approx",    L'≈'},
            { L"#\\nabla",     L'∇'},
            { L"#\\nabla",     L'∇'},
            { L"#\\nabla",     L'∇'},
            { L"#\\sum",       L'∑'},
            { L"#\\prod",      L'∏'},
            { L"#\\int",       L'∫'},
            { L"#\\oint",      L'∮'},
            { L"#\\pm",        L'±'},
            { L"#\\div",       L'÷'},
            { L"#\\cdot",      L'·'},
            { L"#\\star",      L'⋆'},
            { L"#\\circ",      L'∘'},
            { L"#\\bullet",    L'•'},
            { L"#\\diamond",   L'◇'},
            { L"#\\lhd",       L'◁'},
            { L"#\\rhd",       L'▷'},
            { L"#\\trup",      L'△'},
            { L"#\\trdown",    L'▽'},
            { L"#\\times",     L'×'},
            { L"#\\otimes",    L'⊗'},
            { L"#\\in",        L'∈'},
            { L"#\\notin",     L'∉'},
            { L"#\\subset",    L'⊂'},
            { L"#\\subseteq",  L'⊆'},
            { L"#\\in",        L'∈'},
            { L"#\\infty",     L'∞'},
        }; // clang-format on
    constexpr size_t ntab = sizeof(stab) / sizeof(*stab);

    if (str.size() == 2 && (std::isspace(in.peek()) || is_special(in.peek()))) {
        c = in.get();
        return Token::Char;
    }
    if (str.size() == 3) {
        c = str[2];
        return Token::Char;

    } else if (str.size() > 3 && str[2] == L'x') {
        String s{ str.substr(1) };
        s[0] = L'0';
        c = static_cast<Char>(stoi(s));
        return Token::Char;
    } else {
        String name;
        transform(str.begin(), str.end(), back_inserter(name), ::tolower);

        for (size_t i = 0; i < ntab; ++i)
            if (stab[i].name == name) {
                c = static_cast<Char>(stab[i].c);
                return Token::Char;
            }
    }
    return Token::Error;
}

//! Lexical analyse a special scheme symbol.
Parser::Token Parser::lex_special(String& str, istream_type& in)
{
    if (str == L"#")
        return Token::Vector;

    Token tok;

    switch (str.at(1)) {
    case 't':
        if (str == L"#t" || str == L"#true")
            return Token::True;
        [[fallthrough]];

    case 'f':
        if (str == L"#f" || str == L"#false")
            return Token::False;
        [[fallthrough]];

    case '\\':
        return lex_char(str, chrtok, in);

    case 'e':
        tok = lex_number(str.substr(2), numtok);
        if (tok == Token::Number)
            numtok = trunc(numtok);
        return tok;

    case 'i':
        return lex_number(str.substr(2), numtok);

    case 'r':
        return lex_regex(str, in);

    default:
        return Token::Error;
    }
}

//! Scan if str contains an scheme unquote "," or unquote-splicing ",@"
Parser::Token Parser::lex_unquote(const String& str, istream_type& in)
{
    if (str.size() != 1)
        return Token::Error;

    if (in.peek() == '@') {
        in.get();
        return Token::UnquoteSplice;
    }
    return Token::Unquote;
}

//! Skip a comment line.
Parser::Token Parser::skip_comment(istream_type& in) const
{
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return Token::Comment;
}

/**
     * Predicate returns true if the first n characters in str could form a
     * a number.
     *
     * Check if the first n caracters are digits, contains a floating point,
     * an exponent characters (e,E) or an imaginary unit characters (i,I).
     *
     * @param str String to test.
     * @param n   Unless zero, test the first n characters or the whole string
     *            otherwise.
     */
bool Parser::is_digit(const String& str, size_t n)
{
    n = n ? std::min(n, str.size()) : str.size();

    bool has_digit = iswdigit(str.front()),
         has_sign = strchr("+-", str.front()),
         has_imag = false;

    if (str.empty() || (str.size() == 1 && !has_digit))
        return false;
    else
        for (auto ic = str.begin(), ie = ic + n; ic != ie; ++ic) {
            if (!has_digit)
                has_digit = iswdigit(*ic);

            if (!has_imag)
                has_imag = strchr("iI", *ic);

            if (!iswdigit(*ic) && !strchr("+-.iIeE", *ic))
                return false;
        }
    return has_digit || (str.size() <= 2 && (has_sign || has_imag));
}

//! Predicate returns true if the argument character is a special
//! scheme character, starting a new expression, string or comment.
bool Parser::is_special(int c) { return strchr("()\"'`,;", c); }

//! Predicate return true if argument character is an allowed scheme character.
bool Parser::is_alpha(int c)
{
    return iswgraph(c) && !iswdigit(c) && !is_special(c);
}

/**
     * Return the next token from the input stream.
     *
     * Depending on the token type, the token value is stored in member variable
     * strtok, numtok or chrtok. For invalid input an Error token is returned.
     */
Parser::Token Parser::get_token(istream_type& in)
{
    // Check if there is a put-back token available:
    if (put_back != Token::None) {
        Token tok = put_back;
        put_back = Token::None;
        return tok;
    }
    // Ignore all leading whitespaces:
    Char c;
    while (in >> c && iswspace(c))
        ;

    if (!in.good())
        return in.eof() ? Token::Eof : Token::Error;

    strtok.clear();
    strtok.push_back(static_cast<Char>(c));

    // Read chars until a trailing whitespace, a special scheme character or EOF is reached:
    if (!is_special(c)) {
        while (in >> std::noskipws >> c && !iswspace(c) && !is_special(c))
            strtok.push_back(static_cast<Char>(c));

        if (!in.good() && !in.eof())
            return Token::Error;

        //in.unget();
        in.putback(c);
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

Cell Parser::read(istream_type& in)
{
    in.clear();
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
            return scm.list(s_quote, read(in));

        case Token::QuasiQuote:
            return scm.list(s_quasiquote, read(in));

        case Token::Unquote:
            return scm.list(s_unquote, read(in));

        case Token::UnquoteSplice:
            return scm.list(s_unquotesplice, read(in));

        case Token::Number:
            return numtok;

        case Token::String:
            return str(strtok);

        case Token::Regex:
            return regex(strtok);

        case Token::Symbol:
            return scm.symbol(strtok);

        case Token::Vector:
            return parse_vector(in);

        case Token::OBrace:
            return parse_list(in);

        case Token::Eof:
            return static_cast<Char>(EOF);

        case Token::Error:
        default:
            throw parse_error("invalid token");
        }
}

//! Read a scheme vector from stream.
Cell Parser::parse_vector(istream_type& in)
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
    throw parse_error("error while reading vector");
}

//! Read a scheme list from stream.
Cell Parser::parse_list(istream_type& in)
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
            [[fallthrough]];
        case Token::Eof:
        case Token::Error:
            goto error;

        default:
            put_back = tok;
            cell = read(in);

            if (is_pair(tail)) {
                set_cdr(tail, scm.cons(cell, nil));
                tail = cdr(tail);
            } else {
                list = tail = scm.cons(cell, nil);
                scm.addenv(s_expr, list); // add list to env to prevent gc from deleting it.
            }
        }
    }
error:
    throw parse_error("error while reading list");
}
} // namespace pscm
