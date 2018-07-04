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
            std::cout << "store=" << store_size() << " : " << expr << std::endl;
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

        //auto in = std::stringstream("(/ 10 3)");
        //expr = parser.parse(in);

        expr = pscm::list(pscm::list(Intern::_lambda, nil, num(1)));

        cout << expr << " ---> ";
        expr = eval(e, expr);
        cout << expr << endl;

    } catch (std::bad_variant_access& e) {
        cout << e.what() << endl;

    } catch (std::invalid_argument& e) {
        cout << e.what() << endl;
    }
    return 0;
}
