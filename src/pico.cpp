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
        Symenv e = senv();
        Cell expr;

        Cell lambda = pscm::list(Intern::_lambda, pscm::list(sym("x")), sym("x"));
        Cell clause = pscm::list(pscm::list(lambda, str("hallo paul")), Intern::_arrow, lambda);
        Cell apply = pscm::list(Intern::_apply, lambda, num(1), nil);
        Cell cond = pscm::list(Intern::_cond, clause);

        expr = cond;

        //        std::string str = {
        //            "(call-with-input-file \"test.txt\" "
        //            "   (lambda (port) port))"
        //            ";       (display (read-line port))"
        //            ";       (display (read-line port))))"
        //        };

        //        str = "(apply (lambda (x y ) (* x y)) 2 100 ())";

        //        std::istringstream is{ str };

        //        Parser parser;
        //        expr = parser.parse(is);

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
