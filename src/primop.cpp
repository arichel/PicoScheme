/*********************************************************************************/ /**
 * @file primop.cpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/

#include "primop.hpp"

namespace pscm {

Cell fun_add(const std::vector<Cell>& args)
{
    Number sum = 0;

    for (Number val : args)
        sum += val;

    return sum;
}

}; // namespace pscm
