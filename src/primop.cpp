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

//using std::get;
using varg = std::vector<Cell>;

/**
 * @brief Check argument vector size.
 * If parameter max is zero, exact n arguments are required otherwise
 * at least n up to max.
 *
 * @param args Argument vector to check.
 * @param n    Number of arguments, or minimum number of arguments.
 * @param max  Zero or maximum number of arguments.
 */
static void argn(const varg& args, size_t n, size_t max = 0)
{
    size_t argn = args.size();

    if (!max && argn != n)
        throw std::invalid_argument("invalid number of arguments");

    else if (max && (argn < n || argn > max))
        throw std::invalid_argument("invalid number of arguments");
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

static Cell fun_booleq(const varg& args)
{
    argn(args, 2, ~0);

    if (!is_bool(args[0]))
        return false;

    Bool prv = args[0], b;

    for (auto iter = args.begin() + 1; iter != args.end(); prv = b, ++iter)
        if (!is_bool(*iter) || prv != (b = *iter))
            return false;

    return true;
}

static Cell fun_numeq(const varg& args)
{
    argn(args, 2, ~0);

    if (args[0] != args[1])
        return false;

    Number val = args[1];
    for (auto iter = args.begin() + 2; iter != args.end(); val = std::get<Number>(*iter), ++iter)
        if (val != *iter)
            return false;

    return true;
}

static Cell fun_numlt(const varg& args)
{
    argn(args, 2, ~0);

    Number lhs = args[0], rhs = args[1];

    if (!(lhs < rhs))
        return false;

    for (auto iter = args.begin() + 2; iter != args.end(); ++iter) {
        lhs = rhs;
        rhs = std::get<Number>(static_cast<Cell>(*iter));

        if (!(lhs < rhs))
            return false;
    }
    return true;
}

static Cell fun_numgt(const varg& args)
{
    argn(args, 2, ~0);

    Number lhs = args[0], rhs = args[1];

    if (!(lhs > rhs))
        return false;

    for (auto iter = args.begin() + 2; iter != args.end(); ++iter) {
        lhs = rhs;
        rhs = std::get<Number>(static_cast<Cell>(*iter));

        if (!(lhs > rhs))
            return false;
    }
    return true;
}

static Cell fun_numle(const varg& args)
{
    argn(args, 2, ~0);

    Number lhs = args[0], rhs = args[1];

    if (!(lhs <= rhs))
        return false;

    for (auto iter = args.begin() + 2; iter != args.end(); ++iter) {
        lhs = rhs;
        rhs = std::get<Number>(*iter);

        if (!(lhs <= rhs))
            return false;
    }
    return true;
}

static Cell fun_numge(const varg& args)
{
    argn(args, 2, ~0);

    Number lhs = args[0], rhs = args[1];

    if (!(lhs >= rhs))
        return false;

    for (auto iter = args.begin() + 2; iter != args.end(); ++iter) {
        lhs = rhs;
        rhs = std::get<Number>(*iter);

        if (!(lhs >= rhs))
            return false;
    }
    return true;
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
    argn(args, 1, ~0);

    Number res = args.size() > 1 ? args[0] : neg(args[0]);

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
    argn(args, 1, ~0);

    Number res = args.size() > 1 ? args[0] : inv(args[0]);

    for (auto iter = ++args.begin(); iter != args.end(); ++iter)
        res /= *iter;

    return res;
}

static Cell fun_log(const varg& args)
{
    argn(args, 1, 2);

    if (args.size() == 1)
        return pscm::log(args[0]);
    else {
        Number x = args[0], y = args[1];
        return y != Number{ 10 } ? pscm::log(x) / pscm::log(y)
                                 : pscm::log10(x);
    }
}

static Cell fun_write(const varg& args)
{
    argn(args, 1, 2);

    Port* port = args.size() > 1 ? std::get<Port*>(args[1]) : &std::cout;

    *port << args[0];
    return none;
}

Cell call(const Symenv& senv, Intern primop, const varg& args)
{
    switch (primop) {
    /* Section 6.1: Equivalence predicates */
    case Intern::op_eq:
    case Intern::op_eqv:
        return argn(args, 2), args[0] == args[1];
    case Intern::op_equal:
        return argn(args, 2), is_equal(args[0], args[1]);

    /* Section 6.2: Numbers */
    case Intern::op_numeq:
        return fun_numeq(args);
    case Intern::op_numlt:
        return fun_numlt(args);
    case Intern::op_numgt:
        return fun_numgt(args);
    case Intern::op_numle:
        return fun_numle(args);
    case Intern::op_numge:
        return fun_numge(args);
    case Intern::op_add:
        return fun_add(args);
    case Intern::op_sub:
        return fun_sub(args);
    case Intern::op_mul:
        return fun_mul(args);
    case Intern::op_div:
        return fun_div(args);
    case Intern::op_zero:
        return argn(args, 1), !(std::get<Number>(args[0]) != Number{ 0 });
    case Intern::op_sin:
        return argn(args, 1), pscm::sin(args[0]);
    case Intern::op_cos:
        return argn(args, 1), pscm::cos(args[0]);
    case Intern::op_tan:
        return argn(args, 1), pscm::tan(args[0]);
    case Intern::op_asin:
        return argn(args, 1), pscm::asin(args[0]);
    case Intern::op_acos:
        return argn(args, 1), pscm::acos(args[0]);
    case Intern::op_atan:
        return argn(args, 1), pscm::atan(args[0]);
    case Intern::op_sinh:
        return argn(args, 1), pscm::sinh(args[0]);
    case Intern::op_cosh:
        return argn(args, 1), pscm::cosh(args[0]);
    case Intern::op_tanh:
        return argn(args, 1), pscm::tanh(args[0]);
    case Intern::op_asinh:
        return argn(args, 1), pscm::asinh(args[0]);
    case Intern::op_acosh:
        return argn(args, 1), pscm::acosh(args[0]);
    case Intern::op_atanh:
        return argn(args, 1), pscm::atanh(args[0]);
    case Intern::op_exp:
        return argn(args, 1), pscm::exp(args[0]);
    case Intern::op_pow:
        return argn(args, 2), pscm::pow(args[0], args[1]);
    case Intern::op_square:
        return argn(args, 1), args[0] * args[0];
    case Intern::op_log:
        return fun_log(args);
    case Intern::op_log10:
        return argn(args, 1), pscm::log10(args[0]);
    case Intern::op_sqrt:
        return argn(args, 1), pscm::sqrt(args[0]);
    case Intern::op_abs:
        return argn(args, 1), pscm::abs(args[0]);
    case Intern::op_real:
        return argn(args, 1), pscm::real(args[0]);
    case Intern::op_imag:
        return argn(args, 1), pscm::imag(args[0]);
    case Intern::op_arg:
        return argn(args, 1), pscm::arg(args[0]);
    case Intern::op_conj:
        return argn(args, 1), pscm::conj(args[0]);
    case Intern::op_rect:
        return argn(args, 2), pscm::rect(args[0], args[1]);
    case Intern::op_polar:
        return argn(args, 2), pscm::polar(args[0], args[1]);

    /* Section 6.3: Booleans */
    case Intern::op_not:
        return argn(args, 1), !is_true(args[0]);
    case Intern::op_isbool:
        return argn(args, 1), is_bool(args[0]);
    case Intern::op_isbooleq:
        return fun_booleq(args);

    /* Section 6.4: Pair and lists */
    case Intern::op_cons:
        return argn(args, 2), cons(args[0], args[1]);
    case Intern::op_car:
        return argn(args, 1), car(args[0]);
    case Intern::op_cdr:
        return argn(args, 1), cdr(args[0]);
    case Intern::op_setcar:
        return argn(args, 2), set_car(args[0], args[1]), none;
    case Intern::op_setcdr:
        return argn(args, 2), set_cdr(args[0], args[1]), none;
    case Intern::op_list:
        return fun_list(args);

    /* Section 6.6: Characters */
    case Intern::op_ischar:
        return argn(args, 1),
               is_char(args[0]);
    case Intern::op_charint:
        return argn(args, 1), num(std::get<Char>(args[0]));

        /* Section 6.7: Strings */

    /* Section 6.8: Vectors */
    case Intern::op_mkvec:
        return argn(args, 1, 2), args.size() != 2 ? vec(args[0]) : vec(args[0], args[1]);
    case Intern::op_vec:
        return vcopy(args);

    default:
        throw std::invalid_argument("invalid primary operation");
    }
}
}; // namespace pscm
