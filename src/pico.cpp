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
    Cell s = str("hallo");
    cout << std::get<String>(s).use_count() << endl;

    try {
        Cell lst = pscm::list(str("hallo"), s, str("paul"), s, fun_write);

        fun_write(cons(lst, nil));

        cout << "length " << list_length(lst) << endl;

        cout << std::get<String>(s).use_count() << endl;
     }
    catch (std::bad_variant_access& e)
    {
        cout << e.what() << endl;
    }
    catch (std::invalid_argument& e)
    {
        cout << e.what() << endl;
    }
    cout << store.size() << endl;
    return 0;
}
