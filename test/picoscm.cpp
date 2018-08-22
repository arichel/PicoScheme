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
#include <picoscm/primop.hpp>
#include <picoscm/symbol.hpp>
#include <picoscm/types.hpp>

using namespace std;
using namespace pscm;

int main(int argn, char* argv[])
{
    using pscm::Intern, pscm::Cell, pscm::list, pscm::sym, pscm::num, pscm::str, pscm::fun, pscm::nil;

    fun(sym("greet"), [cntr = 0](auto senv, auto args) mutable -> Cell {
        return list(str("hello world"), num(cntr++));
    });

    pscm::load("picoscmrc.scm");
    pscm::repl();
    return 0;

    try {
        pscm::SymenvPtr e = pscm::senv();

        //        Cell expr = pscm::list(Intern::op_map,
        //            pscm::list(Intern::_lambda, pscm::list(sym("x")), sym("x")),
        //            pscm::list(Intern::_quote, pscm::list(num(1), num(2), num(3))));

        //        Cell expr = pscm::list(Intern::_apply,
        //            pscm::list(Intern::_lambda, pscm::list(sym("x")), sym("x")),
        //            pscm::list(Intern::_quote, pscm::list(num(1))));

        pscm::Parser parser;
        std::istringstream stream("(define (h)"
                                  ";\n"
                                  ")");

        Cell expr = parser.read(stream);

        cout << expr;
        Cell proc = pscm::eval(e, expr);
        cout << proc << endl;

    } catch (std::bad_variant_access& e) {
        cout << e.what() << endl;

    } catch (std::invalid_argument& e) {
        cout << e.what() << endl;
    }
    return 0;
}
