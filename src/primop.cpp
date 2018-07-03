/*********************************************************************************/ /**
 * @file primop.cpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#include <iostream>

#include "primop.hpp"
#include "stream.hpp"

namespace pscm {

using std::get;
using varg = std::vector<Cell>;

static Cell fun_add(const varg& args)
{
    Number sum = 0;

    for (Number val : args)
        sum += val;

    return sum;
}

Cell fun_sub(const varg& args)
{
    Number diff = args.at(0);

    for (auto iter = ++args.begin(); iter != args.end(); ++iter)
        diff -= *iter;

    return diff;
}

Cell fun_mul(const varg& args)
{
    Number prod = 0;

    for (Number val : args)
        prod *= val;

    return prod;
}

Cell fun_write(const varg& args)
{
    Port* port = args.size() > 1 ? get<Port*>(args[1]) : &std::cout;

    *port << args.at(0);
    return none;
}

Cell call(const Symenv& senv, Intern primop, const varg& args)
{
    switch (primop) {
    case Intern::op_add:
        return fun_add(args);
    case Intern::op_sub:
        return fun_sub(args);
    case Intern::op_mul:
        return fun_mul(args);
    default:
        throw std::invalid_argument("invalid primary operation");
    }
}

}; // namespace pscm
