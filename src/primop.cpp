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

static Cell fun_cons(const varg& args)
{
    return cons(args.at(0), args.at(1));
}

static Cell fun_car(const varg& args)
{
    return car(args.at(0));
}

static Cell fun_cdr(const varg& args)
{
    return cdr(args.at(0));
}

static Cell fun_setcar(const varg& args)
{
    Cell cons = args.at(0);
    set_car(cons, args.at(1));
    return none;
}

static Cell fun_setcdr(const varg& args)
{
    Cell cons = args.at(0);
    set_cdr(cons, args.at(1));
    return none;
}

static Cell fun_list(const varg& args)
{
    Cell list = nil;

    if (args.size()) {
        list = cons(args.front(), nil);

        Cell tail = list;

        for (auto iter = ++args.begin(); iter != args.end(); ++iter, tail = cdr(tail))
            set_cdr(tail, cons(*iter, nil));
    }
    return list;
}

static Cell fun_add(const varg& args)
{
    Number res = 0;

    for (Number val : args)
        res += val;

    return res;
}

static Cell fun_sub(const varg& args)
{
    Number res = args.at(0);

    for (auto iter = ++args.begin(); iter != args.end(); ++iter)
        res -= *iter;

    return res;
}

static Cell fun_mul(const varg& args)
{
    Number res = 1;

    for (Number val : args)
        res *= val;

    return res;
}

static Cell fun_div(const varg& args)
{
    Number res = args.at(0);

    for (auto iter = ++args.begin(); iter != args.end(); ++iter)
        res /= *iter;

    return res;
}

static Cell fun_write(const varg& args)
{
    Port* port = args.size() > 1 ? get<Port*>(args[1]) : &std::cout;

    *port << args.at(0);
    return none;
}

Cell call(const Symenv& senv, Intern primop, const varg& args)
{
    switch (primop) {
    case Intern::op_cons:
        return fun_cons(args);
    case Intern::op_car:
        return fun_car(args);
    case Intern::op_cdr:
        return fun_cdr(args);
    case Intern::op_setcar:
        return fun_setcar(args);
    case Intern::op_setcdr:
        return fun_setcdr(args);
    case Intern::op_list:
        return fun_list(args);
    case Intern::op_add:
        return fun_add(args);
    case Intern::op_sub:
        return fun_sub(args);
    case Intern::op_mul:
        return fun_mul(args);
    case Intern::op_div:
        return fun_div(args);
    default:
        throw std::invalid_argument("invalid primary operation");
    }
}

}; // namespace pscm
