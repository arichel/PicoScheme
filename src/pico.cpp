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
        Cell expr = cons(str("huhu"), cons(str("paul"), cons(10., nil)));

        set_car(cdr(expr), str("hubert"));

        Cell args = cons(expr, nil);

        fun_write(args);

        cout << sizeof(expr) << endl;
    }
    catch (std::bad_variant_access& e)
    {
        cout << e.what() << endl;
    }
    return 0;
}
