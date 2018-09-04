/********************************************************************************/ /**
 * @file pico.cpp
 *
 *************************************************************************************/
#include <assert.h>
#include <iostream>
#include <memory>

#include <picoscm/parser.hpp>
#include <picoscm/scheme.hpp>

using namespace std;
using namespace pscm;

int main(int argn, char* argv[])
{
    using pscm::Intern, pscm::Cell, pscm::mknum, pscm::mkstr, pscm::nil;

    pscm::Scheme scm;

    scm.mkfun("greet", [cntr = 0](Scheme& scm, const SymenvPtr& senv, const std::vector<Cell>& args) mutable -> Cell {
        return scm.list(mkstr("hello world"), mknum(cntr++));
    });

    scm.load("picoscmrc.scm");
    scm.repl();
    return 0;

    try {

        pscm::SymenvPtr e = scm.mkenv();

        //        Cell expr = pscm::list(Intern::op_map,
        //            pscm::list(Intern::_lambda, pscm::list(sym("x")), sym("x")),
        //            pscm::list(Intern::_quote, pscm::list(num(1), num(2), num(3))));

        //        Cell expr = pscm::list(Intern::_apply,
        //            pscm::list(Intern::_lambda, pscm::list(sym("x")), sym("x")),
        //            pscm::list(Intern::_quote, pscm::list(num(1))));

        pscm::Parser parser(scm);
        std::istringstream stream("(define (h)"
                                  ";\n"
                                  ")");

        Cell expr = parser.read(stream);

        cout << expr;
        Cell proc = scm.eval(e, expr);
        cout << proc << endl;

    } catch (std::bad_variant_access& e) {
        cout << e.what() << endl;

    } catch (std::invalid_argument& e) {
        cout << e.what() << endl;
    }
    return 0;
}
