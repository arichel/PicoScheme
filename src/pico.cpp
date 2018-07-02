/********************************************************************************/ /**
 * @file pico.hpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#include <iostream>

#include "cell.hpp"
#include "eval.hpp"
#include "number.hpp"
#include "stream.hpp"

using namespace std;
using namespace pscm;

std::tuple<int, double, std::string> testret()
{
    return std::make_tuple(100, 300., "hallo paul");
}

int main()
{
    try {

        int i = 0;
        double x = 0.;
        string s = "blub";

        std::tie(i, x, s) = testret();

        cout << i << ' ' << x << ' ' << s << endl;

        Symenv e = senv();
        Cell expr;

        expr = pscm::list(Intern::_define, sym("x0"), num(1000));
        cout << eval(e, expr) << endl;

        expr = pscm::list(Intern::_define, sym("+"), Intern::op_add);
        cout << eval(e, expr) << endl;

        expr = pscm::list(Intern::_setb, sym("x0"), str("hallo paul"));
        cout << eval(e, expr) << endl;

        expr = pscm::list(Intern::_or, 1_int, 2_int, false, 3_int);
        cout << eval(e, expr) << endl;

        expr = pscm::list(Intern::op_add, 1_int, 2_int, pscm::list(Intern::op_add, 100_int, 200_int), 4_int, 5_int);
        cout << eval(e, expr) << endl;

        expr = pscm::list(Intern::_lambda, pscm::list(sym("a"), sym("b")),
            pscm::list(Intern::op_add, sym("a"), sym("b")));
        cout << eval(e, expr) << endl;

        expr = pscm::list(Intern::_define, pscm::list(sym("hello"), sym("x")),
            pscm::list(Intern::op_add, sym("x"), sym("x")));

        cout << eval(e, expr) << endl;
        cout << eval(e, pscm::list(sym("hello"), num(100))) << endl;

        expr = pscm::list(Intern::_if, false, true);
        cout << eval(e, expr) << endl;

        Cell args = pscm::list(Intern::_quote, pscm::list(num(100), num(200)));

        expr = pscm::list(Intern::_apply, sym("+"), num(3), args);
        cout << eval(e, expr) << endl;

    } catch (std::bad_variant_access& e) {
        cout << e.what() << endl;
    } catch (std::invalid_argument& e) {
        cout << e.what() << endl;
    }
    return 0;
}
