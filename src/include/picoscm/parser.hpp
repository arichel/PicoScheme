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

public:
    Parser(Scheme& scm)
        : scm(scm)
    {
    }

    Cell read(std::istream& in);

    static Cell strnum(const std::string& mkstr);

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
        Regex, // #re/12343434/

        Eof,
        Error
    };

    Cell parse_list(std::istream& in);
    Cell parse_vector(std::istream& in);
    Token get_token(std::istream& in);

    static bool is_alpha(int c);
    static bool is_special(int c);
    static bool is_digit(const std::string&, size_t n = 0);

    static Token lex_number(const std::string&, Number&);
    static Token lex_string(std::string&, std::istream& in);
    static Token lex_regex(std::string&, std::istream& in);
    static Token lex_symbol(const std::string&);
    static Token lex_unquote(const std::string&, std::istream& in);
    static Token lex_char(const std::string&, Char& c, std::istream& in);

    Token lex_special(std::string&, std::istream& in);
    Token skip_comment(std::istream& in) const;

    Token put_back = Token::None;
    std::string strtok;
    Number numtok;
    Char chrtok;

    Scheme& scm;

    const Symbol s_quote = scm.mksym("quote"), s_quasiquote = scm.mksym("quasiquote"),
                 s_unquote = scm.mksym("unquote"), s_unquotesplice = scm.mksym("unquote-splicing");
};

} // namespace pscm
#endif // PARSER_HPP
