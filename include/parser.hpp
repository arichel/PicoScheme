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

//std::ostream& operator<<(std::ostream& os, const Cell& cell);

class Parser {

public:
    Cell parse(std::istream& in);

private:
    enum class Token : int {
        None,
        OBrace, // (
        CBrace, // )
        Comment, // \;[^\n\r]*

        Dot,
        Quote,
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

    Token lex_symbol(const std::string& str);
    Token lex_char(const std::string& str, Char& c);
    Token lex_special(const std::string& str);
    Token lex_number(const std::string& str, Number& num);
    Token lex_string(std::string& str, std::istream& in);
    Token skip_comment(std::istream& in);

    bool is_alpha(int c);
    bool is_special(int c);
    bool is_digit(const std::string& str, size_t n = 0);

    Token put_back = Token::None;
    std::string strtok;
    Number numtok;
    Char chrtok;
};

}; // namespace pscm
#endif // PARSER_HPP
