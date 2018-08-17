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

#include "types.hpp"

namespace pscm {

struct Cell;

class Parser {

public:
    Cell read(std::istream& in);

    static Cell strnum(const std::string& str);

private:
    enum class Token : int {
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
        Char, //

        String, // "([^"]*)"
        Number, // (+|-)[0-9]+(\.[0-9]+)
        Symbol, // [a-zA-Z_%:+-][a-zA-Z_%:+-]*
        Vector,

        Eof,
        Error
    };

    Cell parse_list(std::istream& in);
    Cell parse_vector(std::istream& in);
    Token get_token(std::istream& in);

    static bool is_alpha(int c);
    static bool is_special(int c);
    static bool is_digit(const std::string& str, size_t n = 0);

    static Token lex_number(const std::string& str, Number& num);
    static Token lex_string(std::string& str, std::istream& in);
    static Token lex_symbol(const std::string& str);
    static Token lex_unquote(const std::string& str, std::istream& in);
    static Token lex_char(const std::string& str, Char& c, std::istream& in);

    Token lex_special(const std::string& str, std::istream& in);
    Token skip_comment(std::istream& in) const;

    Token put_back = Token::None;
    std::string strtok;
    Number numtok;
    Char chrtok;

    const Symbol s_quote = sym("quote"), s_quasiquote = sym("quasiquote"),
                 s_unquote = sym("unquote"), s_unquotesplice = sym("unquote-splicing");
};

} // namespace pscm
#endif // PARSER_HPP
