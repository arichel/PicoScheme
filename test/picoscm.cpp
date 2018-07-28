/********************************************************************************/ /**
 * @file pico.cpp
 *
 *************************************************************************************/
#include <assert.h>
#include <iostream>
#include <memory>

#include <picoscm/cell.hpp>
#include <picoscm/eval.hpp>
#include <picoscm/parser.hpp>
#include <picoscm/symbol.hpp>
#include <picoscm/types.hpp>

using namespace std;
using namespace pscm;

int main()
{
    repl();
    return 0;

    try {
        SymenvPtr e = senv();

        Cell expr = pscm::list(Intern::op_map,
            pscm::list(Intern::_lambda, pscm::list(sym("x")), sym("x")),
            pscm::list(Intern::_quote, pscm::list(num(1), num(2), num(3))));

        //        Cell expr = pscm::list(Intern::_apply,
        //            pscm::list(Intern::_lambda, pscm::list(sym("x")), sym("x")),
        //            pscm::list(Intern::_quote, pscm::list(num(1))));

        //        Parser parser;
        //        std::istringstream stream("',@3");
        //        Cell expr = parser.parse(stream);

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