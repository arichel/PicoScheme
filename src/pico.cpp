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

    for (;;)
        try {

            for (;;) {
                expr = reader.parse(std::cin);
                expr = eval(env, expr);

                if (is_intern(expr) && get<Intern>(expr) == Intern::op_exit)
                    return;

                std::cout << expr << std::endl;
            }
        } catch (std::bad_variant_access& e) {
            std::cerr << e.what() << ": " << expr << endl;

        } catch (std::out_of_range& e) {
            std::cerr << e.what() << ": " << expr << endl;

        } catch (std::invalid_argument& e) {
            std::cerr << e.what() << ": " << expr << endl;
        }
}

int main()
{
    //    repl();
    //    return 0;

    try {
        Number x = 3, z = { 3, 0 };

        cout << x << " " << z << " : " << (z != x) << endl;
        return 0;

        Symenv e = senv();
        //        Cell list = cons(Intern::_quote, cons(pscm::list(num(1), num(2), num(3)), nil));
        //        Cell expr = pscm::list(Intern::op_listref, list, num(1.));

        std::istringstream stream("(list-ref '(0 1 2) 1.)");

        Parser parser;
        Cell expr = parser.parse(stream);

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
