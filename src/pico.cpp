/********************************************************************************/ /**
 * @file pico.hpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#include <assert.h>
#include <iostream>
#include <memory>

#include "cell.hpp"
#include "eval.hpp"
#include "parser.hpp"
#include "symbol.hpp"
#include "types.hpp"

using namespace std;
using namespace pscm;

void repl()
{
    Symenv env = senv();
    Parser reader;
    Cell expr;

start:
    try {
        for (;;) {
            expr = reader.parse(std::cin);
            expr = eval(env, expr);
            std::cout << "store=" << store_size() << " : " << expr << std::endl;
        }
    } catch (std::bad_variant_access& e) {
        cout << e.what() << ": " << expr << endl;

    } catch (std::out_of_range& e) {
        cout << e.what() << ": " << expr << endl;

    } catch (std::invalid_argument& e) {
        cout << e.what() << ": " << expr << endl;
    }
    goto start;
}

int main()
{
    repl();
    return 0;

    try {

        Cell cell = 'a';

        Char& c = pscm::get<Char>(cell);

        pscm::get<Char>(cell) = 'b';

        cout << "char: " << cell << endl;

        cout << reinterpret_cast<size_t>(&get<Char>(cell)) << "  " << reinterpret_cast<size_t>(&c) << endl;

        return 0;

        Symenv e = senv();

        std::string str = {
            "(call-with-input-file \"test.txt\" "
            "   (lambda (port) port))"
            ";       (display (read-line port))"
            ";       (display (read-line port))))"
        };

        str = "(apply (lambda (x y ) (* x y)) 2 100 ())";

        std::istringstream is{ str };

        Parser parser;
        Cell expr = parser.parse(is);

        cout << expr << " ---> ";
        Cell proc = eval(e, expr);
        cout << proc << endl;

    } catch (std::bad_variant_access& e) {
        cout << e.what() << endl;

    } catch (std::invalid_argument& e) {
        cout << e.what() << endl;
    }
    return 0;
}
