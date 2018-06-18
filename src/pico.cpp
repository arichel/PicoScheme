/********************************************************************************//**
 * @file pico.hpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#include "cell.hpp"

using namespace pscm;

int main()
{
    String s {"huhu"};

    Cell expr = cons(func(fun_write), cons(&s, cons(str("paul"), cons(10., nil))));
    Cell args = cons(expr, cons(port(), nil));
    fun_write(args);

    return 0;
}
