/********************************************************************************/ /**
 * @file pico.cpp
 *
 *************************************************************************************/
#include <assert.h>
#include <iostream>
#include <memory>
#include <regex>

#include <picoscm/parser.hpp>
#include <picoscm/scheme.hpp>

using namespace std;
using namespace pscm;

int main(int argn, char* argv[])
{

    //    std::regex rx("hallo|paul");
    //    cout << sizeof(rx) << endl;
    //    return 0;

    using pscm::Intern, pscm::Cell, pscm::mknum, pscm::mkstr, pscm::nil;

    pscm::Scheme scm;

    scm.mkfun("greet", [cntr = 0](Scheme& scm, const SymenvPtr&, const std::vector<Cell>&) mutable -> Cell {
        return scm.list(mkstr("hello world"), mknum(cntr++));
    });

    if (argn > 1)
        scm.load(argv[1]);
    else
        scm.load("picoscmrc.scm");

    // scm.repl();
    // return 0;

    try {

        pscm::SymenvPtr env = scm.mkenv();

        //        Cell expr = pscm::list(Intern::op_map,
        //            pscm::list(Intern::_lambda, pscm::list(sym("x")), sym("x")),
        //            pscm::list(Intern::_quote, pscm::list(num(1), num(2), num(3))));

        //        Cell expr = pscm::list(Intern::_apply,
        //            pscm::list(Intern::_lambda, pscm::list(sym("x")), sym("x")),
        //            pscm::list(Intern::_quote, pscm::list(num(1))));

        pscm::Parser parser(scm);
        std::istringstream stream("(define (hello x)  "
                                  "   (display x)     "
                                  "   (newline))      "
                                  "                   "
                                  "(define n 43)      "
                                  "(hello n)          ");
        Cell expr = parser.read(stream);
        Cell proc = scm.eval(env, expr);
        cout << proc << endl;

        cout << scm.eval(env, parser.read(stream)) << endl;
        cout << scm.eval(env, parser.read(stream)) << endl;

        scm.gcollect(env);

    } catch (std::bad_variant_access& e) {
        cout << e.what() << endl;

    } catch (std::invalid_argument& e) {
        cout << e.what() << endl;
    }
    return 0;
}
