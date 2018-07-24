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

static Cell fun_booleq(const varg& args)
{
    argn(args, 2, ~size_t{ 0 });

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
    if (args.at(0) != args.at(1))
        return false;

    auto val = get<Number>(args[1]);
    for (auto iter = args.begin() + 2; iter != args.end(); val = get<Number>(*iter), ++iter)
        if (val != get<Number>(*iter))
            return false;

    return true;
}

static Cell fun_numlt(const varg& args)
{
    auto lhs = get<Number>(args.at(0)), rhs = get<Number>(args.at(1));

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
    auto lhs = get<Number>(args.at(0)), rhs = get<Number>(args.at(1));

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
    argn(args, 2, ~size_t{ 0 });

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
    argn(args, 2, ~size_t{ 0 });

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

/**
 * @brief Scheme @em list function.
 * @verbatim
 *   (list [arg_0 ... arg_n]) => nil | (arg_0 ... arg_n)
 * @endverbatim
 */
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

/**
 * @brief Scheme @em append function.
 * @verbatim
 *   (append [list_0 list_1 ... expr]) => nil | expr | (list_0 . list_1 ... . expr)
 * @endverbatim
 */
static Cell fun_append(const varg& args)
{
    if (!args.size())
        return nil;

    if (args.size() == 1)
        return args.front();

    Cell head = args.back(), tail = nil;

    // Append each list:
    for (size_t i = 0; i < args.size() - 1; ++i) {
        for (Cell list = args[i]; is_pair(list); list = cdr(list)) {
            if (is_nil(tail)) {
                head = cons(car(list), nil);
                tail = head;
            } else {
                set_cdr(tail, cons(car(list), nil));
                tail = cdr(tail);
            }
        }
    }
    // Append last expression
    if (is_pair(tail))
        set_cdr(tail, args.back());

    return head;
}

/**
 * @brief Scheme @em make-list function.
 * @verbatim
 *   (make-list len [fill = none]) => (fill ... fill)
 * @endverbatim
 */
static Cell fun_makelist(const varg& args)
{
    Int size = get<Int>(get<Number>(args.at(0)));
    if (size < 1)
        return nil;

    Cell val = none;
    if (args.size() > 1)
        val = args[1];

    Cell head = cons(val, nil);

    for (Cell tail = head; size > 1; --size, tail = cdr(tail))
        set_cdr(tail, cons(val, nil));

    return head;
}

/**
 * @brief Scheme @em reverse list function.
 * @verbatim
 *   (reverse (list 1 2 ... n)) => (n n-1 ... 2 1)
 * @endverbatim
 */
static Cell fun_reverse(const varg& args)
{
    Cell list = args.at(0), head = nil;

    for (/* */; is_pair(list); head = cons(car(list), head), list = cdr(list))
        ;

    return head;
}

/**
 * @brief Scheme inplace @em reverse! list function.
 */
static Cell fun_reverseb(const varg& args)
{
    Cell list = args.at(0), head = nil;

    for (Cell tail = list; is_pair(list); head = list, list = tail) {
        tail = cdr(tail);
        set_cdr(list, head);
    }
    return head;
}

/**
 * @brief Scheme @em list-ref function.
 * @verbatim
 *   (list-ref '(x0 x1 x2 ... xn) 2) => x2
 * @endverbatim
 */
static Cell fun_listref(const varg& args)
{
    Int k = get<Int>(get<Number>(args.at(1)));

    Cell list = args[0];
    for (/* */; k > 0 && is_pair(list); list = cdr(list), --k)
        ;

    (is_pair(list) && !k) || ((void)(throw std::invalid_argument("invalid list index")), 0);
    return car(list);
}

/**
 * @brief Scheme @em list-set! function.
 * @verbatim
 *   (list-set! '(x0 x1 x2 ... xn) 2 'z2) => (x0 x1 z2 ... xn)
 * @endverbatim
 */
static Cell fun_listsetb(const varg& args)
{
    Int k = get<Int>(get<Number>(args.at(1)));

    Cell list = args[0];
    for (/* */; k > 0 && is_pair(list); list = cdr(list), --k)
        ;

    (is_pair(list) && !k) || ((void)(throw std::invalid_argument("invalid list index")), 0);
    set_car(list, args.at(2));
    return none;
}

/**
 * @brief Scheme @em vector-ref function.
 * @verbatim
 *   (vector-ref #(x0 x1 x2 ... xn) 2) => x2)
 * @endverbatim
 */
static Cell fun_vector_ref(const varg& args)
{
    using size_type = VectorPtr::element_type::size_type;
    auto pos = static_cast<size_type>(get<Int>(get<Number>(args.at(1))));
    return get<VectorPtr>(args[0])->at(pos);
}

/**
 * @brief Scheme @em vector-set! function.
 * @verbatim
 *   (vector-set! #(x0 x1 x2 ... xn) 2 'z2) => #(x0 x1 z2 ... xn)
 * @endverbatim
 */
static Cell fun_vector_setb(const varg& args)
{
    using size_type = VectorPtr::element_type::size_type;
    auto pos = static_cast<size_type>(get<Int>(get<Number>(args.at(1))));
    get<VectorPtr>(args[0])->at(pos) = args.at(2);
    return none;
}

/**
 * @brief Scheme @em list->vector function.
 * @verbatim
 *   (list->vector '(x0 x1 x2 ... xn)) => #(x0 x1 x2 ... xn)
 * @endverbatim
 */
static Cell fun_list2vec(const varg& args)
{
    Cell list = args.at(0);
    VectorPtr v = std::make_shared<VectorPtr::element_type>();

    for (/* */; is_pair(list); list = cdr(list))
        v->push_back(car(list));

    is_nil(list) || ((void)(throw std::invalid_argument("not a proper list")), 0);
    return v;
}

/**
 * @brief Scheme @em vector->list function.
 * @verbatim
 *   (vector->list  #(x0 x1 x2 ... xn) [pos [end]]) => '(x0 x1 x2 ... xn)
 * @endverbatim
 */
static Cell fun_vec2list(const varg& args)
{
    using size_type = VectorPtr::element_type::difference_type;

    const VectorPtr& vec = get<VectorPtr>(args.at(0));
    size_type pos = 0, end = static_cast<size_type>(vec->size());

    if (args.size() > 1)
        pos = static_cast<size_type>(get<Int>(get<Number>(args[1])));
    if (args.size() > 2)
        end = std::min(static_cast<size_type>(get<Int>(get<Number>(args[2]))), end);

    if (!end)
        return nil;

    Cell list = cons(vec->at(pos), nil), tail = list;

    std::for_each(vec->begin() + pos + 1, vec->begin() + end, [&tail](const Cell& cell) {
        set_cdr(tail, cons(cell, nil));
        tail = cdr(tail);
    });
    return list;
}

/**
 * @brief Scheme @em vector-copy function.
 * @verbatim
 *   (vector-copy #(x0 x1 x2 ... xn) [pos [end]]) => #(x0 x1 x2 ... xn)
 * @endverbatim
 */
static Cell fun_vec_copy(const varg& args)
{
    using size_type = VectorPtr::element_type::difference_type;

    const VectorPtr& v = get<VectorPtr>(args.at(0));
    size_type pos = 0, end = static_cast<size_type>(v->size());

    if (args.size() > 1)
        pos = static_cast<size_type>(get<Int>(get<Number>(args[1])));
    if (args.size() > 2)
        end = std::min(static_cast<size_type>(get<Int>(get<Number>(args[2]))), end);

    return std::make_shared<VectorPtr::element_type>(v->begin() + pos, v->begin() + end);
}

/**
 * @brief Scheme inplace @em vector-copy! function.
 * @verbatim
 *   (vector-copy! vec-dest idx vec-source [pos [end]]) => vec-dest
 * @endverbatim
 */
static Cell fun_vec_copyb(const varg& args)
{
    using size_type = VectorPtr::element_type::difference_type;

    const VectorPtr& src = get<VectorPtr>(args.at(2));
    VectorPtr dst = get<VectorPtr>(args[0]);

    size_type idx = static_cast<size_type>(get<Int>(get<Number>(args[1]))),
              pos = 0, end = static_cast<size_type>(src->size());

    if (args.size() > 3)
        pos = static_cast<size_type>(get<Int>(get<Number>(args[3])));
    if (args.size() > 4)
        end = std::min(static_cast<size_type>(get<Int>(get<Number>(args[4]))), end);

    std::copy(src->begin() + pos, src->begin() + end, dst->begin() + idx);
    return dst;
}

/**
 * @brief Scheme inplace @em vector-fill! function.
 * @verbatim
 *   (vector-fill! vec value [pos [end]]) => vec
 * @endverbatim
 */
static Cell fun_vec_fillb(const varg& args)
{
    using size_type = VectorPtr::element_type::difference_type;

    VectorPtr vec = get<VectorPtr>(args.at(0));
    size_type pos = 0, end = static_cast<size_type>(vec->size());

    if (args.size() > 2)
        pos = static_cast<size_type>(get<Int>(get<Number>(args[2])));
    if (args.size() > 3)
        end = std::min(static_cast<size_type>(get<Int>(get<Number>(args[3]))), end);

    std::fill(vec->begin() + pos, vec->end() + end, args.at(1));
    return vec;
}

/**
 * @brief Scheme inplace @em vector-append function.
 * @verbatim
 *   (vector-append vec_0 vec_1 ... vec_n) => vec := {vec_0, vec_1, ..., vec_n}
 * @endverbatim
 */
static Cell fun_vec_append(const varg& args)
{
    VectorPtr vec = get<VectorPtr>(args.at(0));

    for (const Cell& cell : args) {
        const VectorPtr& v = get<VectorPtr>(cell);

        std::copy(v->begin(), v->end(), std::back_inserter(*vec));
    }
    return vec;
}

static Cell fun_callw_infile(const Symenv& senv, const String& filnam, const Cell& proc)
{
    Port port;
    port.open(*filnam, std::ios_base::in)
        || ((void)(throw std::invalid_argument("could not open port")), 0);

    Cons cons[4];
    return eval(senv, alist(cons, Intern::_apply, proc, port, nil));
}

static Cell fun_open_infile(const String& filnam)
{
    Port port;
    port.open(*filnam, std::ios_base::in)
        || ((void)(throw std::invalid_argument("could not open port")), 0);

    return port;
}

static Cell fun_open_outfile(const String& filnam)
{
    Port port;

    port.open(*filnam, std::ios_base::out)
        || ((void)(throw std::invalid_argument("could not open port")), 0);

    return port;
}

static Cell fun_readline(const varg& args)
{
    argn(args, 0, 1);
    String line = str("");

    if (args.size() > 0) {
        Port port = get<Port>(args[0]);
        (port.is_open() && port.is_input())
            || ((void)(throw std::invalid_argument("port is closed")), 0);

        std::getline(port.stream(), *line);
    } else {
        std::string str;
        std::getline(std::cin, str);
    }
    return line;
}

static Cell fun_read(const varg& args)
{
    argn(args, 0, 1);
    Port port;

    if (args.size() > 0) {
        port = get<Port>(args[0]);
        (port.is_open() && port.is_input())
            || ((void)(throw std::invalid_argument("port is closed")), 0);
    }
    Parser parser;
    return parser.parse(port.stream());
}

static Cell fun_readchar(const varg& args)
{
    argn(args, 0, 1);
    Port port;

    if (args.size() > 0) {
        port = get<Port>(args[0]);
        (port.is_open() && port.is_input())
            || ((void)(throw std::invalid_argument("port is closed")), 0);
    }
    return static_cast<Char>(port.stream().get());
}

static Cell fun_peekchar(const varg& args)
{
    argn(args, 0, 1);
    Port port;

    if (args.size() > 0) {
        port = get<Port>(args[0]);
        (port.is_open() && port.is_input())
            || ((void)(throw std::invalid_argument("port is closed")), 0);
    }
    return static_cast<Char>(port.stream().peek());
}

static Cell fun_readstr(const varg& args)
{
    argn(args, 1, 2);
    Port port;

    Number num = get<Number>(args[0]);
    if (is_int(num) || is_negative(num))
        throw std::invalid_argument("must be a nonnegative number");

    if (args.size() > 1) {
        port = get<Port>(args[0]);
        (port.is_open() && port.is_input())
            || ((void)(throw std::invalid_argument("port is closed")), 0);
    }
    Parser parser;
    return parser.parse(port.stream());
}

static Cell fun_macroexp(const Symenv& senv, const varg& args)
{
    Cell expr = args.at(0);

    if (!is_pair(expr))
        return expr;

    Cell proc = eval(senv, car(expr));
    if (!is_macro(proc))
        return expr;

    return get<Proc>(proc).expand(expr);
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
        return (void)(argn(args, 1)), !is_true(args[0]);
    case Intern::op_isbool:
        return (void)(argn(args, 1)), is_bool(args[0]);
    case Intern::op_isbooleq:
        return fun_booleq(args);

    /* Section 6.4: Pair and lists */
    case Intern::op_cons:
        return (void)(argn(args, 2)), cons(args[0], args[1]);
    case Intern::op_car:
        return (void)(argn(args, 1)), car(args[0]);
    case Intern::op_cdr:
        return (void)(argn(args, 1)), cdr(args[0]);
    case Intern::op_caar:
        return (void)(argn(args, 1)), caar(args[0]);
    case Intern::op_cddr:
        return (void)(argn(args, 1)), cddr(args[0]);
    case Intern::op_cadr:
        return (void)(argn(args, 1)), cadr(args[0]);
    case Intern::op_cdar:
        return (void)(argn(args, 1)), cdar(args[0]);
    case Intern::op_setcar:
        return (void)(argn(args, 2)), (void)(set_car(args[0], args[1])), none;
    case Intern::op_setcdr:
        return (void)(argn(args, 2)), (void)(set_cdr(args[0], args[1])), none;
    case Intern::op_list:
        return fun_list(args);
    case Intern::op_mklist:
        return fun_makelist(args);
    case Intern::op_ispair:
        return is_pair(args.at(0));
    case Intern::op_islist:
        return is_list(args.at(0));
    case Intern::op_append:
        return fun_append(args);
    case Intern::op_length:
        return Number{ list_length(args.at(0)) };
    case Intern::op_listref:
        return fun_listref(args);
    case Intern::op_listsetb:
        return fun_listsetb(args);
    case Intern::op_reverse:
        return fun_reverse(args);
    case Intern::op_reverseb:
        return fun_reverseb(args);

    /* Section 6.5: Symbols */
    case Intern::op_issym:
        return is_symbol(args.at(0));
    case Intern::op_symstr:
        return std::make_shared<String::element_type>(get<Symbol>(args.at(0)).value());
    case Intern::op_strsym:
        return sym(get<String>(args.at(0))->c_str());

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
        return args.size() > 1 ? vec(args[0], args[1]) : vec(args.at(0), none);
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
        return (void)(argn(args, 1)), is_proc(args[0]) || (is_intern(args[0]) && std::get<Intern>(args[0]) >= Intern::op_eq);

    /* Section 6.11: Exceptions */
    /* Section 6.12: Environments and evaluation */
    case Intern::op_exit:
        return Intern::op_exit;
    case Intern::op_replenv:
        return senv;
    case Intern::op_eval:
        return eval(args.size() > 1 ? get<Symenv>(args[1]) : senv, args[0]);
    case Intern::op_macroexp:
        return fun_macroexp(senv, args);

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
