/*********************************************************************************/ /**
 * @file primop.cpp
 *
 * Implementations of some scheme functions as defined in the r7rs
 * reference document.
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#include <algorithm>
#include <cctype>
#include <iostream>
#include <memory>

#include "gc.hpp"
#include "parser.hpp"
#include "primop.hpp"
#include "procedure.hpp"
#include "scheme.hpp"

using varg = std::vector<pscm::Cell>;

namespace pscm::primop {

using namespace std::string_literals;

/**
 * Scheme @em boolean=? predicate function
 */
static Cell booleq(const varg& args)
{
    if (!is_bool(args.at(0)))
        return false;

    Bool prv = get<Bool>(args[0]), b;

    for (auto iter = args.begin() + 1; iter != args.end(); prv = b, ++iter)
        if (!is_bool(*iter) || prv != (b = get<Bool>(*iter)))
            return false;

    return true;
}

/**
 * Scheme number equality = predicate function.
 */
static Cell numeq(const varg& args)
{
    if (args.at(0) != args.at(1))
        return false;

    auto val = get<Number>(args[1]);
    for (auto iter = args.begin() + 2; iter != args.end(); val = get<Number>(*iter), ++iter)
        if (val != get<Number>(*iter))
            return false;

    return true;
}

/**
 * Scheme number lower then < predicate function
 */
static Cell numlt(const varg& args)
{
    auto lhs = get<Number>(args.at(0)), rhs = get<Number>(args.at(1));

    if (!(lhs < rhs))
        return false;

    for (auto iter = args.begin() + 2; iter != args.end(); ++iter) {
        lhs = rhs;
        rhs = get<Number>(static_cast<Cell>(*iter));

        if (!(lhs < rhs))
            return false;
    }
    return true;
}

/**
 * Scheme number greater then > predicate function.
 */
static Cell numgt(const varg& args)
{
    auto lhs = get<Number>(args.at(0)), rhs = get<Number>(args.at(1));

    if (!(lhs > rhs))
        return false;

    for (auto iter = args.begin() + 2; iter != args.end(); ++iter) {
        lhs = rhs;
        rhs = get<Number>(static_cast<Cell>(*iter));

        if (!(lhs > rhs))
            return false;
    }
    return true;
}

/**
 * Scheme number lower equal <= predicate function.
 */
static Cell numle(const varg& args)
{
    auto rhs = get<Number>(args.at(1)), lhs = get<Number>(args[0]);

    if (!(lhs <= rhs))
        return false;

    for (auto iter = args.begin() + 2; iter != args.end(); ++iter) {
        lhs = rhs;
        rhs = get<Number>(*iter);

        if (!(lhs <= rhs))
            return false;
    }
    return true;
}

/**
 * Scheme number greater equal >= predicate function.
 */
static Cell numge(const varg& args)
{
    auto rhs = get<Number>(args.at(1)), lhs = get<Number>(args[0]);

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

/**
 * Scheme number max function.
 */
static Cell max(const varg& args)
{
    Number res = pscm::max(get<Number>(args.at(0)), get<Number>(args.at(1)));

    for (auto ip = args.begin() + 2, ie = args.end(); ip != ie; ++ip)
        res = pscm::max(res, get<Number>(*ip));

    return res;
}

/**
 * Scheme number min function.
 */
static Cell min(const varg& args)
{
    Number res = pscm::min(get<Number>(args.at(0)), get<Number>(args.at(1)));

    for (auto ip = args.begin() + 2, ie = args.end(); ip != ie; ++ip)
        res = pscm::min(res, get<Number>(*ip));

    return res;
}

/**
 * Scheme number addition + operator function.
 */
static Cell add(const varg& args)
{
    Number res = Int{ 0 };

    for (auto& val : args)
        res += get<Number>(val);

    return res;
}

/**
 * Scheme number substraction - operator function.
 */
static Cell sub(const varg& args)
{
    auto res = args.size() > 1 ? get<Number>(args.at(0)) : -get<Number>(args.at(0));

    for (auto ip = args.begin() + 1, ie = args.end(); ip != ie; ++ip)
        res -= get<Number>(*ip);

    return res;
}
/**
 * Scheme number multiplication * operator function.
 */
static Cell mul(const varg& args)
{
    Number res = 1;

    for (const Cell& val : args)
        res *= get<Number>(val);

    return res;
}

/**
 * Scheme number division / operator function.
 */
static Cell div(const varg& args)
{
    auto res = args.size() > 1 ? get<Number>(args[0]) : inv(get<Number>(args.at(0)));

    for (auto iter = ++args.begin(); iter != args.end(); ++iter)
        res /= get<Number>(*iter);

    return res;
}

/**
 * Scheme logarithm @em log function.
 */
static Cell log(const varg& args)
{
    if (args.size() < 2)
        return pscm::log(get<Number>(args.at(0)));
    else {
        const Number &y = get<Number>(args.at(1)), &x = get<Number>(args[0]);

        return y != Number{ 10 } ? pscm::log(x) / pscm::log(y)
                                 : pscm::log10(x);
    }
}

/**
 * Scheme hypothenuse functions.
 */
static Cell hypot(const varg& args)
{
    return args.size() > 2 ? pscm::hypot(get<Number>(args[0]),
                                 get<Number>(args[1]),
                                 get<Number>(args[2]))
                           : pscm::hypot(get<Number>(args.at(0)),
                                 get<Number>(args.at(1)));
}

static Cell ex2inex(const Cell& cell)
{
    auto& num = get<Number>(cell);
    return is_type<Int>(num) ? Number{ static_cast<Float>(get<Int>(num)) } : num;
}

static Cell inex2ex(const Cell& cell)
{
    auto& num = get<Number>(cell);

    if (is_type<Complex>(num) && !is_zero(imag(num)))
        throw std::invalid_argument("inexact->exact - invalid cast for complex number");

    return is_type<Int>(num) ? num
                             : is_type<Float>(num) ? Number{ static_cast<Int>(get<Float>(num)) }
                                                   : Number{ static_cast<Int>(get<Float>(real(num))) };
}

static Cell numstr(const varg& args)
{
    std::basic_ostringstream<Char> buf;
    buf << get<Number>(args.at(0));
    return std::make_shared<StringPtr::element_type>(buf.str());
}

/**
 * Scheme list @em append function.
 * @verbatim (append [list_0 list_1 ... expr]) => nil | expr | (list_0 . list_1 ... . expr) @endverbatim
 */
static Cell append(Scheme& scm, const varg& args)
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
                head = scm.cons(car(list), nil);
                tail = head;
            } else {
                set_cdr(tail, scm.cons(car(list), nil));
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
 * Scheme @em list function.
 * @verbatim (list [arg_0 ... arg_n]) => nil | (arg_0 ... arg_n) @endverbatim
 */
static Cell list(Scheme& scm, const varg& args)
{
    Cell list = nil;
    if (args.size()) {
        list = scm.cons(args.front(), nil);

        Cell tail = list;
        for (auto iter = ++args.begin(); iter != args.end(); ++iter, tail = cdr(tail))
            set_cdr(tail, scm.cons(*iter, nil));
    }
    return list;
}

/**
 * Scheme @em make-list function.
 * @verbatim (make-list len [fill = none]) => (fill ... fill) @endverbatim
 */
static Cell makelist(Scheme& scm, const varg& args)
{
    Int size = get<Int>(get<Number>(args.at(0)));
    if (size < 1)
        return nil;

    Cell val = none;
    if (args.size() > 1)
        val = args[1];

    Cell head = scm.cons(val, nil);

    for (Cell tail = head; size > 1; --size, tail = cdr(tail))
        set_cdr(tail, scm.cons(val, nil));

    return head;
}

/**
 * Scheme @em reverse list function.
 * @verbatim (reverse (list 1 2 ... n)) => (n n-1 ... 2 1) @endverbatim
 */
static Cell reverse(Scheme& scm, const varg& args)
{
    Cell list = args.at(0), head = nil;

    for (/* */; is_pair(list); head = scm.cons(car(list), head), list = cdr(list))
        ;

    return head;
}

/**
 * Scheme inplace @em reverse! list function.
 */
static Cell reverseb(const varg& args)
{
    Cell list = args.at(0), head = nil;

    for (Cell tail = list; is_pair(list); head = list, list = tail) {
        tail = cdr(tail);
        set_cdr(list, head);
    }
    return head;
}

/**
 * Scheme @em list-ref function.
 * @verbatim (list-ref '(x0 x1 x2 ... xn) 2) => x2 @endverbatim
 */
static Cell listref(const varg& args)
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
 * @verbatim (list-set! '(x0 x1 x2 ... xn) 2 'z2) => (x0 x1 z2 ... xn) @endverbatim
 */
static Cell listsetb(const varg& args)
{
    Int k = get<Int>(get<Number>(args.at(1)));

    Cell list = args[0];
    for (/* */; k > 0 && is_pair(list); list = cdr(list), --k)
        ;

    (is_pair(list) && !k) || ((void)(throw std::invalid_argument("invalid list index")), 0);
    set_car(list, args.at(2));
    return none;
}

static Cell listcopy(Scheme& scm, const varg& args)
{
    Cell list = args.at(0);

    if (is_nil(list))
        return nil;

    Cell head = scm.cons(car(list), nil), tail = head;

    for (list = cdr(list); is_pair(list); list = cdr(list), tail = cdr(tail))
        set_cdr(tail, scm.cons(car(list), nil));

    if (!is_nil(list))
        set_cdr(tail, list);

    return head;
}

static Cell is_proc(const varg& args)
{
    const Cell& cell = args.at(0);
    return pscm::is_proc(cell) || pscm::is_func(cell)
        || (is_intern(cell) && get<Intern>(cell) >= Intern::_apply);
}

static Cell apply(Scheme& scm, const SymenvPtr& senv, const Cell& proc, const varg& args = varg{})
{
    if (pscm::is_proc(proc)) {
        Cons arg[2], cns[4];
        Cell argv = pscm::list(arg, Intern::_quote, nil);
        Cell expr = pscm::list(cns, Intern::_apply, proc, argv, nil);

        if (args.empty()) {
            set_cdr(cddr(expr), nil); // terminate expr-list after proc
            return scm.eval(senv, expr);

        } else if (args.size() == 1) {
            set_car(cdr(argv), args.front());
            return scm.eval(senv, expr);

        } else {
            std::vector<Cons> vec;
            vec.reserve(args.size());
            Cell head = cons(vec, args.front(), nil), tail = head;

            for (auto ip = args.begin() + 1, ie = args.end(); ip != ie; ++ip, tail = cdr(tail))
                set_cdr(tail, cons(vec, *ip, nil));

            set_car(cdr(argv), head);
            set_cdr(cddr(expr), nil);
            return scm.eval(senv, expr);
        }
    } else
        return scm.apply(senv, proc, args);
}

static Cell apply(Scheme& scm, const SymenvPtr& senv, const varg& args)
{
    if (args.size() <= 1)
        throw std::invalid_argument("apply - invalid number of arguments");

    varg arg;
    for (auto ip = args.begin() + 1, ie = args.end() - 1; ip != ie; ++ip)
        arg.push_back(*ip);

    for (Cell list = args.back(); is_pair(list); list = cdr(list))
        arg.push_back(car(list));

    return apply(scm, senv, args.at(0), arg);
}

static Cell vec2list(Scheme& scm, const varg& args);

/**
 * Call with current continuation.
 *
 * Simple implementation as escape continuation
 */
static Cell callcc(Scheme& scm, const SymenvPtr& senv, const varg& args)
{
    struct continuation_exception {
        continuation_exception(Scheme& scm, const varg& args)
        {
            if (args.empty())
                return;

            continuation = args.size() != 1 ? primop::list(scm, args) : args[0];
        }
        Cell continuation = none;
    };
    auto lambda = [](Scheme& scm, const SymenvPtr&, const varg& args) -> Cell {
        throw continuation_exception{ scm, args };
        return none;
    };

    try {
        return primop::apply(scm, senv, args.at(0), varg{ scm.function(senv, std::move(lambda)) });
    } catch (const continuation_exception& e) {
        return e.continuation;
    }
}

static Cell callwval(Scheme& scm, const SymenvPtr& senv, const varg& args)
{
    struct callwval_exception : std::exception {
        callwval_exception(Scheme& scm, const SymenvPtr& senv, varg args = varg{})
            : senv{ senv }
            , args{ std::move(args) }
        {
            auto values = [this](Scheme& scm, const SymenvPtr&, const varg& args) -> Cell {
                if (this->args.empty())
                    throw callwval_exception{ scm, this->senv, args };
                return primop::list(scm, args);
            };
            scm.function(senv, "values", std::move(values));
        }
        SymenvPtr senv;
        varg args;
    };

    try {
        callwval_exception exc{ scm, senv };
        return primop::apply(scm, senv, args.at(0));
    } catch (const callwval_exception& e) {
        return primop::apply(scm, senv, args.at(1), e.args);
    }
}

struct scheme_exception : std::exception {
    scheme_exception(Scheme& scm, SymenvPtr senv, varg args)
        : senv{ std::move(senv) }
        , args{ std::move(args) }
    {
        auto raise = [](Scheme& scm, const SymenvPtr& senv, const varg& args) -> Cell {
            if (args.size() != 1)
                throw std::invalid_argument("raise requires exact one argument");

            throw scheme_exception{ scm, senv, args };
            return none;
        };
        auto raisecont = [this](Scheme& scm, const SymenvPtr& senv, const varg& args) -> Cell {
            if (args.size() != 1)
                throw std::invalid_argument("raise requires exact one argument");

            auto& proc = get<Procedure>(this->args.at(0));
            return primop::apply(scm, senv, proc, args);
        };
        auto error = [](Scheme& scm, const SymenvPtr& senv, const varg& args) -> Cell {
            if (args.size() < 2)
                throw std::invalid_argument("error requires at least two arguments");

            if (!is_string(args[0]))
                throw std::invalid_argument("error requires a message string as first argument");

            throw scheme_exception{ scm, senv, args };
            return none;
        };
        auto is_errobj = [](Scheme&, const SymenvPtr&, const varg& args) -> Cell {
            return is_pair(args.at(0)) && is_string(car(args[0])) && is_pair(cdr(args[0]));
        };
        auto errmsg = [is_errobj](Scheme& scm, const SymenvPtr& env, const varg& args) -> Cell {
            if (!get<Bool>(is_errobj(scm, env, args)))
                throw std::invalid_argument("argument is not an error object");

            return car(args[0]);
        };
        auto errirr = [is_errobj](Scheme& scm, const SymenvPtr& env, const varg& args) -> Cell {
            if (!get<Bool>(is_errobj(scm, env, args)))
                throw std::invalid_argument("argument is not an error object");
            return cdr(args[0]);
        };
        // clang-format off
        scm.function(senv, "raise",                  std::move(raise));
        scm.function(senv, "raise-continuable",      std::move(raisecont));
        scm.function(senv, "error",                  std::move(error));
        scm.function(senv, "error-object?",          std::move(is_errobj));
        scm.function(senv, "error-object-message",   std::move(errmsg));
        scm.function(senv, "error-object-irritants", std::move(errirr));
        // clang-format on
    }
    Cell apply(Scheme& scm, const SymenvPtr&, const Cell& proc) const
    {
        // handle exception by (error <msg> <obj0> [<obj1> ...])
        return args.size() != 1 ? primop::apply(scm, this->senv, proc, varg{ primop::list(scm, args) })
                                // handle exception by (raise obj)
                                : primop::apply(scm, this->senv, proc, args);
    }
    SymenvPtr senv;
    varg args;
};
/**
 * Scheme function @em with-exception-handler
 * (with-exception-handler <handler> <thunk>)
 * (handler <obj>)
 *
 * Example:
 * @verbatim
 * (with-exception-handler ...
 * @endverbatim
 */
static Cell withexcept(Scheme& scm, const SymenvPtr& senv, const varg& args)
{
    try {
        scheme_exception exc{ scm, senv, args };
        return primop::apply(scm, senv, args.at(1));

    } catch (const scheme_exception& e) {
        return e.apply(scm, senv, args.at(0));
    }
}

static Cell error(Scheme& scm, const SymenvPtr& senv, const varg& args)
{
    if (args.size() < 2 || !is_string(args[0]))
        throw std::invalid_argument("invalid number of arguments or not a message string");

    throw scheme_exception{ scm, senv, args };
    return none;
}

/**
 * Scheme list @em member function.
 */
static Cell memq(const SymenvPtr&, const varg& args)
{
    Cell list = args.at(1);
    const Cell& obj = args.front();

    for (; is_pair(list); list = cdr(list))
        if (obj == car(list))
            return list;

    is_nil(list)
        || ((void)(throw std::invalid_argument("invalid argument list")), 0);

    return false;
}

/**
 * Scheme list @em member function.
 */
static Cell member(Scheme scm, const SymenvPtr& senv, const varg& args)
{
    Cell list = args.at(1);
    const Cell& obj = args.front();

    if (args.size() > 2) {
        const Cell& proc = args[2];
        varg argv = { obj, none };

        for (; is_pair(list); list = cdr(list)) {
            argv.back() = car(list);
            if (is_true(apply(scm, senv, proc, argv)))
                return list;
        }

    } else
        for (; is_pair(list); list = cdr(list))
            if (is_equal(obj, car(list)))
                return list;

    is_nil(list)
        || ((void)(throw std::invalid_argument("member - invalid argument list")), 0);

    return false;
}

/**
 * Scheme list @em assq function.
 */
static Cell assq(const SymenvPtr&, const varg& args)
{
    Cell list = args.at(1);
    const Cell& obj = args.front();

    for (; is_pair(list); list = cdr(list)) {
        if (!is_pair(car(list)))
            break;

        if (obj == caar(list))
            return car(list);
    }

    is_nil(list)
        || ((void)(throw std::invalid_argument("not an association list")), 0);

    return false;
}

/**
 * Scheme list @em assoc function.
 */
static Cell assoc(Scheme& scm, const SymenvPtr& senv, const varg& args)
{
    Cell list = args.at(1);
    const Cell& obj = args.front();

    if (args.size() > 2) {
        const Cell& proc = args[2];
        varg argv = { obj, none };

        for (; is_pair(list); list = cdr(list)) {
            if (is_pair(car(list)))
                break;

            argv.back() = caar(list);
            if (is_true(apply(scm, senv, proc, argv)))
                return car(list);
        }

    } else
        for (; is_pair(list); list = cdr(list)) {
            if (!is_pair(car(list)))
                break;

            if (is_equal(obj, caar(list)))
                return car(list);
        }

    is_nil(list)
        || ((void)(throw std::invalid_argument("assoc - invalid argument list")), 0);

    return false;
}

/**
 * Scheme char=? function.
 */
static Cell ischareq(const varg& args)
{
    Char c = get<Char>(args.at(0));

    for (auto ip = args.begin() + 1, ie = args.end(); ip != ie; ++ip)
        if (c != get<Char>(*ip))
            return false;
    return true;
}

/**
 * Scheme char<? function.
 */
static Cell ischarlt(const varg& args)
{
    Char c = get<Char>(args.at(0));

    for (auto ip = args.begin() + 1, ie = args.end(); ip != ie; ++ip)
        if (c >= get<Char>(*ip))
            return false;
    return true;
}

/**
 * Scheme char>? function.
 */
static Cell ischargt(const varg& args)
{
    Char c = get<Char>(args.at(0));

    for (auto ip = args.begin() + 1, ie = args.end(); ip != ie; ++ip)
        if (c <= get<Char>(*ip))
            return false;
    return true;
}

/**
 * Scheme char<=? function.
 */
static Cell ischarle(const varg& args)
{
    Char c = get<Char>(args.at(0));

    for (auto ip = args.begin() + 1, ie = args.end(); ip != ie; ++ip)
        if (c > get<Char>(*ip))
            return false;
    return true;
}

/**
 * Scheme char>=? function.
 */
static Cell ischarge(const varg& args)
{
    Char c = get<Char>(args.at(0));

    for (auto ip = args.begin() + 1, ie = args.end(); ip != ie; ++ip)
        if (c < get<Char>(*ip))
            return false;
    return true;
}

/**
 * Scheme char-ci=? function.
 */
static Cell ischcieq(const varg& args)
{
    Char ci, c = get<Char>(args.at(0));

    if (std::isalpha(c))
        c = std::tolower(c);

    for (auto ip = args.begin() + 1, ie = args.end(); ip != ie; ++ip) {
        ci = get<Char>(*ip);

        if (std::isalpha(ci))
            ci = std::tolower(ci);

        if (c != ci)
            return false;
    }
    return true;
}

/**
 * Scheme char-ci<? function.
 */
static Cell ischcilt(const varg& args)
{
    Char ci, c = get<Char>(args.at(0));

    if (std::isalpha(c))
        c = std::tolower(c);

    for (auto ip = args.begin() + 1, ie = args.end(); ip != ie; ++ip) {
        ci = get<Char>(*ip);

        if (std::isalpha(ci))
            ci = std::tolower(ci);

        if (c >= ci)
            return false;
    }
    return true;
}

/**
 * Scheme char-ci>? function.
 */
static Cell ischcigt(const varg& args)
{
    Char ci, c = get<Char>(args.at(0));

    if (std::isalpha(c))
        c = std::tolower(c);

    for (auto ip = args.begin() + 1, ie = args.end(); ip != ie; ++ip) {
        ci = get<Char>(*ip);

        if (std::isalpha(ci))
            ci = std::tolower(ci);

        if (c <= ci)
            return false;
    }
    return true;
}

/**
 * Scheme char-ci<=? function.
 */
static Cell ischcile(const varg& args)
{
    Char ci, c = get<Char>(args.at(0));

    if (std::isalpha(c))
        c = std::tolower(c);

    for (auto ip = args.begin() + 1, ie = args.end(); ip != ie; ++ip) {
        ci = get<Char>(*ip);

        if (std::isalpha(ci))
            ci = std::tolower(ci);

        if (c > ci)
            return false;
    }
    return true;
}

/**
 * Scheme char-ci>=? function.
 */
static Cell ischcige(const varg& args)
{
    Char ci, c = get<Char>(args.at(0));

    if (std::isalpha(c))
        c = std::tolower(c);

    for (auto ip = args.begin() + 1, ie = args.end(); ip != ie; ++ip) {
        ci = get<Char>(*ip);

        if (std::isalpha(ci))
            ci = std::tolower(ci);

        if (c < ci)
            return false;
    }
    return true;
}

/**
 * Scheme digit-value function.
 */
static Cell digitval(const varg& args)
{
    const Char c = get<Char>(args.at(0)), c0 = '0';

    if (std::isdigit(c))
        return Number{ static_cast<Int>(c - c0) };
    return false;
}
/**
 * Scheme @em make-string function.
 * (make-string len [char])
 */
static Cell mkstring(const varg& args)
{
    Int size = get<Int>(get<Number>(args.at(0)));
    size >= 0 || ((void)(throw std::invalid_argument("invalid negative number")), 0);

    Char c = ' ';
    if (args.size() > 1)
        c = get<Char>(args[1]);

    return std::make_shared<StringPtr::element_type>(size, c);
}

/**
 * Scheme string function.
 */
static Cell string(const varg& args)
{
    StringPtr pstr = std::make_shared<StringPtr::element_type>();
    pstr->reserve(args.size());

    for (auto& cell : args)
        pstr->push_back(get<Char>(cell));

    return pstr;
}

/**
 * Scheme string->list function.
 */
static Cell strlist(Scheme& scm, const varg& args)
{
    const auto& pstr = get<StringPtr>(args.at(0));

    if (pstr->empty())
        return nil;

    Int pos = 0, end = static_cast<Int>(pstr->length());

    if (args.size() > 2)
        end = std::min(get<Int>(get<Number>(args[2])), end);

    if (args.size() > 1)
        pos = std::min(get<Int>(get<Number>(args[1])), end);

    Cell head = scm.cons(pstr->at(pos), nil), tail = head;

    for (auto ip = pstr->begin() + pos + 1, ie = pstr->begin() + end; ip != ie; ++ip, tail = cdr(tail))
        set_cdr(tail, scm.cons(*ip, nil));

    return head;
}

/**
 * Scheme list->string function.
 */
static Cell liststr(const varg& args)
{
    Cell list = args.at(0);

    auto pstr = std::make_shared<StringPtr::element_type>();

    if (is_nil(list))
        return pstr;

    for (/* */; is_pair(list); list = cdr(list))
        pstr->push_back(get<Char>(car(list)));

    is_nil(list)
        || ((void)(throw std::invalid_argument("list->string - not a proper list")), 0);

    return pstr;
}

/**
 * Scheme @em string=? function.
 */
static Cell isstreq(const varg& args)
{
    const auto& sptr = get<StringPtr>(args.at(0));

    for (auto ip = args.begin() + 1, ie = args.end(); ip != ie; ++ip)
        if (*sptr != *get<StringPtr>(*ip))
            return false;

    return true;
}

/**
 * Scheme @em string<? function.
 */
static Cell isstrlt(const varg& args)
{
    auto sptr = get<StringPtr>(args.at(0));

    for (auto ip = args.begin() + 1, ie = args.end(); ip != ie; sptr = get<StringPtr>(*ip), ++ip) {
        if (*sptr >= *get<StringPtr>(*ip))
            return false;
    }

    return true;
}

/**
 * Scheme @em string>? function.
 */
static Cell isstrgt(const varg& args)
{
    auto sptr = get<StringPtr>(args.at(0));

    for (auto ip = args.begin() + 1, ie = args.end(); ip != ie; sptr = get<StringPtr>(*ip), ++ip) {
        if (*sptr <= *get<StringPtr>(*ip))
            return false;
    }
    return true;
}

/**
 * Scheme @em string<=? function.
 */
static Cell isstrle(const varg& args)
{
    auto sptr = get<StringPtr>(args.at(0));

    for (auto ip = args.begin() + 1, ie = args.end(); ip != ie; sptr = get<StringPtr>(*ip), ++ip) {
        if (*sptr > *get<StringPtr>(*ip))
            return false;
    }
    return true;
}

/**
 * Scheme @em string>=? function.
 */
static Cell isstrge(const varg& args)
{
    auto sptr = get<StringPtr>(args.at(0));

    for (auto ip = args.begin() + 1, ie = args.end(); ip != ie; sptr = get<StringPtr>(*ip), ++ip) {
        if (*sptr < *get<StringPtr>(*ip))
            return false;
    }
    return true;
}

/**
 * Scheme @em string-ci=? function.
 */
static Cell isstrcieq(const varg& args)
{
    const auto& sptr = get<StringPtr>(args.at(0));

    auto len = sptr->length();

    for (auto ip = args.begin() + 1, ie = args.end(); ip != ie; ++ip) {
        const auto& sp = get<StringPtr>(*ip);

        if (len != sp->length())
            return false;

        if (!equal(sptr->begin(), sptr->end(), sp->begin(),
                [](auto c0, auto c1) -> bool { return std::tolower(c0) == std::tolower(c1); }))
            return false;
    }
    return true;
}

/**
 * Scheme @em string-ci<? function.
 */
static Cell isstrcilt(const varg& args)
{
    auto sptr = get<StringPtr>(args.at(0));

    for (auto ip = args.begin() + 1, ie = args.end(); ip != ie; sptr = get<StringPtr>(*ip), ++ip) {
        const auto& sp = get<StringPtr>(*ip);

        if (sptr->length() < sp->length()) {
            if (!equal(sptr->begin(), sptr->end(), sp->begin(),
                    [](auto c0, auto c1) -> bool { return std::tolower(c0) <= std::tolower(c1); }))
                return false;
        } else if (sptr->length() == sp->length()) {
            if (sptr->empty())
                return false;

            if (!equal(sptr->begin(), sptr->end(), sp->begin(),
                    [](auto c0, auto c1) -> bool { return std::tolower(c0) < std::tolower(c1); }))
                return false;

        } else {
            if (!equal(sp->begin(), sp->end(), sptr->begin(),
                    [](auto c0, auto c1) -> bool { return std::tolower(c0) > std::tolower(c1); }))
                return false;
        }
    }
    return true;
}

/**
 * Scheme @em string-ci>? function.
 */
static Cell isstrcigt(const varg& args)
{
    auto sptr = get<StringPtr>(args.at(0));

    for (auto ip = args.begin() + 1, ie = args.end(); ip != ie; sptr = get<StringPtr>(*ip), ++ip) {
        const auto& sp = get<StringPtr>(*ip);

        if (sptr->length() > sp->length()) {
            if (!equal(sp->begin(), sp->end(), sptr->begin(),
                    [](auto c0, auto c1) -> bool { return std::tolower(c0) <= std::tolower(c1); }))
                return false;

        } else if (sptr->length() == sp->length()) {
            if (sptr->empty())
                return false;

            if (!equal(sptr->begin(), sptr->end(), sp->begin(),
                    [](auto c0, auto c1) -> bool { return std::tolower(c0) > std::tolower(c1); }))
                return false;
        } else {
            if (!equal(sptr->begin(), sptr->end(), sp->begin(),
                    [](auto c0, auto c1) -> bool { return std::tolower(c0) > std::tolower(c1); }))
                return false;
        }
    }
    return true;
}

/**
 * Scheme @em string-ci<=? function.
 */
static Cell isstrcile(const varg& args)
{
    auto sptr = get<StringPtr>(args.at(0));

    for (auto ip = args.begin() + 1, ie = args.end(); ip != ie; sptr = get<StringPtr>(*ip), ++ip) {
        const auto& sp = get<StringPtr>(*ip);

        if (sptr->length() <= sp->length()) {
            if (!equal(sptr->begin(), sptr->end(), sp->begin(),
                    [](auto c0, auto c1) -> bool { return std::tolower(c0) <= std::tolower(c1); }))
                return false;
        } else {
            if (!equal(sp->begin(), sp->end(), sptr->begin(),
                    [](auto c0, auto c1) -> bool { return std::tolower(c0) > std::tolower(c1); }))
                return false;
        }
    }
    return true;
}

/**
 * Scheme @em string-ci>=? function.
 */
static Cell isstrcige(const varg& args)
{
    auto sptr = get<StringPtr>(args.at(0));

    for (auto ip = args.begin() + 1, ie = args.end(); ip != ie; sptr = get<StringPtr>(*ip), ++ip) {
        const auto& sp = get<StringPtr>(*ip);

        if (sptr->length() >= sp->length()) {
            if (!equal(sp->begin(), sp->end(), sptr->begin(),
                    [](auto c0, auto c1) -> bool { return std::tolower(c0) <= std::tolower(c1); }))
                return false;
        } else {
            if (!equal(sptr->begin(), sptr->end(), sp->begin(),
                    [](auto c0, auto c1) -> bool { return std::tolower(c0) > std::tolower(c1); }))
                return false;
        }
    }
    return true;
}

/**
 * Scheme @em string-upcase function.
 */
static Cell strupcase(const varg& args)
{
    auto sptr = std::make_shared<StringPtr::element_type>(*get<StringPtr>(args.at(0)));
    std::transform(sptr->begin(), sptr->end(), sptr->begin(), ::toupper);
    return sptr;
}

/**
 * Scheme @em string-upcase function.
 */
static Cell strdowncase(const varg& args)
{
    auto sptr = std::make_shared<StringPtr::element_type>(*get<StringPtr>(args.at(0)));
    std::transform(sptr->begin(), sptr->end(), sptr->begin(), ::tolower);
    return sptr;
}

/**
 * Scheme @em string-upcase! function.
 */
static Cell strupcaseb(const varg& args)
{
    auto sptr = get<StringPtr>(args.at(0));
    std::transform(sptr->begin(), sptr->end(), sptr->begin(), ::toupper);
    return sptr;
}

/**
 * Scheme @em string-upcase! function.
 */
static Cell strdowncaseb(const varg& args)
{
    auto sptr = get<StringPtr>(args.at(0));
    std::transform(sptr->begin(), sptr->end(), sptr->begin(), ::tolower);
    return sptr;
}

/**
 * Scheme @em string-append function.
 */
static Cell strappend(const varg& args)
{
    if (args.empty())
        return std::make_shared<StringPtr::element_type>();

    auto pstr = std::make_shared<StringPtr::element_type>(*get<StringPtr>(args.at(0)));

    for (auto ip = args.begin() + 1, ie = args.end(); ip != ie; ++ip)
        pstr->append(*get<StringPtr>(*ip));

    return pstr;
}

/**
 * Scheme inplace @em string-append! function.
 */
static Cell strappendb(const varg& args)
{
    auto& sptr = get<StringPtr>(args.at(0));

    for (auto ip = args.begin() + 1, ie = args.end(); ip != ie; ++ip)
        sptr->append(*get<StringPtr>(*ip));

    return sptr;
}

/**
 * Scheme string-copy function.
 */
static Cell strcopy(const varg& args)
{
    const auto& pstr = get<StringPtr>(args.at(0));

    if (pstr->empty())
        return nil;

    Int pos = 0, end = pstr->length();

    if (args.size() > 2)
        end = std::min(get<Int>(get<Number>(args[2])), end);

    if (args.size() > 1)
        pos = std::min(get<Int>(get<Number>(args[1])), end);

    return std::make_shared<StringPtr::element_type>(pstr->substr(pos, end - pos));
}

/**
 * Scheme string-copy! function.
 */
static Cell strcopyb(const varg& args)
{
    using size_type = StringPtr::element_type::size_type;

    auto& pdst = get<StringPtr>(args.at(0));
    const auto& psrc = get<StringPtr>(args.at(2));

    if (psrc->empty())
        return pdst;

    auto at = std::min(static_cast<size_type>(get<Int>(get<Number>(args[2]))), pdst->length());

    at < pdst->length()
        || ((void)(throw std::invalid_argument("string-copy! - invalid string index position")), 0);

    size_type pos = 0, end = psrc->length();

    if (args.size() > 4)
        end = std::min(static_cast<size_type>(get<Int>(get<Number>(args[4]))), end);

    if (args.size() > 3)
        pos = std::min(static_cast<size_type>(get<Int>(get<Number>(args[3]))), end);

    if (pos || end != psrc->length())
        pdst->replace(at, end - pos, psrc->substr(pos, end - pos));
    else
        pdst->replace(at, psrc->length(), *psrc);

    return pdst;
}

/**
 * Scheme string-copy function.
 */
static Cell strfillb(const varg& args)
{
    Char c = get<Char>(args.at(1));
    const auto& pstr = get<StringPtr>(args.front());

    Int pos = 0, end = pstr->length();

    if (args.size() > 3)
        end = std::min(get<Int>(get<Number>(args[3])), end);

    if (args.size() > 3)
        pos = std::min(get<Int>(get<Number>(args[2])), end);

    pstr->replace(pos, end - pos, &c, 1);
    return pstr;
}

static Cell make_vector(const varg& args)
{
    auto size = get<Int>(get<Number>(args.at(0)));
    size >= 0 || ((void)(throw std::invalid_argument("vector length must be a non-negative integer")), 0);

    Cell val{ args.size() > 1 ? args[1] : none };
    return pscm::vec(size, val);
}

/**
 * Scheme @em vector-ref function.
 * @verbatim (vector-ref #(x0 x1 x2 ... xn) 2) => x2) @endverbatim
 */
static Cell vector_ref(const varg& args)
{
    using size_type = VectorPtr::element_type::size_type;
    auto pos = static_cast<size_type>(get<Int>(get<Number>(args.at(1))));
    return get<VectorPtr>(args[0])->at(pos);
}

/**
 * Scheme @em vector-set! function.
 * @verbatim (vector-set! #(x0 x1 x2 ... xn) 2 'z2) => #(x0 x1 z2 ... xn) @endverbatim
 */
static Cell vector_setb(const varg& args)
{
    using size_type = VectorPtr::element_type::size_type;
    auto pos = static_cast<size_type>(get<Int>(get<Number>(args.at(1))));
    get<VectorPtr>(args[0])->at(pos) = args.at(2);
    return none;
}

/**
 * Scheme @em list->vector function.
 * @verbatim (list->vector '(x0 x1 x2 ... xn)) => #(x0 x1 x2 ... xn) @endverbatim
 */
static Cell list2vec(const varg& args)
{
    Cell list = args.at(0);
    VectorPtr v = std::make_shared<VectorPtr::element_type>();

    for (/* */; is_pair(list); list = cdr(list))
        v->push_back(car(list));

    is_nil(list) || ((void)(throw std::invalid_argument("not a proper list")), 0);
    return v;
}

/**
 * Scheme @em vector->list function.
 * @verbatim (vector->list  #(x0 x1 x2 ... xn) [pos [end]]) => '(x0 x1 x2 ... xn)  @endverbatim
 */
static Cell vec2list(Scheme& scm, const varg& args)
{
    using size_type = VectorPtr::element_type::difference_type;

    const VectorPtr& vec = get<VectorPtr>(args.at(0));
    size_type pos = 0, end = static_cast<size_type>(vec->size());

    if (args.size() > 2)
        end = std::min(static_cast<size_type>(get<Int>(get<Number>(args[2]))), end);
    if (args.size() > 1)
        pos = std::min(static_cast<size_type>(get<Int>(get<Number>(args[1]))), end);

    if (pos == end)
        return nil;

    Cell list = scm.cons(vec->at(pos), nil), tail = list;

    for (auto ip = vec->begin() + pos + 1, ie = vec->begin() + end; ip != ie; ++ip, tail = cdr(tail))
        set_cdr(tail, scm.cons(*ip, nil));

    return list;
}

/**
 * Scheme @em vector-copy function.
 * @verbatim (vector-copy #(x0 x1 x2 ... xn) [pos [end]]) => #(x0 x1 x2 ... xn) @endverbatim
 */
static Cell vec_copy(const varg& args)
{
    using size_type = VectorPtr::element_type::difference_type;

    const VectorPtr& v = get<VectorPtr>(args.at(0));
    size_type pos = 0, end = static_cast<size_type>(v->size());

    if (args.size() > 2)
        end = std::min(static_cast<size_type>(get<Int>(get<Number>(args[2]))), end);
    if (args.size() > 1)
        pos = std::min(static_cast<size_type>(get<Int>(get<Number>(args[1]))), end);

    return pos != end ? std::make_shared<VectorPtr::element_type>(v->begin() + pos, v->begin() + end)
                      : std::make_shared<VectorPtr::element_type>(0);
}

/**
 * Scheme inplace @em vector-copy! function.
 * @verbatim (vector-copy! vec-dest idx vec-source [pos [end]]) => vec-dest @endverbatim
 */
static Cell vec_copyb(const varg& args)
{
    using size_type = VectorPtr::element_type::difference_type;

    const VectorPtr& src = get<VectorPtr>(args.at(2));
    VectorPtr dst = get<VectorPtr>(args[0]);

    size_type idx = static_cast<size_type>(get<Int>(get<Number>(args[1]))),
              pos = 0, end = static_cast<size_type>(src->size());

    if (args.size() > 4)
        end = std::min(static_cast<size_type>(get<Int>(get<Number>(args[4]))), end);
    if (args.size() > 3)
        pos = std::min(static_cast<size_type>(get<Int>(get<Number>(args[3]))), end);
    if (pos != end)
        std::copy(src->begin() + pos, src->begin() + end, dst->begin() + idx);

    return dst;
}

/**
 * Scheme inplace @em vector-fill! function.
 * @verbatim (vector-fill! vec value [pos [end]]) => vec @endverbatim
 */
static Cell vec_fillb(const varg& args)
{
    using size_type = VectorPtr::element_type::difference_type;

    VectorPtr vec = get<VectorPtr>(args.at(0));
    size_type pos = 0, end = static_cast<size_type>(vec->size());

    if (args.size() > 3)
        end = std::min(static_cast<size_type>(get<Int>(get<Number>(args[3]))), end);
    if (args.size() > 2)
        pos = std::min(static_cast<size_type>(get<Int>(get<Number>(args[2]))), end);
    if (pos != end)
        std::fill(vec->begin() + pos, vec->end() + end, args.at(1));

    return vec;
}

/**
 * Scheme @em vector-append function.
 * @verbatim (vector-append vec_0 vec_1 ... vec_n) => vec := {vec_0, vec_1, ..., vec_n} @endverbatim
 */
static Cell vec_append(const varg& args)
{
    auto vptr = std::make_shared<VectorPtr::element_type>(*get<VectorPtr>(args.at(0)));

    for (auto ip = begin(args) + 1, ie = end(args); ip != ie; ++ip)
        if (is_vector(*ip)) {
            const VectorPtr& v = get<VectorPtr>(*ip);
            std::copy(v->begin(), v->end(), std::back_inserter(*vptr));
        } else
            vptr->push_back(*ip);

    return vptr;
}

/**
 * Scheme inplace @em vector-append function.
 *
 * Append vectors or cells to the first argument vector.
 *
 * @verbatim (vector-append vec_0 vec_1 ... vec_n) => vec := {vec_0, vec_1, ..., vec_n} @endverbatim
 */
static Cell vec_appendb(const varg& args)
{
    VectorPtr vptr = get<VectorPtr>(args.at(0));

    for (auto ip = begin(args) + 1, ie = end(args); ip != ie; ++ip)
        if (is_vector(*ip)) {
            const VectorPtr& v = get<VectorPtr>(*ip);
            if (vptr == v)
                vptr->reserve(vptr->size() * 2);

            std::copy(v->begin(), v->end(), std::back_inserter(*vptr));
        } else
            vptr->push_back(*ip);

    return vptr;
}

static Cell callw_port(Scheme& scm, const SymenvPtr& senv, const PortPtr& port, const Cell& proc)
{
    Cons cons[4];
    Cell cell = scm.eval(senv, pscm::list(cons, Intern::_apply, proc, port, nil));
    port->close();
    return cell;
}

static Cell open_infile(const String& filnam)
{
    using port_type = FilePort<Char>;
    auto port = std::make_shared<port_type>(filnam, port_type::in);

    if (!port->is_open())
        throw std::ios_base::failure("couldn't open input file: '"s
            + string_convert<char>(filnam) + "'"s);

    return port;
}

/**
 * Scheme @em open-output-file function with optional append flag.
 *
 * (open-output-file <filename> [append? = #false])
 *
 * Default file open mode is to clear the content of an existing file.
 */
static Cell open_outfile(const varg& args)
{
    using port_type = FilePort<Char>;
    port_type::openmode mode = port_type::out;

    if (args.size() > 1 && !is_false(args[1]))
        mode |= port_type::app;

    auto& filnam = *get<StringPtr>(args.at(0));
    auto port = std::make_shared<port_type>(filnam, mode);

    if (!port->is_open())
        throw std::ios_base::failure("couldn't open output file: '"s
            + string_convert<char>(filnam) + "'"s);

    return port;
}

static Cell close_inport(Port<Char>& port)
{
    port.isInput() || ((void)(throw input_port_exception(port)), 0);
    port.close();
    return none;
}

static Cell close_outport(Port<Char>& port)
{
    port.isOutput() || ((void)(throw output_port_exception(port)), 0);
    port.close();
    return none;
}

/**
 * Scheme @em call-with-input-file procedure.
 *
 * @param scm
 * @param senv
 * @param filnam
 * @param proc
 * @return
 */
static Cell callw_infile(Scheme& scm, const SymenvPtr& senv, const String& filnam, const Cell& proc)
{
    using port_type = FilePort<Char>;
    auto port = std::make_shared<port_type>(filnam, port_type::in);

    if (!port->is_open())
        throw port_type::stream_type::failure("couldn't open input file: '"s
            + string_convert<char>(filnam) + "'"s);

    Cons cons[4];
    Cell cell = scm.eval(senv, pscm::list(cons, Intern::_apply, proc, port, nil));

    port->close();
    return cell;
}

static Cell callw_outfile(Scheme& scm, const SymenvPtr& senv, const String& filnam, const Cell& proc)
{
    using port_type = FilePort<Char>;
    auto port = std::make_shared<port_type>(filnam, port_type::out);

    if (!port->is_open())
        throw std::ios_base::failure("couldn't open output file: '"s
            + string_convert<char>(filnam) + "'"s);

    Cons cons[4];
    Cell cell = scm.eval(senv, pscm::list(cons, Intern::_apply, proc, port, nil));

    port->close();
    return cell;
}

/**
 * Scheme output @em display function.
 */
static Cell display(Scheme& scm, const varg& args)
{
    if (args.size() < 2)
        scm.outPort().stream() << pscm::display(args.at(0));
    else {
        auto& port = *get<PortPtr>(args[1]);
        port.isOutput() || ((void)(throw output_port_exception(port)), 0);

        try {
            port.stream() << pscm::display(args[0]);

        } catch (std::ios_base::failure&) {
            throw output_port_exception(port);
        }
    }
    return none;
}

/**
 * Scheme output @em write function.
 */
static Cell write(Scheme& scm, const varg& args)
{
    if (args.size() < 2)
        scm.outPort().stream() << args.at(0);
    else {
        auto& port = *get<PortPtr>(args[1]);
        port.isOutput() || ((void)(throw output_port_exception(port)), 0);

        try {
            port.stream() << args[0];

        } catch (std::ios_base::failure&) {
            throw output_port_exception(port);
        }
    }
    return none;
}

/**
 * Scheme output @em newline function.
 */
static Cell newline(Scheme& scm, const varg& args)
{
    if (args.empty())
        scm.outPort().stream() << '\n';
    else {
        auto& port = *get<PortPtr>(args[0]);
        port.isOutput() || ((void)(throw output_port_exception(port)), 0);

        try {
            port.stream() << '\n';
        } catch (std::ios_base::failure&) {
            throw output_port_exception(port);
        }
    }
    return none;
}

/**
 * Scheme output @em (flush-output-port [port]) function.
 */
static Cell flush(Scheme& scm, const varg& args)
{
    auto& port = args.empty() ? scm.outPort()
                              : *get<PortPtr>(args[0]);

    port.isOutput() || ((void)(throw output_port_exception(port)), 0);
    port.flush();
    return none;
}

/**
 * Scheme output @em write-char function.
 * (write-char <char> [<output-port>]) function.
 */
static Cell write_char(Scheme& scm, const varg& args)
{
    if (args.size() < 2)
        scm.outPort().stream() << get<Char>(args.at(0));
    else {
        auto& port = *get<PortPtr>(args[1]);
        port.isOutput() || ((void)(throw output_port_exception(port)), 0);

        try {
            port.stream() << get<Char>(args[0]);

        } catch (std::ios_base::failure&) {
            throw output_port_exception(port);
        }
    }
    return none;
}

/**
 * Scheme output @em write-string function.
 */
static Cell write_str(Scheme& scm, const varg& args)
{
    auto& port = args.size() > 1 ? *get<PortPtr>(args[1])
                                 : scm.outPort();
    port.isOutput() || ((void)(throw output_port_exception(port)), 0);

    try {
        auto& str = *get<StringPtr>(args.at(0));
        size_t ip = args.size() > 2 ? get<Int>(get<Number>(args[2])) : 0;
        size_t ie = args.size() > 3 ? get<Int>(get<Number>(args[3])) : str.size();

        if (ip || ie != str.size())
            port.stream() << str.substr(ip, ie);
        else
            port.stream() << str;

    } catch (std::ios_base::failure&) {
        throw output_port_exception(port);
    }
    return none;
}

static Cell read(Scheme& scm, const varg& args)
{
    auto& port = args.empty() ? scm.inPort()
                              : *get<PortPtr>(args[0]);
    port.isInput() || ((void)(throw input_port_exception(port)), 0);

    try {
        Parser parser{ scm };
        return parser.read(port.stream());
    } catch (std::ios_base::failure&) {
        throw input_port_exception(port);
    }
}

static Cell read_char(Scheme& scm, const varg& args)
{
    if (args.empty()) {
        auto& is = scm.inPort().stream();
        is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return static_cast<Char>(is.get());
    } else {
        auto& port = *get<PortPtr>(args[0]);
        port.isInput() || ((void)(throw input_port_exception(port)), 0);

        try {
            if (port.eof())
                return static_cast<Char>(EOF);
            return static_cast<Char>(port.stream().get());

        } catch (std::ios_base::failure&) {
            throw input_port_exception(port);
        }
    }
}

static Cell peek_char(Scheme& scm, const varg& args)
{
    if (args.empty()) {
        auto& is = scm.inPort().stream();
        is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return static_cast<Char>(is.peek());
    } else {
        auto& port = *get<PortPtr>(args[0]);
        port.isInput() || ((void)(throw input_port_exception(port)), 0);
        return static_cast<Char>(port.stream().peek());
    }
}

static Cell readline(Scheme& scm, const varg& args)
{
    String str;
    if (args.empty()) {
        auto& is = scm.inPort().stream();
        is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::getline(is, str);
    } else {
        auto& port = *get<PortPtr>(args[0]);
        port.isInput() || ((void)(throw input_port_exception(port)), 0);

        try {
            if (getline(port.stream(), str).eof() && str.empty())
                return static_cast<Char>(EOF);
        } catch (std::ios_base::failure&) {
            throw input_port_exception(port);
        }
    }
    return std::make_shared<String>(std::move(str));
}

/**
 * Scheme @em read-string function.
 * @todo This is a dummy implementation.
 */
static Cell read_str(Scheme& scm, const varg& args)
{
    auto len = get<Int>(get<Number>(args.at(0)));

    if (is_negative(len))
        throw std::invalid_argument("must be a nonnegative number");

    String str(len + 1, '\0');

    if (args.size() > 1) {
        auto& port = *get<PortPtr>(args[1]);
        port.isInput() || ((void)(throw input_port_exception(port)), 0);

        try {
            port.stream().read(str.data(), len);
            len = port.stream().gcount();
            if (!len && port.eof())
                return static_cast<Char>(EOF);
        } catch (std::ios_base::failure&) {
            throw input_port_exception(port);
        }
    } else {
        auto& is = scm.inPort().stream();
        is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        is.read(str.data(), len);
        len = is.gcount();
    }
    str.resize(len);
    str.shrink_to_fit();
    return std::make_shared<String>(std::move(str));
}

static Cell gcollect(Scheme& scm, const SymenvPtr& senv, const varg& args)
{
    GCollector gc;
    bool logok = args.size() > 0 ? get<Bool>(args[0]) : false;

    gc.logging(logok);
    gc.collect(scm, senv);
    return none;
}

static Cell gcdump(Scheme& scm, const varg& args)
{
    auto port = args.size() > 0 ? get<PortPtr>(args[0])
                                : std::make_shared<StandardPort<Char>>(std::ios_base::out);
    GCollector gc;
    gc.dump(scm, *port);
    return none;
}

static Cell macroexp(Scheme& scm, const SymenvPtr& senv, const varg& args)
{
    Cell expr = args.at(0);

    if (!is_pair(expr))
        return expr;

    Cell proc = scm.eval(senv, car(expr));
    if (!is_macro(proc))
        return expr;

    return get<Procedure>(proc).expand(scm, expr);
}

static Cell for_each(Scheme& scm, const SymenvPtr& senv, const varg& args)
{
    args.size() > 1
        || ((void)(throw std::invalid_argument("for-each - not enough arguments")), 0);

    const Cell& proc = args.front();

    if (args.size() <= 2) // single list version:
    {
        varg argv{ 1 };
        for (Cell list = args.at(1); is_pair(list); list = cdr(list)) {
            argv[0] = car(list);
            apply(scm, senv, proc, argv);
        }
        return none;
    } else { // multiple list version:
        std::vector<Cell> lists{ args.begin() + 1, args.end() };

        varg argv;
        argv.reserve(lists.size());

        for (;;) {
            for (auto& l : lists)
                if (is_pair(l)) {
                    argv.push_back(car(l));
                    l = cdr(l);
                } else
                    return none;

            apply(scm, senv, proc, argv);
            argv.clear();
        }
    }
}

static Cell map(Scheme& scm, const SymenvPtr& senv, const varg& args)
{
    args.size() > 1
        || ((void)(throw std::invalid_argument("map - not enough arguments")), 0);

    const Cell& proc = args.front();

    if (args.size() <= 2) // single list version:
    {
        Cell list = args.at(1);
        if (is_nil(list))
            return nil;

        varg argv{ 1, car(list) };
        Cell head = scm.cons(apply(scm, senv, proc, argv), nil), tail = head;

        for (list = cdr(list); is_pair(list); list = cdr(list), tail = cdr(tail)) {
            argv[0] = car(list);
            set_cdr(tail, scm.cons(apply(scm, senv, proc, argv), nil));
        }
        return head;

    } else { // multiple list version:
        std::vector<Cell> lists{ args.begin() + 1, args.end() };

        varg argv;
        argv.reserve(lists.size());
        Cell head = nil, tail = nil;

        for (;;) {
            for (auto& l : lists)
                if (is_pair(l)) {
                    argv.push_back(car(l));
                    l = cdr(l);
                } else
                    return head;

            if (is_pair(head)) {
                set_cdr(tail, scm.cons(apply(scm, senv, proc, argv), nil));
                tail = cdr(tail);
            } else
                head = tail = scm.cons(apply(scm, senv, proc, argv), nil);
            argv.clear();
        }
    }
}

/**
 * Return a regular expression object from argument string.
 * Scheme function (regex "regex"
 *
 * @param args[0]  Regular expression string.
 * @param args[1]  Optional -
 *
 * @throws a std::regex_error exception if the supplied regular expression
 *         string is invalid
 */
static Cell regex(Scheme&, const varg& args)
{
    using regex = RegexPtr::element_type;
    auto& pstr = get<StringPtr>(args.at(0));

    regex::flag_type flags = regex::ECMAScript | regex::icase;
    return std::make_shared<RegexPtr::element_type>(*pstr, flags);
}

/**
 * Determine if the argument regular expression matches the entire string.
 * Implement scheme function: <em> (regex-match regex string [submatch? = false]) </em>
 *
 * @param args[0] Regular expression
 * @param args[1] String to match
 * @param args[2] Optional - if true and match exists, return vector of
 *                matched string and all possible submatches or false otherwise.
 * @return false if no match exists, true or vector otherwise.
 */
static Cell regex_match(const varg& args)
{
    using string = StringPtr::element_type;
    using vector = VectorPtr::element_type;

    auto& pregex = get<RegexPtr>(args.at(0));
    auto& pstr = get<StringPtr>(args.at(1));

    bool submatches = args.size() > 2 && get<Bool>(args[2]);

    if (submatches) {
        std::match_results<string::const_iterator> smatch;

        if (std::regex_match(*pstr, smatch, *pregex)) {

            auto vres{ std::make_shared<vector>() };
            vres->reserve(smatch.size());

            for (auto& m : smatch)
                vres->push_back(pscm::str(m.str()));

            return vres;
        } else
            return false;

    } else
        return std::regex_match(*pstr, *pregex);
}

/**
 * Determine if the argument regular expression matches some subsequence in the string.
 * Implement scheme function: <em> (regex-match regex string [submatch? = false]) </em>
 *
 * @param args[0] Regular expression
 * @param args[1] String to match
 * @return false if no match exists, or a vector of found submatches otherwise.
 */
static Cell regex_search(const varg& args)
{
    using string = StringPtr::element_type;
    using vector = VectorPtr::element_type;

    auto& pregex = get<RegexPtr>(args.at(0));
    auto str{ *get<StringPtr>(args.at(1)) };

    std::match_results<string::const_iterator> smatch;
    auto vres{ std::make_shared<vector>(0) };

    while (std::regex_search(str, smatch, *pregex)) {
        vres->push_back(pscm::str(smatch.str()));
        str = smatch.suffix();
    }
    return vres->size() ? Cell{ vres } : Cell{ false };
}

static Cell dict_insert(const varg& args)
{
    auto& dict = *get<MapPtr>(args.at(0));
    auto [_, ok] = dict.insert(std::make_pair(args.at(1), args.at(2)));
    return ok;
}

static Cell dict_assign(const varg& args)
{
    auto& dict = *get<MapPtr>(args.at(0));
    dict.insert_or_assign(args.at(1), args.at(2));
    return none;
}

static Cell dict_find(const varg& args)
{
    auto& dict = *get<MapPtr>(args.at(0));
    auto pos = dict.find(args.at(1));

    Cell not_found{ args.size() > 2 ? args[2] : none };
    return pos != dict.end() ? pos->second : not_found;
}

} // namespace pscm::primop

namespace pscm {

Cell call(Scheme& scm, const SymenvPtr& senv, Intern primop, const varg& args)
{
    switch (primop) {
    /* Section 6.1: Equivalence predicates */
    case Intern::op_eq:
    case Intern::op_eqv:
        return args.at(0) == args.at(1);
    case Intern::op_equal:
        return is_equal(args.at(0), args.at(1));

    /* Section 6.2: Numbers */
    case Intern::op_isnum:
        return is_number(args.at(0));
    case Intern::op_iscpx:
        return is_number(args.at(0));
    case Intern::op_isreal:
        return is_number(args.at(0))
            && (is_int(get<Number>(args.front()))
                   || is_float(get<Number>(args.front())));
    case Intern::op_israt:
        return is_number(args.at(0)) && is_integer(get<Number>(args.front()));
    case Intern::op_isint:
        return is_number(args.at(0)) && is_integer(get<Number>(args.front()));
    case Intern::op_isexact:
        return is_number(args.at(0)) && is_int(get<Number>(args.front()));
    case Intern::op_isinexact:
        return is_number(args.at(0)) && !is_int(get<Number>(args.front()));
    case Intern::op_isexactint:
        return is_number(args.at(0)) && is_int(get<Number>(args.front()));
    case Intern::op_ex2inex:
        return primop::ex2inex(args.at(0));
    case Intern::op_inex2ex:
        return primop::inex2ex(args.at(0));
    case Intern::op_isodd:
        return is_number(args.at(0)) && is_odd(get<Number>(args.front()));
    case Intern::op_iseven:
        return is_number(args.at(0)) && !is_odd(get<Number>(args.front()));
    case Intern::op_numeq:
        return primop::numeq(args);
    case Intern::op_numlt:
        return primop::numlt(args);
    case Intern::op_numgt:
        return primop::numgt(args);
    case Intern::op_numle:
        return primop::numle(args);
    case Intern::op_numge:
        return primop::numge(args);
    case Intern::op_add:
        return primop::add(args);
    case Intern::op_sub:
        return primop::sub(args);
    case Intern::op_mul:
        return primop::mul(args);
    case Intern::op_div:
        return primop::div(args);
    case Intern::op_min:
        return primop::min(args);
    case Intern::op_max:
        return primop::max(args);
    case Intern::op_ispos:
        return get<Number>(args.at(0)) > Number{ 0 };
    case Intern::op_isneg:
        return get<Number>(args.at(0)) < Number{ 0 };
    case Intern::op_mod:
        return get<Number>(args.at(0)) % get<Number>(args.at(1));
    case Intern::op_rem:
        return remainder(get<Number>(args.at(0)), get<Number>(args.at(1)));
    case Intern::op_zero:
        return is_zero(get<Number>(args.at(0)));
    case Intern::op_floor:
        return pscm::floor(get<Number>(args.at(0)));
    case Intern::op_ceil:
        return pscm::ceil(get<Number>(args.at(0)));
    case Intern::op_quotient:
        return quotient(get<Number>(args.at(0)), get<Number>(args.at(1)));
    case Intern::op_trunc:
        return pscm::trunc(get<Number>(args.at(0)));
    case Intern::op_round:
        return pscm::round(get<Number>(args.at(0)));
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
        return primop::log(args);
    case Intern::op_log10:
        return pscm::log10(get<Number>(args.at(0)));
    case Intern::op_sqrt:
        return pscm::sqrt(get<Number>(args.at(0)));
    case Intern::op_cbrt:
        return pscm::cbrt(get<Number>(args.at(0)));
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
    case Intern::op_rect:;
        return pscm::rect(get<Number>(args.at(0)), get<Number>(args.at(1)));
    case Intern::op_polar:
        return pscm::polar(get<Number>(args.at(0)), get<Number>(args.at(1)));
    case Intern::op_hypot:
        return primop::hypot(args);
    case Intern::op_strnum:
        return Parser::strnum(*get<StringPtr>(args.at(0)));
    case Intern::op_numstr:
        return primop::numstr(args);

    /* Section 6.3: Booleans */
    case Intern::op_not:
        return !is_true(args.at(0));
    case Intern::op_isbool:
        return is_bool(args.at(0));
    case Intern::op_isbooleq:
        return primop::booleq(args);

    /* Section 6.4: Pair and lists */
    case Intern::op_cons:
        return scm.cons(args.at(0), args.at(1));
    case Intern::op_car:
        return car(args.at(0));
    case Intern::op_cdr:
        return cdr(args.at(0));
    case Intern::op_caar:
        return caar(args.at(0));
    case Intern::op_cddr:
        return cddr(args.at(0));
    case Intern::op_cadr:
        return cadr(args.at(0));
    case Intern::op_cdar:
        return cdar(args.at(0));
    case Intern::op_caddr:
        return caddr(args.at(0));
    case Intern::op_setcar:
        return (void)(set_car(args.at(0), args.at(1))), none;
    case Intern::op_setcdr:
        return (void)(set_cdr(args.at(0), args.at(1))), none;
    case Intern::op_list:
        return primop::list(scm, args);
    case Intern::op_mklist:
        return primop::makelist(scm, args);
    case Intern::op_isnil:
        return is_nil(args.at(0));
    case Intern::op_ispair:
        return is_pair(args.at(0));
    case Intern::op_islist:
        return is_list(args.at(0));
    case Intern::op_append:
        return primop::append(scm, args);
    case Intern::op_length:
        return Number{ list_length(args.at(0)) };
    case Intern::op_listref:
        return primop::listref(args);
    case Intern::op_listsetb:
        return primop::listsetb(args);
    case Intern::op_listcopy:
        return primop::listcopy(scm, args);
    case Intern::op_reverse:
        return primop::reverse(scm, args);
    case Intern::op_reverseb:
        return primop::reverseb(args);
    case Intern::op_memq:
        return primop::memq(senv, args);
    case Intern::op_memv:
        return primop::memq(senv, args);
    case Intern::op_member:
        return primop::member(scm, senv, args);
    case Intern::op_assq:
        return primop::assq(senv, args);
    case Intern::op_assv:
        return primop::assq(senv, args);
    case Intern::op_assoc:
        return primop::assoc(scm, senv, args);

    /* Section 6.5: Symbols */
    case Intern::op_issym:
        return is_symbol(args.at(0));
    case Intern::op_symstr:
        return std::make_shared<StringPtr::element_type>(get<Symbol>(args.at(0)).value());
    case Intern::op_strsym:
        return scm.symbol(get<StringPtr>(args.at(0))->c_str());
    case Intern::op_gensym:
        return scm.symbol();

    /* Section 6.6: Characters */
    case Intern::op_ischar:
        return is_type<Char>(args.at(0));
    case Intern::op_charint:
        return num(get<Char>(args.at(0)));
    case Intern::op_intchar:
        return static_cast<Char>((get<Int>(get<Number>(args.at(0)))));
    case Intern::op_ischareq:
        return primop::ischareq(args);
    case Intern::op_ischarlt:
        return primop::ischarlt(args);
    case Intern::op_ischargt:
        return primop::ischargt(args);
    case Intern::op_ischarle:
        return primop::ischarle(args);
    case Intern::op_ischarge:
        return primop::ischarge(args);
    case Intern::op_ischcieq:
        return primop::ischcieq(args);
    case Intern::op_ischcilt:
        return primop::ischcilt(args);
    case Intern::op_ischcigt:
        return primop::ischcigt(args);
    case Intern::op_ischcile:
        return primop::ischcile(args);
    case Intern::op_ischcige:
        return primop::ischcige(args);
    case Intern::op_isalpha:
        return static_cast<bool>(std::isalpha(get<Char>(args.at(0))));
    case Intern::op_isdigit:
        return static_cast<bool>(std::isdigit(get<Char>(args.at(0))));
    case Intern::op_iswspace:
        return static_cast<bool>(std::isspace(get<Char>(args.at(0))));
    case Intern::op_isupper:
        return static_cast<bool>(std::isupper(get<Char>(args.at(0))));
    case Intern::op_islower:
        return static_cast<bool>(std::islower(get<Char>(args.at(0))));
    case Intern::op_upcase:
        return static_cast<Char>(std::toupper(get<Char>(args.at(0))));
    case Intern::op_downcase:
        return static_cast<Char>(std::tolower(get<Char>(args.at(0))));
    case Intern::op_digitval:
        return primop::digitval(args);

    /* Section 6.7: Strings */
    case Intern::op_isstr:
        return is_type<StringPtr>(args.at(0));
    case Intern::op_mkstr:
        return primop::mkstring(args);
    case Intern::op_str:
        return primop::string(args);
    case Intern::op_strappend:
        return primop::strappend(args);
    case Intern::op_strappendb:
        return primop::strappendb(args);
    case Intern::op_strlen:
        return Number{ get<StringPtr>(args.at(0))->length() };
    case Intern::op_strref:
        return get<StringPtr>(args.at(0))->at(get<Int>(get<Number>(args.at(1))));
    case Intern::op_strsetb:
        return get<StringPtr>(args.at(0))->at(get<Int>(get<Number>(args.at(1))))
            = get<Char>(args.at(2));
    case Intern::op_isstreq:
        return primop::isstreq(args);
    case Intern::op_isstrlt:
        return primop::isstrlt(args);
    case Intern::op_isstrgt:
        return primop::isstrgt(args);
    case Intern::op_isstrle:
        return primop::isstrle(args);
    case Intern::op_isstrge:
        return primop::isstrge(args);
    case Intern::op_isstrcieq:
        return primop::isstrcieq(args);
    case Intern::op_isstrcilt:
        return primop::isstrcilt(args);
    case Intern::op_isstrcigt:
        return primop::isstrcigt(args);
    case Intern::op_isstrcile:
        return primop::isstrcile(args);
    case Intern::op_isstrcige:
        return primop::isstrcige(args);
    case Intern::op_strupcase:
        return primop::strupcase(args);
    case Intern::op_strdowncase:
        return primop::strdowncase(args);
    case Intern::op_strupcaseb:
        return primop::strupcaseb(args);
    case Intern::op_strdowncaseb:
        return primop::strdowncaseb(args);
    case Intern::op_substr:
        return primop::strcopy(args);
    case Intern::op_strcopy:
        return primop::strcopy(args);
    case Intern::op_strcopyb:
        return primop::strcopyb(args);
    case Intern::op_strfillb:
        return primop::strfillb(args);
    case Intern::op_strlist:
        return primop::strlist(scm, args);
    case Intern::op_liststr:
        return primop::liststr(args);

    /* Section 6.8: Vectors */
    case Intern::op_isvec:
        return is_type<VectorPtr>(args.at(0));
    case Intern::op_mkvec:
        return primop::make_vector(args);
    case Intern::op_vec:
        return std::make_shared<VectorPtr::element_type>(args);
    case Intern::op_veclen:
        return Number{ get<VectorPtr>(args.at(0))->size() };
    case Intern::op_vecref:
        return primop::vector_ref(args);
    case Intern::op_vecsetb:
        return primop::vector_setb(args);
    case Intern::op_veclist:
        return primop::vec2list(scm, args);
    case Intern::op_listvec:
        return primop::list2vec(args);
    case Intern::op_veccopy:
        return primop::vec_copy(args);
    case Intern::op_veccopyb:
        return primop::vec_copyb(args);
    case Intern::op_vecappend:
        return primop::vec_append(args);
    case Intern::op_vecappendb:
        return primop::vec_appendb(args);
    case Intern::op_vecfillb:
        return primop::vec_fillb(args);

    /* Section 6.9: Bytevectors */

    /* Section 6.10: Control features */
    case Intern::op_isproc:
        return primop::is_proc(args);
    case Intern::op_callcc:
        return primop::callcc(scm, senv, args);
    case Intern::op_callwval:
        return primop::callwval(scm, senv, args);
    case Intern::op_map:
        return primop::map(scm, senv, args);
    case Intern::op_foreach:
        return primop::for_each(scm, senv, args);

    /* Section 6.11: Exceptions */
    case Intern::op_error:
        return primop::error(scm, senv, args);
    case Intern::op_with_exception:
        return primop::withexcept(scm, senv, args);
    case Intern::op_exit:
        return Intern::op_exit;

    /* Section 6.12: Environments and evaluation */
    case Intern::op_replenv:
        return senv;
    case Intern::op_repl:
        scm.repl(senv);
        return none;
    case Intern::op_eval:
        return scm.eval(args.size() > 1 ? get<SymenvPtr>(args[1]) : senv, args.at(0));
    case Intern::_apply:
        return primop::apply(scm, senv, args);
    case Intern::op_gc:
        return primop::gcollect(scm, senv, args);
    case Intern::op_gcdump:
        return primop::gcdump(scm, args);
    case Intern::op_macroexp:
        return primop::macroexp(scm, senv, args);

    /* Section 6.13: Input and output */
    case Intern::op_isport:
        return is_type<PortPtr>(args.at(0));
    case Intern::op_isinport:
        return is_type<PortPtr>(args.at(0)) && get<PortPtr>(args[0])->isInput();
    case Intern::op_isoutport:
        return is_type<PortPtr>(args.at(0)) && get<PortPtr>(args[0])->isOutput();
    case Intern::op_istxtport:
        return is_type<PortPtr>(args.at(0)) && !get<PortPtr>(args[0])->isBinary();
    case Intern::op_isbinport:
        return is_type<PortPtr>(args.at(0)) && get<PortPtr>(args[0])->isBinary();
    case Intern::op_isinport_open:
        return is_type<PortPtr>(args.at(0)) && get<PortPtr>(args[0])->isInput() && get<PortPtr>(args[0])->good();
    case Intern::op_isoutport_open:
        return is_type<PortPtr>(args.at(0)) && get<PortPtr>(args[0])->isOutput() && get<PortPtr>(args[0])->good();
    case Intern::op_callw_port:
        return primop::callw_port(scm, senv, get<PortPtr>(args.at(0)), args.at(1));
    case Intern::op_callw_infile:
        return primop::callw_infile(scm, senv, *get<StringPtr>(args.at(0)), args.at(1));
    case Intern::op_callw_outfile:
        return primop::callw_outfile(scm, senv, *get<StringPtr>(args.at(0)), args.at(1));
    case Intern::op_open_infile:
        return primop::open_infile(*get<StringPtr>(args.at(0)));
    case Intern::op_open_outfile:
        return primop::open_outfile(args);
    case Intern::op_close_port:
        return ((void)(get<PortPtr>(args.at(0))->close()), none);
    case Intern::op_close_inport:
        return primop::close_inport(*get<PortPtr>(args.at(0)));
    case Intern::op_close_outport:
        return primop::close_outport(*get<PortPtr>(args.at(0)));
    case Intern::op_readline:
        return primop::readline(scm, args);
    case Intern::op_read:
        return primop::read(scm, args);
    case Intern::op_read_char:
        return primop::read_char(scm, args);
    case Intern::op_peek_char:
        return primop::peek_char(scm, args);
    case Intern::op_read_str:
        return primop::read_str(scm, args);
    case Intern::op_eof:
        return static_cast<Char>(EOF);
    case Intern::op_iseof:
        return is_type<Char>(args.at(0)) && get<Char>(args[0]) == static_cast<Char>(EOF);
    case Intern::op_flush:
        return primop::flush(scm, args);
    case Intern::op_write:
        return primop::write(scm, args);
    case Intern::op_display:
        return primop::display(scm, args);
    case Intern::op_newline:
        return primop::newline(scm, args);
    case Intern::op_write_char:
        return primop::write_char(scm, args);
    case Intern::op_write_str:
        return primop::write_str(scm, args);

    /* Section 6.14: System interface */
    case Intern::op_load:
        scm.load(*get<StringPtr>(args.at(0)), senv);
        return none;

    /* Section extensions - Regular expressions */
    case Intern::op_regex:
        return primop::regex(scm, args);
    case Intern::op_regex_match:
        return primop::regex_match(args);
    case Intern::op_regex_search:
        return primop::regex_search(args);

    /* Section extensions - Date, clock and time measurements */
    case Intern::op_clock:
        return std::make_shared<Clock>();
    case Intern::op_clock_toc:
        return Number{ get<ClockPtr>(args.at(0))->toc() };
    case Intern::op_clock_tic:
        return ((void)get<ClockPtr>(args.at(0))->tic(), none);
    case Intern::op_clock_pause:
        return ((void)get<ClockPtr>(args.at(0))->pause(), none);
    case Intern::op_clock_resume:
        return ((void)get<ClockPtr>(args.at(0))->resume(), none);

    case Intern::op_usecount:
        return Number{ use_count(args.at(0)) };
    case Intern::op_hash:
        return Number{ pscm::hash<Cell>{}(args.at(0)) };

    /* Section extensions - Dictionary as std::map */
    case Intern::op_make_dict:
        return std::make_shared<MapPtr::element_type>();
    case Intern::op_dict_ref:
        return std::get<MapPtr>(args.at(0))->at(args.at(1));
    case Intern::op_dict_setb:
        return ((void)(std::get<MapPtr>(args.at(0))->at(args.at(1)) = args.at(2)), none);
    case Intern::op_dict_isempty:
        return std::get<MapPtr>(args.at(0))->empty();
    case Intern::op_dict_size:
        return Number{ std::get<MapPtr>(args.at(0))->size() };
    case Intern::op_dict_count:
        return Number{ std::get<MapPtr>(args.at(0))->count(args.at(1)) };
    case Intern::op_dict_erase:
        return Bool{ std::get<MapPtr>(args.at(0))->erase(args.at(1)) != 0 };
    case Intern::op_dict_clear:
        return ((void)std::get<MapPtr>(args.at(0))->clear(), none);
    case Intern::op_dict_insert:
        return primop::dict_insert(args);
    case Intern::op_dict_assign:
        return primop::dict_assign(args);
    case Intern::op_dict_find:
        return primop::dict_find(args);

    default:
        throw std::invalid_argument("invalid primary opcode");
    }
}

void add_environment_defaults(Scheme& scm)
{
    // clang-format off
    scm.addenv(
       {  { scm.symbol("#t"),               true },
          { scm.symbol("#f"),               false },
          { scm.symbol("#true"),            true },
          { scm.symbol("#false"),           false },
          { scm.symbol("π"),                num(pi<Float>) },
          { scm.symbol("%pi"),              num(pi<Float>) },
          { scm.symbol("%e"),               num(e<Float>) },
          { scm.symbol("%G"),               num(G<Float>) },
          { scm.symbol("%c"),               num(c<Float>) },
          { scm.symbol("%h"),               num(h<Float>) },
          { scm.symbol("%qe"),              num(q_e<Float>) },
          { scm.symbol("%NA"),              num(N_A<Float>) },
          { scm.symbol("%R"),               num(R<Float>) },
          { scm.symbol("%mu0"),             num(mu_0<Float>) },
          { scm.symbol("%eps0"),            num(epsilon_0<Float>) },
          { scm.symbol("%sigma"),           num(sigma<Float>) },

          /* Basic scheme syntax opcodes */
          { scm.symbol("or"),               Intern::_or },
          { scm.symbol("and"),              Intern::_and },
          { scm.symbol("if"),               Intern::_if },
          { scm.symbol("cond"),             Intern::_cond },
          { scm.symbol("else"),             Intern::_else },
          { scm.symbol("=>"),               Intern::_arrow },
          { scm.symbol("when"),             Intern::_when },
          { scm.symbol("unless"),           Intern::_unless },
          { scm.symbol("begin"),            Intern::_begin },
          { scm.symbol("define"),           Intern::_define },
          { scm.symbol("set!"),             Intern::_setb },
          { scm.symbol("lambda"),           Intern::_lambda },
          { scm.symbol("define-macro"),     Intern::_macro },
          { scm.symbol("quote"),            Intern::_quote },
          { scm.symbol("quasiquote"),       Intern::_quasiquote },
          { scm.symbol("unquote"),          Intern::_unquote },
          { scm.symbol("unquote-splicing"), Intern::_unquotesplice },
          { scm.symbol("apply"),            Intern::_apply },

          /* Section 6.1: Equivalence predicates */
          { scm.symbol("eq?"),              Intern::op_eq },
          { scm.symbol("eqv?"),             Intern::op_eqv },
          { scm.symbol("equal?"),           Intern::op_equal },

          /* Section 6.2: Numbers */
          { scm.symbol("number?"),          Intern::op_isnum },
          { scm.symbol("complex?"),         Intern::op_iscpx },
          { scm.symbol("real?"),            Intern::op_isreal },
          { scm.symbol("rational?"),        Intern::op_israt },
          { scm.symbol("integer?"),         Intern::op_isint },
          { scm.symbol("exact?"),           Intern::op_isexact },
          { scm.symbol("inexact?"),         Intern::op_isinexact },
          { scm.symbol("exact-integer?"),   Intern::op_isexactint },
          { scm.symbol("exact->inexact"),   Intern::op_ex2inex },
          { scm.symbol("inexact->exact"),   Intern::op_inex2ex },
          { scm.symbol("even?"),            Intern::op_iseven },
          { scm.symbol("odd?"),             Intern::op_isodd },
          { scm.symbol("="),                Intern::op_numeq },
          { scm.symbol("<"),                Intern::op_numlt },
          { scm.symbol(">"),                Intern::op_numgt },
          { scm.symbol("<="),               Intern::op_numle },
          { scm.symbol(">="),               Intern::op_numge },
          { scm.symbol("+"),                Intern::op_add },
          { scm.symbol("-"),                Intern::op_sub },
          { scm.symbol("*"),                Intern::op_mul },
          { scm.symbol("/"),                Intern::op_div },
          { scm.symbol("min"),              Intern::op_min },
          { scm.symbol("max"),              Intern::op_max },
          { scm.symbol("positive?"),        Intern::op_ispos },
          { scm.symbol("negative?"),        Intern::op_isneg },
          { scm.symbol("zero?"),            Intern::op_zero },
          { scm.symbol("modulo"),           Intern::op_mod },
          { scm.symbol("remainder"),        Intern::op_rem },
          { scm.symbol("quotient"),         Intern::op_quotient },
          { scm.symbol("floor"),            Intern::op_floor },
          { scm.symbol("ceil"),             Intern::op_ceil },
          { scm.symbol("trunc"),            Intern::op_trunc },
          { scm.symbol("round"),            Intern::op_round },
          { scm.symbol("sin"),              Intern::op_sin },
          { scm.symbol("cos"),              Intern::op_cos },
          { scm.symbol("tan"),              Intern::op_tan },
          { scm.symbol("asin"),             Intern::op_asin },
          { scm.symbol("acos"),             Intern::op_acos },
          { scm.symbol("atan"),             Intern::op_atan },
          { scm.symbol("sinh"),             Intern::op_sinh },
          { scm.symbol("cosh"),             Intern::op_cosh },
          { scm.symbol("tanh"),             Intern::op_tanh },
          { scm.symbol("asinh"),            Intern::op_asinh },
          { scm.symbol("acosh"),            Intern::op_acosh },
          { scm.symbol("atanh"),            Intern::op_atanh },
          { scm.symbol("sqrt"),             Intern::op_sqrt },
          { scm.symbol("cbrt"),             Intern::op_cbrt },
          { scm.symbol("exp"),              Intern::op_exp },
          { scm.symbol("expt"),             Intern::op_pow },
          { scm.symbol("log"),              Intern::op_log },
          { scm.symbol("log10"),            Intern::op_log10 },
          { scm.symbol("square"),           Intern::op_square },
          { scm.symbol("real-part"),        Intern::op_real },
          { scm.symbol("imag-part"),        Intern::op_imag },
          { scm.symbol("magnitude"),        Intern::op_abs },
          { scm.symbol("abs"),              Intern::op_abs },
          { scm.symbol("angle"),            Intern::op_arg },
          { scm.symbol("make-rectangular"), Intern::op_rect },
          { scm.symbol("make-polar"),       Intern::op_polar },
          { scm.symbol("conjugate"),        Intern::op_conj },
          { scm.symbol("hypot"),            Intern::op_hypot },
          { scm.symbol("string->number"),   Intern::op_strnum },
          { scm.symbol("number->string"),   Intern::op_numstr },

          /* Section 6.3: Booleans */
          { scm.symbol("not"),              Intern::op_not },
          { scm.symbol("boolean?"),         Intern::op_isbool },
          { scm.symbol("boolean=?"),        Intern::op_isbooleq },

          /* Section 6.4: Pair and lists */
          { scm.symbol("cons"),             Intern::op_cons },
          { scm.symbol("car"),              Intern::op_car },
          { scm.symbol("cdr"),              Intern::op_cdr },
          { scm.symbol("caar"),             Intern::op_caar },
          { scm.symbol("cddr"),             Intern::op_cddr },
          { scm.symbol("cadr"),             Intern::op_cadr },
          { scm.symbol("cdar"),             Intern::op_cdar },
          { scm.symbol("caddr"),            Intern::op_caddr },
          { scm.symbol("set-car!"),         Intern::op_setcar },
          { scm.symbol("set-cdr!"),         Intern::op_setcdr },
          { scm.symbol("list"),             Intern::op_list },
          { scm.symbol("null?"),            Intern::op_isnil },
          { scm.symbol("pair?"),            Intern::op_ispair },
          { scm.symbol("list?"),            Intern::op_islist },
          { scm.symbol("make-list"),        Intern::op_mklist },
          { scm.symbol("append"),           Intern::op_append },
          { scm.symbol("length"),           Intern::op_length },
          { scm.symbol("list-ref"),         Intern::op_listref },
          { scm.symbol("list-set!"),        Intern::op_listsetb },
          { scm.symbol("list-copy"),        Intern::op_listcopy },
          { scm.symbol("reverse"),          Intern::op_reverse },
          { scm.symbol("reverse!"),         Intern::op_reverseb },
          { scm.symbol("memq"),             Intern::op_memq },
          { scm.symbol("memv"),             Intern::op_memv },
          { scm.symbol("member"),           Intern::op_member },
          { scm.symbol("assq"),             Intern::op_assq },
          { scm.symbol("assv"),             Intern::op_assv },
          { scm.symbol("assoc"),            Intern::op_assoc },

          /* Section 6.5: Symbols */
          { scm.symbol("symbol?"),          Intern::op_issym },
          { scm.symbol("symbol->string"),   Intern::op_symstr },
          { scm.symbol("string->symbol"),   Intern::op_strsym },
          { scm.symbol("gensym"),           Intern::op_gensym },

          /* Section 6.6: Characters */
          { scm.symbol("char?"),            Intern::op_ischar },
          { scm.symbol("char->integer"),    Intern::op_charint },
          { scm.symbol("integer->char"),    Intern::op_intchar },
          { scm.symbol("char=?"),           Intern::op_ischareq },
          { scm.symbol("char<?"),           Intern::op_ischarlt },
          { scm.symbol("char>?"),           Intern::op_ischargt },
          { scm.symbol("char<=?"),          Intern::op_ischarle },
          { scm.symbol("char>=?"),          Intern::op_ischarge },
          { scm.symbol("char-ci=?"),        Intern::op_ischcieq },
          { scm.symbol("char-ci<?"),        Intern::op_ischcilt },
          { scm.symbol("char-ci>?"),        Intern::op_ischcigt },
          { scm.symbol("char-ci<=?"),       Intern::op_ischcile },
          { scm.symbol("char-ci>=?"),       Intern::op_ischcige },
          { scm.symbol("char-alphabetic?"), Intern::op_isalpha },
          { scm.symbol("char-numeric?"),    Intern::op_isdigit },
          { scm.symbol("char-whitespace?"), Intern::op_iswspace },
          { scm.symbol("char-upper-case?"), Intern::op_isupper },
          { scm.symbol("char-lower-case?"), Intern::op_islower },
          { scm.symbol("digit-value"),      Intern::op_digitval },
          { scm.symbol("char-upcase"),      Intern::op_upcase },
          { scm.symbol("char-downcase"),    Intern::op_downcase },

          /* Section 6.7:using namespace std::string_literals; Strings */
          { scm.symbol("string?"),          Intern::op_isstr },
          { scm.symbol("string"),           Intern::op_str },
          { scm.symbol("make-string"),      Intern::op_mkstr },
          { scm.symbol("string-ref"),       Intern::op_strref },
          { scm.symbol("string-set!"),      Intern::op_strsetb },
          { scm.symbol("string-length"),    Intern::op_strlen },
          { scm.symbol("string=?"),         Intern::op_isstreq },
          { scm.symbol("string<?"),         Intern::op_isstrlt },
          { scm.symbol("string>?"),         Intern::op_isstrgt },
          { scm.symbol("string<=?"),        Intern::op_isstrle },
          { scm.symbol("string>=?"),        Intern::op_isstrge },
          { scm.symbol("string-ci=?"),      Intern::op_isstrcieq },
          { scm.symbol("string-ci=?"),      Intern::op_isstrcieq },
          { scm.symbol("string-ci<?"),      Intern::op_isstrcilt },
          { scm.symbol("string-ci>?"),      Intern::op_isstrcigt },
          { scm.symbol("string-ci<=?"),     Intern::op_isstrcile },
          { scm.symbol("string-ci>=?"),     Intern::op_isstrcige },
          { scm.symbol("string-upcase"),    Intern::op_strupcase },
          { scm.symbol("string-downcase"),  Intern::op_strdowncase },
          { scm.symbol("string-upcase!"),   Intern::op_strupcaseb },
          { scm.symbol("string-downcase!"), Intern::op_strdowncaseb },
          { scm.symbol("string-append"),    Intern::op_strappend },
          { scm.symbol("string-append!"),   Intern::op_strappendb },
          { scm.symbol("string->list"),     Intern::op_strlist },
          { scm.symbol("list->string"),     Intern::op_liststr },
          { scm.symbol("substring"),        Intern::op_substr },
          { scm.symbol("string-copy"),      Intern::op_strcopy },
          { scm.symbol("string-copy!"),     Intern::op_strcopyb },
          { scm.symbol("string-fill!"),     Intern::op_strfillb },

          /* Section 6.8: Vectors */
          { scm.symbol("vector?"),          Intern::op_isvec },
          { scm.symbol("make-vector"),      Intern::op_mkvec },
          { scm.symbol("vector"),           Intern::op_vec },
          { scm.symbol("vector-length"),    Intern::op_veclen },
          { scm.symbol("vector-ref"),       Intern::op_vecref },
          { scm.symbol("vector-set!"),      Intern::op_vecsetb },
          { scm.symbol("vector->list"),     Intern::op_veclist },
          { scm.symbol("list->vector"),     Intern::op_listvec },
          { scm.symbol("vector-copy"),      Intern::op_veccopy },
          { scm.symbol("vector-copy!"),     Intern::op_veccopyb },
          { scm.symbol("vector-append"),    Intern::op_vecappend },
          { scm.symbol("vector-append!"),   Intern::op_vecappendb },
          { scm.symbol("vector-fill!"),     Intern::op_vecfillb },

          /* Section 6.9: Bytevectors */

          /* Section 6.10: Control features */
          { scm.symbol("procedure?"), Intern::op_isproc },
          { scm.symbol("map"), Intern::op_map },
          { scm.symbol("for-each"), Intern::op_foreach },
          { scm.symbol("call/cc"), Intern::op_callcc },
          { scm.symbol("call-with-current-continuation"), Intern::op_callcc },
          { scm.symbol("call-with-values"), Intern::op_callwval },

          /* Section 6.11: Exceptions */
          { scm.symbol("error"), Intern::op_error },
          { scm.symbol("with-exception-handler"), Intern::op_with_exception },
          { scm.symbol("exit"), Intern::op_exit },

          /* Section 6.12: Environments and evaluation */
          { scm.symbol("interaction-environment"), Intern::op_replenv },
          { scm.symbol("eval"), Intern::op_eval },
          { scm.symbol("repl"), Intern::op_repl },
          { scm.symbol("gc"), Intern::op_gc },
          { scm.symbol("gc-dump"), Intern::op_gcdump },
          { scm.symbol("macro-expand"), Intern::op_macroexp },

          /* Section 6.13: Input and output */
          { scm.symbol("port?"), Intern::op_isport },
          { scm.symbol("input-port?"), Intern::op_isinport },
          { scm.symbol("output-port?"), Intern::op_isoutport },
          { scm.symbol("input-port-open?"), Intern::op_isinport_open },
          { scm.symbol("output-port-open?"), Intern::op_isoutport_open },
          { scm.symbol("textual-port?"), Intern::op_istxtport },
          { scm.symbol("binary-port?"), Intern::op_isbinport },
          { scm.symbol("call-with-input-file"), Intern::op_callw_infile },
          { scm.symbol("call-with-output-file"), Intern::op_callw_outfile },
          { scm.symbol("open-input-file"), Intern::op_open_infile },
          { scm.symbol("open-output-file"), Intern::op_open_outfile },
          { scm.symbol("close-port"), Intern::op_close_port },
          { scm.symbol("close-input-port"), Intern::op_close_inport },
          { scm.symbol("close-output-port"), Intern::op_close_outport },
          { scm.symbol("eof-object?"), Intern::op_iseof },
          { scm.symbol("eof-object"), Intern::op_eof },
          { scm.symbol("flush-output-port"), Intern::op_flush },
          { scm.symbol("read-line"), Intern::op_readline },
          { scm.symbol("read-char"), Intern::op_read_char },
          { scm.symbol("peek-char"), Intern::op_peek_char },
          { scm.symbol("read-string"), Intern::op_read_str },
          { scm.symbol("write"), Intern::op_write },
          { scm.symbol("read"), Intern::op_read },
          { scm.symbol("display"), Intern::op_display },
          { scm.symbol("newline"), Intern::op_newline },
          { scm.symbol("write-char"), Intern::op_write_char },
          { scm.symbol("write-string"), Intern::op_write_str },

          /* Section 6.14: System interface */
          { scm.symbol("load"), Intern::op_load },

          /* Extension: regular expressions */
          { scm.symbol("regex"), Intern::op_regex },
          { scm.symbol("regex-match"), Intern::op_regex_match },
          { scm.symbol("regex-search"), Intern::op_regex_search },

          /* Extension: clock */
          { scm.symbol("clock"),        Intern::op_clock},
          { scm.symbol("clock-tic"),    Intern::op_clock_tic},
          { scm.symbol("clock-toc"),    Intern::op_clock_toc},
          { scm.symbol("clock-pause"),  Intern::op_clock_pause},
          { scm.symbol("clock-resume"), Intern::op_clock_resume},

          /* Extension: dictionary */
          { scm.symbol("make-dict"),    Intern::op_make_dict},
          { scm.symbol("dict-ref"),     Intern::op_dict_ref},
          { scm.symbol("dict-set!"),    Intern::op_dict_setb},
          { scm.symbol("dict-size"),    Intern::op_dict_size},
          { scm.symbol("dict-empty?"),  Intern::op_dict_isempty},
          { scm.symbol("dict-clear"),   Intern::op_dict_clear},
          { scm.symbol("dict-erase"),   Intern::op_dict_erase},
          { scm.symbol("dict-find"),    Intern::op_dict_find},
          { scm.symbol("dict-count"),   Intern::op_dict_count},
          { scm.symbol("dict-insert"),  Intern::op_dict_insert},
          { scm.symbol("dict-assign!"), Intern::op_dict_assign},
          { scm.symbol("dict-find"),    Intern::op_dict_find},

          { scm.symbol("use-count"),    Intern::op_usecount },
          { scm.symbol("hash"),         Intern::op_hash },
       });
    // clang-format on
}
} // namespace pscm
