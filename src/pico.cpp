/********************************************************************************/ /**
 * @file pico.hpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#include <assert.h>
#include <iostream>

#include "cell.hpp"
#include "eval.hpp"
#include "stream.hpp"
#include "symbol.hpp"
#include "types.hpp"

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
        cout << e.what() << ": " << expr << endl;

    } catch (std::out_of_range& e) {
        cout << e.what() << ": " << expr << endl;

    } catch (std::invalid_argument& e) {
        cout << e.what() << ": " << expr << endl;
    }
    goto start;
}

auto test(const Symenv& senv, const Proc& proc)
{
    return proc.apply(senv, pscm::list(num(10)), true);
}

struct Test {

    Test() = default;
    Test(const Test&) = default;
    Test(size_t size)
    {
    }
    std::vector<double> vec;
    double val;
};

int main()
{
    repl();
    return 0;

    try {

        Test tst;

        std::unordered_map<int, double> map(0);

        cout << map.bucket_count() << endl;

        //        Symenv e = senv();
        //        Cell expr = pscm::list(sym("lambda"), pscm::list(sym("x")), pscm::list(sym("*"), sym("x"), sym("x")));

        //        //        Parser parser;
        //        //        Cell expr;

        //        cout << expr << " ---> ";
        //        Cell proc = eval(e, expr);

        //        cout << test(e, proc).second << endl;

    } catch (std::bad_variant_access& e) {
        cout << e.what() << endl;

    } catch (std::invalid_argument& e) {
        cout << e.what() << endl;
    }
    return 0;
}
