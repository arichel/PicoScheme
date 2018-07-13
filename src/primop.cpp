/*********************************************************************************/ /**
 * @file primop.cpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#include <iostream>

#include "eval.hpp"
#include "parser.hpp"
#include "primop.hpp"

namespace pscm {

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

    if (args.size() > 1) {
        Port& port = std::get<Port>(const_cast<Cell&>(args[1]));

        port.stream() << args[0];
    } else
        std::cout << args[0];

    return none;
}

inline Cell fun_make_vector(const varg& args)
{
    argn(args, 1, 2);

    Number size = args[0];

    is_int(size) && !is_negative(size)
        || (throw std::invalid_argument("vector length must be a non-negative integer"), 0);

    return Vector{ size, args.size() == 2 ? args[1] : Cell{ none } };
}

inline Cell fun_vector_ref(const varg& args)
{
    argn(args, 2);

    Number pos = args[1];
    is_int(pos) && !is_negative(pos)
        || (throw std::invalid_argument("vector position must be a non-negative integer"), 0);

    return std::get<Vector>(args[0]).at(pos);
}

inline Cell fun_vector_setb(const varg& args)
{
    argn(args, 3);

    Number pos = args[1];
    is_int(pos) && !is_negative(pos)
        || (throw std::invalid_argument("vector position must be a non-negative integer"), 0);

    std::get<Vector>(Cell{ args[0] }).at(pos) = args[2];
    return none;
}

inline Cell fun_vec2list(const varg& args)
{
    argn(args, 1, 3);
    Vector vec = args[0];
    Number pos = args.size() > 1 ? args[1] : Number{ 0 },
           end = args.size() > 2 ? args[2] : Number{ vec.size() };

    is_int(pos) && !is_negative(pos) && pos <= end
        || (throw std::invalid_argument("invalid first vector index"), 0);

    is_int(end) && !is_negative(end) && end <= Number{ vec.size() }
        || (throw std::invalid_argument("invalid second vector index"), 0);

    if (!vec.size())
        return nil;

    Cell list = cons(vec.at(pos), nil), tail = list;

    std::for_each(vec.begin() + pos + 1, vec.begin() + end, [&tail](Cell& cell) {
        set_cdr(tail, cons(cell, nil));
        tail = cdr(tail);
    });
    return list;
}

inline Cell fun_list2vec(const varg& args)
{
    argn(args, 1);
    Vector vec;

    Cell list = args[0];
    for (/* */; is_pair(list); list = cdr(list))
        vec.push_back(car(list));

    is_nil(list) || (throw std::invalid_argument("not a proper list"), 0);
    return vec;
}

inline Cell fun_vec_copy(const varg& args)
{
    argn(args, 1, 3);
    Vector vec = args[0];

    Number pos = args.size() > 1 ? args[1] : Number{ 0 },
           end = args.size() > 2 ? args[2] : Number{ vec.size() };

    is_int(pos) && !is_negative(pos) && pos <= end
        || (throw std::invalid_argument("invalid first vector index"), 0);

    is_int(end) && !is_negative(end) && end <= Number{ vec.size() }
        || (throw std::invalid_argument("invalid second vector index"), 0);

    return Vector{ vec.begin() + pos, vec.begin() + end };
}

inline Cell fun_vec_copyb(const varg& args)
{
    argn(args, 3, 5);
    Vector vec = args[0], src = args[2];

    Number idx = args[1],
           pos = args.size() > 3 ? args[3] : Number{ 0 },
           end = args.size() > 4 ? args[4] : Number{ src.size() };

    is_int(idx) && !is_negative(idx)
        || (throw std::invalid_argument("invalid destination vector index"), 0);

    is_int(pos) && !is_negative(pos) && pos <= end
        || (throw std::invalid_argument("invalid first vector index"), 0);

    is_int(end) && !is_negative(end) && end <= Number{ src.size() }
        || (throw std::invalid_argument("invalid second vector index"), 0);

    vec.copy(idx, src.begin() + pos, src.begin() + end);
    return vec;
}

inline Cell fun_vec_append(const varg& args)
{
    argn(args, 1, ~0);
    Vector vec = args[0];

    for (const Vector& v : args)
        vec.append(v.begin(), v.end());

    return vec;
}

inline Cell fun_vec_fillb(const varg& args)
{
    argn(args, 2, 4);
    Vector vec = args[0];

    Number pos = args.size() > 2 ? args[2] : Number{ 0 },
           end = args.size() > 3 ? args[3] : Number{ vec.size() };

    is_int(pos) && !is_negative(pos) && pos <= end
        || (throw std::invalid_argument("invalid first vector index"), 0);

    is_int(end) && !is_negative(end) && end <= Number{ vec.size() }
        || (throw std::invalid_argument("invalid second vector index"), 0);

    vec.fill(args[1], pos, end);
    return vec;
}

inline Cell fun_callw_infile(const Symenv& senv, const String& filnam, const Cell& proc)
{
    Port port;
    port.open(*filnam, std::ios_base::in)
        || (throw std::invalid_argument("could not open port"), 0);

    Cons cons[4];
    return eval(senv, alist(cons, Intern::_apply, proc, port, nil));
}

inline Cell fun_open_infile(const String& filnam)
{
    Port port;

    port.open(*filnam, std::ios_base::in)
        || (throw std::invalid_argument("could not open port"), 0);

    return port;
}

inline Cell fun_open_outfile(const String& filnam)
{
    Port port;

    port.open(*filnam, std::ios_base::out)
        || (throw std::invalid_argument("could not open port"), 0);

    return port;
}

inline Cell fun_readline(const varg& args)
{
    argn(args, 0, 1);
    String line = str("");

    if (args.size() > 0) {
        Port port = args[0];
        port.is_open() || (throw std::invalid_argument("port is closed"), 0);
        std::getline(port.stream(), *line);
    } else {
        std::string str;
        std::getline(std::cin, str);
        std::cout << "--> str: " << str << std::endl;
    }

    return line;
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
        return argn(args, 1), is_type<Char>(args[0]);
    case Intern::op_charint:
        return argn(args, 1), num(std::get<Char>(args[0]));

    /* Section 6.7: Strings */
    case Intern::op_isstr:
        return argn(args, 1), is_type<String>(args[0]);

    /* Section 6.8: Vectors */
    case Intern::op_isvec:
        return argn(args, 1), is_type<Vector>(args[0]);
    case Intern::op_mkvec:
        return fun_make_vector(args);
    case Intern::op_vec:
        return Vector{ args };
    case Intern::op_veclen:
        return argn(args, 1), Number{ std::get<Vector>(args[0]).size() };
    case Intern::op_vecref:
        return fun_vector_ref(args);
    case Intern::op_vecsetb:
        return fun_vector_setb(args);
    case Intern::op_veclist:
        return fun_vec2list(args);
    case Intern::op_listvec:
        return fun_list2vec(args);
    case Intern::op_veccopy:
        return fun_vec_copy(args);
    case Intern::op_veccopyb:
        return fun_vec_copyb(args);
    case Intern::op_vecappend:
        return fun_vec_append(args);
    case Intern::op_vecfillb:
        return fun_vec_fillb(args);

    /* Section 6.9: Bytevectors */

    /* Section 6.10: Control features */
    case Intern::op_isproc:
        return argn(args, 1), is_proc(args[0]) || (is_intern(args[0]) && std::get<Intern>(args[0]) >= Intern::op_eq);

    /* Section 6.13: Input and output */
    case Intern::op_callw_infile:
        return fun_callw_infile(senv, args.at(0), args.at(1));
    case Intern::op_open_infile:
        return fun_open_infile(args.at(0));
    case Intern::op_open_outfile:
        return fun_open_outfile(args.at(0));
    case Intern::op_readline:
        return fun_readline(args);

    default:
        throw std::invalid_argument("invalid primary operation");
    }
}
}; // namespace pscm
