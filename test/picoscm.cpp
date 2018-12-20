/********************************************************************************/ /**
 * @file pico.cpp
 *
 *************************************************************************************/
#include <assert.h>
#include <iostream>
#include <memory>

#include <codecvt>
#include <locale>

#include <picoscm/cell.hpp>
#include <picoscm/parser.hpp>
#include <picoscm/scheme.hpp>
#include <picoscm/utils.hpp>

using namespace std;
using namespace pscm;

int main(int argn, char* argv[])
{
    using pscm::Intern, pscm::Cell, pscm::str, pscm::nil;

    pscm::Scheme scm;
    scm.function("greet", [cntr = 0](Scheme& scm, const SymenvPtr&, const std::vector<Cell>&) mutable -> Cell {
        return scm.list(pscm::str("hello world"), pscm::num(cntr++));
    });

    if (argn > 1)
        scm.load(argv[0]);
    else
        scm.load("picoscmrc.scm");

    scm.repl();
    return 0;

    try {
        pscm::SymenvPtr env = scm.getenv();

        //        // (for-each (lambda (x) x) (quote (1 2 3 4)))
        //        Cell expr = scm.list(Intern::op_foreach,
        //            scm.list(Intern::_lambda, scm.list(scm.mksym(L"x")), scm.mksym(L"x")),
        //            scm.list(Intern::_quote, scm.list(pscm::mknum(1), pscm::mknum(2))));

        pscm::Parser parser(scm);
        std::wistringstream stream(L"(map car '((1 . 2) (10 . 20) (100 . 200))))");
        Cell expr = parser.read(stream);

        wcout << expr << endl;
        Cell proc = scm.eval(env, expr);
        wcout << proc << endl;
        return 0;

    } catch (std::bad_variant_access& e) {
        cout << e.what() << endl;

    } catch (std::invalid_argument& e) {
        cout << e.what() << endl;
    }
    return 0;
}
