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
        Number n0;

        constexpr Number n1 = Int{ 3 };
        constexpr Number n2 = Float{ 3.3 };
        constexpr Number n3 = Complex{ 3., 4. };

        Number val = n3 + n2;

        std::complex<Float> z0{ 20., 30. };

        Int x = 100;

        std::complex<Float> res = z0 + x;

        cout << res << ' ' << is_type<Float>(val) << endl;

    } catch (std::bad_variant_access& e) {
        cout << e.what() << endl;
    } catch (std::invalid_argument& e) {
        cout << e.what() << endl;
    }
    return 0;
}
