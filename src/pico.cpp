/********************************************************************************/ /**
 * @file pico.hpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#include "cell.hpp"
#include "number.hpp"

using namespace std;
using namespace pscm;

int main()
{
    try {
        Cell lst = pscm::list(Intern::_and, Intern::_or, Intern::_cond, Intern::_define, Intern::_setb, Intern::_begin, Intern::_apply, Intern::_lambda);

        Cell c = fun_write(cons(lst, nil));
        cout << is_none(c) << endl;

        Float x = -10.3;

        Number n0;
        Number n1 = 3;
        Number n2 = x;
        Number n3 = { 3, 5.3 };
        Number n4{ n2, n1 };

        cout << "n0: " << n0 << ' ' << is_type<Int>(n0) << endl
             << "n1: " << n1 << ' ' << is_type<Int>(n1) << endl
             << "n2: " << n2 << ' ' << is_type<Int>(n2) << endl
             << "n3: " << n3 << ' ' << is_type<Complex>(n3) << endl;

        auto val = pow(n2, n0);

        cout << val << ' ' << is_type<Int>(val) << endl;

    } catch (std::bad_variant_access& e) {
        cout << e.what() << endl;
    } catch (std::invalid_argument& e) {
        cout << e.what() << endl;
    }
    return 0;
}
