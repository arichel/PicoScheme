/********************************************************************************/ /**
 * @file pico.cpp
 *
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

int main()
{
    //    repl();
    //    return 0;

    try {

        Cell s0 = str("hallo");

        Cell s1 = s0;

        return 0;

        SymenvPtr e = senv();
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
