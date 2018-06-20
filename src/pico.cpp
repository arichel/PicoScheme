/********************************************************************************//**
 * @file pico.hpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#include "cell.hpp"

using namespace std;
using namespace pscm;

int main()
{
    try {
        //Cell lst = pscm::list(str("hallo"), str("paul"), sym("hulla hub"), s_nil, s_none, s_false, fun_write);

        Symbol s = atom("#none");

        stab.print();

        cout << (s == stab.s_true) << ' ' << s.is_intern() << ' ' << stab.s_true.is_intern() << endl;;

        //fun_write(cons(lst, nil));
        //cout << endl;
    }
    catch (std::bad_variant_access& e)
    {
        cout << e.what() << endl;
    }
    catch (std::invalid_argument& e)
    {
        cout << e.what() << endl;
    }
    return 0;
}
