/*********************************************************************************/ /**
 * @file stream.hpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#ifndef STREAM_HPP
#define STREAM_HPP

#include <istream>
#include <ostream>

#include "cell.hpp"

namespace pscm {

std::ostream& operator<<(std::ostream& os, const Cell& cell);

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

        Eof,
        Error
    };

    Cell parse_list(std::istream& in);
    Cell parse_list_tail(std::istream& in);

    Token get_token(std::istream& in);

    Token lex_number(std::istream& in);
    Token lex_string(std::istream& in);
    Token lex_symbol(std::istream& in);
    Token lex_special(std::istream& in);

    bool is_alpha(int c);
    bool is_special(int c);
    bool is_digit(std::istream& in, int c);

    Token put_back = Token::None;
    std::string strtok;
    Number numtok;
};

}; // namespace pscm
#endif // STREAM_HPP
