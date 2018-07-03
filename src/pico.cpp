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

void repl()
{
    Symenv env = senv();
    Parser reader;
    Cell expr;

start:
    try {
        for (;;) {
            expr = reader.parse(std::cin);
            expr = eval(env, expr);
            std::cout << expr << std::endl;
        }
    } catch (std::bad_variant_access& e) {
        cout << e.what() << endl;

    } catch (std::invalid_argument& e) {
        cout << e.what() << endl;
    }
    goto start;
}

int main()
{
    repl();
    return 0;

    try {
        Symenv e = senv();
        Parser parser;
        Cell expr;

        auto in = std::stringstream("((lambda (x) (+ x x)) 100)");
        expr = parser.parse(in);

        cout << expr << " ---> ";
        cout << eval(e, expr) << endl;

    } catch (std::bad_variant_access& e) {
        cout << e.what() << endl;

    } catch (std::invalid_argument& e) {
        cout << e.what() << endl;
    }
    return 0;
}
