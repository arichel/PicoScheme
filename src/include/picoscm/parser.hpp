/*********************************************************************************/ /**
 * @file stream.hpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#ifndef PARSER_HPP
#define PARSER_HPP

#include <istream>
#include <ostream>

#include "scheme.hpp"

namespace pscm {

class Parser {
    using istream_type = std::basic_istream<Char>;

public:
    Parser(Scheme& scm)
        : scm(scm)
    {
    }
    //! Read the next scheme expression from the argument input stream.
    Cell read(istream_type& in);

    //! Try to convert the argument string into a scheme number or
    //! return #false for an unsuccessful conversion.
    static Cell strnum(const String&);

private:
    enum class Token {
        None,
        OBrace, // (
        CBrace, // )
        Comment, // \;[^\n\r]*

        Dot,
        Quote,
        QuasiQuote,
        Unquote,
        UnquoteSplice,
        True,
        False,
        Char,

        String, // "([^"]*)"
        Number, // (+|-)[0-9]+(\.[0-9]+)
        Symbol, // [a-zA-Z_%:+-][a-zA-Z_%:+-]*
        Vector,
        Regex, // #re"12343434"

        Eof,
        Error
    };

    Cell parse_list(istream_type&);
    Cell parse_vector(istream_type&);
    Token get_token(istream_type&);

    static bool is_alpha(int c);
    static bool is_special(int c);
    static bool is_digit(const String&, size_t n = 0);

    static Token lex_number(const String&, Number&);
    static Token lex_string(String&, istream_type&);
    static Token lex_regex(String&, istream_type&);
    static Token lex_symbol(const String&);
    static Token lex_unquote(const String&, istream_type&);
    static Token lex_char(const String&, Char& c, istream_type&);

    Token lex_special(String&, istream_type& in);
    Token skip_comment(istream_type& in) const;

    Token put_back = Token::None;
    String strtok;
    Number numtok;
    Char chrtok;
    Scheme& scm;

    const Symbol s_quote = scm.symbol("quote"), s_quasiquote = scm.symbol("quasiquote"),
                 s_unquote = scm.symbol("unquote"), s_unquotesplice = scm.symbol("unquote-splicing"),
                 s_expr = scm.symbol();
};

struct parse_error : public std::exception {
    parse_error(const std::string& str)
        : reason{ str }
    {
    }
    const char* what() const noexcept override
    {
        return reason.c_str();
    }

private:
    std::string reason;
};

} // namespace pscm
#endif // PARSER_HPP
