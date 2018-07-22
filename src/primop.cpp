/*********************************************************************************/ /**
 * @file primop.cpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#include <algorithm>
#include <iostream>
#include <memory>

#include "cell.hpp"
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

    Bool prv = get<Bool>(args[0]), b;

    for (auto iter = args.begin() + 1; iter != args.end(); prv = b, ++iter)
        if (!is_bool(*iter) || prv != (b = get<Bool>(*iter)))
            return false;

    return true;
}

static Cell fun_numeq(const varg& args)
{
    argn(args, 2, ~0);

    if (args[0] != args[1])
        return false;

    auto val = get<Number>(args[1]);
    for (auto iter = args.begin() + 2; iter != args.end(); val = get<Number>(*iter), ++iter)
        if (val != get<Number>(*iter))
            return false;

    return true;
}

static Cell fun_numlt(const varg& args)
{
    argn(args, 2, ~0);

    auto lhs = get<Number>(args[0]), rhs = get<Number>(args[1]);

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

    auto lhs = get<Number>(args[0]), rhs = get<Number>(args[1]);

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

    auto lhs = get<Number>(args[0]), rhs = get<Number>(args[1]);

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

    auto lhs = get<Number>(args[0]), rhs = get<Number>(args[1]);

    if (!(lhs >= rhs))
        return false;

    for (auto iter = args.begin() + 2; iter != args.end(); ++iter) {
        lhs = rhs;
        rhs = get<Number>(*iter);

        if (!(lhs >= rhs))
            return false;
    }
    return true;
}

static Cell fun_add(const varg& args)
{
    Number res = 0;

    for (auto& val : args)
        res += get<Number>(val);

    return res;
}

static Cell fun_sub(const varg& args)
{
    auto res = args.size() > 1 ? get<Number>(args.at(0)) : -get<Number>(args.at(0));

    for (auto iter = ++args.begin(); iter != args.end(); ++iter)
        res -= get<Number>(*iter);

    return res;
}

static Cell fun_mul(const varg& args)
{
    Number res = 1;

    for (const Cell& val : args)
        res *= get<Number>(val);

    return res;
}

static Cell fun_div(const varg& args)
{
    auto res = args.size() > 1 ? get<Number>(args[0]) : inv(get<Number>(args.at(0)));

    for (auto iter = ++args.begin(); iter != args.end(); ++iter)
        res /= get<Number>(*iter);

    return res;
}

static Cell fun_log(const varg& args)
{
    if (args.size() < 2)
        return pscm::log(get<Number>(args.at(0)));
    else {
        const Number &y = get<Number>(args.at(1)), &x = get<Number>(args[0]);

        return y != Number{ 10 } ? pscm::log(x) / pscm::log(y)
                                 : pscm::log10(x);
    }
}

static Cell fun_write(const varg& args)
{
    if (args.size() > 1) {
        Port& port = std::get<Port>(const_cast<Cell&>(args.at(1)));

        port.stream() << args[0];
    } else
        std::cout << args.at(0);

    return none;
}

inline Cell fun_make_vector(const varg& args)
{
    const Number& size = get<Number>(args.at(0));

    is_int(size) && !is_negative(size)
        || (throw std::invalid_argument("vector length must be a non-negative integer"), 0);

    return vec(size, args.size() > 1 ? args[1] : Cell{ none });
}

inline Cell fun_vector_ref(const varg& args)
{
    const Number& pos = get<Number>(args.at(1));

    is_int(pos) && !is_negative(pos)
        || (throw std::invalid_argument("vector position must be a non-negative integer"), 0);

    return std::get<VectorPtr>(args[0])->at(pos);
}

inline Cell fun_vector_setb(const varg& args)
{
    argn(args, 3);
    auto pos = get<Number>(args[1]);

    is_int(pos) && !is_negative(pos)
        || (throw std::invalid_argument("vector position must be a non-negative integer"), 0);

    std::get<VectorPtr>(args[0])->at(pos) = args[2];
    return none;
}

inline Cell fun_vec2list(const varg& args)
{
    argn(args, 1, 3);

    const VectorPtr& vec = get<VectorPtr>(args[0]);

    Number pos = args.size() > 1 ? get<Number>(args[1]) : Number{ 0 },
           end = args.size() > 2 ? get<Number>(args[2]) : Number{ vec->size() };

    is_int(pos) && !is_negative(pos) && pos <= end
        || (throw std::invalid_argument("invalid first vector index"), 0);

    is_int(end) && !is_negative(end) && end <= Number{ vec->size() }
        || (throw std::invalid_argument("invalid second vector index"), 0);

    if (!vec->size())
        return nil;

    Cell list = cons(vec->at(pos), nil), tail = list;

    std::for_each(vec->begin() + pos + 1, vec->begin() + end, [&tail](const Cell& cell) {
        set_cdr(tail, cons(cell, nil));
        tail = cdr(tail);
    });
    return list;
}

inline Cell fun_list2vec(const varg& args)
{
    Cell list = args.at(0);
    VectorPtr v = std::make_shared<VectorPtr::element_type>();

    for (/* */; is_pair(list); list = cdr(list))
        v->push_back(car(list));

    is_nil(list) || (throw std::invalid_argument("not a proper list"), 0);
    return v;
}

inline Cell fun_vec_copy(const varg& args)
{
    argn(args, 1, 3);

    const VectorPtr& v = get<VectorPtr>(args[0]);
    Number pos = args.size() > 1 ? get<Number>(args[1]) : Number{ 0 },
           end = args.size() > 2 ? get<Number>(args[2]) : Number{ v->size() };

    is_int(pos) && !is_negative(pos) && pos <= end
        || (throw std::invalid_argument("invalid first vector index"), 0);

    is_int(end) && !is_negative(end) && end <= Number{ v->size() }
        || (throw std::invalid_argument("invalid second vector index"), 0);

    return std::make_shared<VectorPtr::element_type>(v->begin() + pos, v->begin() + end);
}

inline Cell fun_vec_copyb(const varg& args)
{
    argn(args, 3, 5);

    const VectorPtr& src = get<VectorPtr>(args[2]);
    VectorPtr dst = get<VectorPtr>(args[0]);

    auto idx = get<Number>(args[1]),
         pos = args.size() > 3 ? get<Number>(args[3]) : Number{ 0 },
         end = args.size() > 4 ? get<Number>(args[4]) : Number{ src->size() };

    is_int(idx) && !is_negative(idx)
        || (throw std::invalid_argument("invalid destination vector index"), 0);

    is_int(pos) && !is_negative(pos) && pos <= end
        || (throw std::invalid_argument("invalid first vector index"), 0);

    is_int(end) && !is_negative(end) && end <= Number{ src->size() }
        || (throw std::invalid_argument("invalid second vector index"), 0);

    std::copy(src->begin() + pos, src->begin() + end, dst->begin() + idx);
    return dst;
}

inline Cell fun_vec_append(const varg& args)
{
    VectorPtr vec = get<VectorPtr>(args.at(0));

    for (const Cell& cell : args) {
        const VectorPtr& v = get<VectorPtr>(cell);

        std::copy(v->begin(), v->end(), std::back_inserter(*vec));
    }
    return vec;
}

inline Cell fun_vec_fillb(const varg& args)
{
    argn(args, 2, 4);

    VectorPtr vec = get<VectorPtr>(args[0]);
    auto pos = args.size() > 2 ? get<Number>(args[2]) : Number{ 0 },
         end = args.size() > 3 ? get<Number>(args[3]) : Number{ vec->size() };

    is_int(pos) && !is_negative(pos) && pos <= end
        || (throw std::invalid_argument("invalid first vector index"), 0);

    is_int(end) && !is_negative(end) && end <= Number{ vec->size() }
        || (throw std::invalid_argument("invalid second vector index"), 0);

    std::fill(vec->begin() + pos, vec->end() + std::min(vec->size(), static_cast<size_t>(end)), args[1]);
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
        Port port = get<Port>(args[0]);
        port.is_open() && port.is_input() || (throw std::invalid_argument("port is closed"), 0);

        std::getline(port.stream(), *line);
    } else {
        std::string str;
        std::getline(std::cin, str);
    }
    return line;
}

inline Cell fun_read(const varg& args)
{
    argn(args, 0, 1);
    Port port;

    if (args.size() > 0) {
        port = get<Port>(args[0]);
        port.is_open() && port.is_input() || (throw std::invalid_argument("port is closed"), 0);
    }
    Parser parser;
    return parser.parse(port.stream());
}

inline Cell fun_readchar(const varg& args)
{
    argn(args, 0, 1);
    Port port;

    if (args.size() > 0) {
        port = get<Port>(args[0]);
        port.is_open() && port.is_input() || (throw std::invalid_argument("port is closed"), 0);
    }
    return static_cast<Char>(port.stream().get());
}

inline Cell fun_peekchar(const varg& args)
{
    argn(args, 0, 1);
    Port port;

    if (args.size() > 0) {
        port = get<Port>(args[0]);
        port.is_open() && port.is_input() || (throw std::invalid_argument("port is closed"), 0);
    }
    return static_cast<Char>(port.stream().peek());
}

inline Cell fun_readstr(const varg& args)
{
    argn(args, 1, 2);
    Port port;

    Number num = get<Number>(args[0]);
    if (is_int(num) || is_negative(num))
        throw std::invalid_argument("must be a nonnegative number");

    if (args.size() > 1) {
        port = get<Port>(args[0]);
        port.is_open() && port.is_input() || (throw std::invalid_argument("port is closed"), 0);
    }
    Parser parser;
    return parser.parse(port.stream());
}

Cell call(const Symenv& senv, Intern primop, const varg& args)
{
    switch (primop) {
    /* Section 6.1: Equivalence predicates */
    case Intern::op_eq:
    case Intern::op_eqv:
        return args.at(0) == args.at(1);
    case Intern::op_equal:
        return is_equal(args.at(0), args.at(1));

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
        return is_zero(get<Number>(args.at(0)));
    case Intern::op_sin:
        return pscm::sin(get<Number>(args.at(0)));
    case Intern::op_cos:
        return pscm::cos(get<Number>(args.at(0)));
    case Intern::op_tan:
        return pscm::tan(get<Number>(args.at(0)));
    case Intern::op_asin:
        return pscm::asin(get<Number>(args.at(0)));
    case Intern::op_acos:
        return pscm::acos(get<Number>(args.at(0)));
    case Intern::op_atan:
        return pscm::atan(get<Number>(args.at(0)));
    case Intern::op_sinh:
        return pscm::sinh(get<Number>(args.at(0)));
    case Intern::op_cosh:
        return pscm::cosh(get<Number>(args.at(0)));
    case Intern::op_tanh:
        return pscm::tanh(get<Number>(args.at(0)));
    case Intern::op_asinh:
        return pscm::asinh(get<Number>(args.at(0)));
    case Intern::op_acosh:
        return pscm::acosh(get<Number>(args.at(0)));
    case Intern::op_atanh:
        return pscm::atanh(get<Number>(args.at(0)));
    case Intern::op_exp:
        return pscm::exp(get<Number>(args.at(0)));
    case Intern::op_pow:
        return pscm::pow(get<Number>(args.at(0)), get<Number>(args.at(1)));
    case Intern::op_square:
        return get<Number>(args.at(0)) * get<Number>(args.at(0));
    case Intern::op_log:
        return fun_log(args);
    case Intern::op_log10:
        return pscm::log10(get<Number>(args.at(0)));
    case Intern::op_sqrt:
        return pscm::sqrt(get<Number>(args.at(0)));
    case Intern::op_abs:
        return pscm::abs(get<Number>(args.at(0)));
    case Intern::op_real:
        return pscm::real(get<Number>(args.at(0)));
    case Intern::op_imag:
        return pscm::imag(get<Number>(args.at(0)));
    case Intern::op_arg:
        return pscm::arg(get<Number>(args.at(0)));
    case Intern::op_conj:
        return pscm::conj(get<Number>(args.at(0)));
    case Intern::op_rect:
        return pscm::rect(get<Number>(args.at(0)), get<Number>(args.at(1)));
    case Intern::op_polar:
        return pscm::polar(get<Number>(args.at(0)), get<Number>(args.at(1)));

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
        return is_type<Char>(args.at(0));
    case Intern::op_charint:
        return num(std::get<Char>(args.at(0)));

    /* Section 6.7: Strings */
    case Intern::op_isstr:
        return is_type<String>(args.at(0));

    /* Section 6.8: Vectors */
    case Intern::op_isvec:
        return is_type<VectorPtr>(args.at(0));
    case Intern::op_mkvec:
        return fun_make_vector(args);
    case Intern::op_vec:
        return std::make_shared<VectorPtr::element_type>(args);
    case Intern::op_veclen:
        return Number{ get<VectorPtr>(args.at(0))->size() };
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

    /* Section 6.11: Exceptions */
    /* Section 6.12: Environments and evaluation */
    case Intern::op_exit:
        return Intern::op_exit;

    /* Section 6.13: Input and output */
    case Intern::op_isport:
        return is_type<Port>(args.at(0));
    case Intern::op_isinport:
        return is_type<Port>(args.at(0)) && get<Port>(args[0]).is_input();
    case Intern::op_isoutport:
        return is_type<Port>(args.at(0)) && get<Port>(args[0]).is_output();
    case Intern::op_istxtport:
        return is_type<Port>(args.at(0)) && !get<Port>(args[0]).is_binary();
    case Intern::op_isbinport:
        return is_type<Port>(args.at(0)) && get<Port>(args[0]).is_binary();
    case Intern::op_isinport_open:
        return is_type<Port>(args.at(0)) && get<Port>(args[0]).is_input() && get<Port>(args[0]).is_open();
    case Intern::op_isoutport_open:
        return is_type<Port>(args.at(0)) && get<Port>(args[0]).is_output() && get<Port>(args[0]).is_open();
    case Intern::op_callw_infile:
        return fun_callw_infile(senv, get<String>(args.at(0)), args.at(1));
    case Intern::op_open_infile:
        return fun_open_infile(get<String>(args.at(0)));
    case Intern::op_open_outfile:
        return fun_open_outfile(get<String>(args.at(0)));
    case Intern::op_readline:
        return fun_readline(args);
    case Intern::op_read:
        return fun_read(args);
    case Intern::op_readchar:
        return fun_readchar(args);
    case Intern::op_peekchar:
        return fun_peekchar(args);
    case Intern::op_readstr:
        return fun_readstr(args);
    case Intern::op_eof:
        return Char{ EOF };
    case Intern::op_iseof:
        return is_type<Char>(args.at(0)) && get<Char>(args[0]) == EOF;

    default:
        throw std::invalid_argument("invalid primary operation");
    }
}
} // namespace pscm
