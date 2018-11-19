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
    std::ostringstream buf;
    buf << get<Number>(args.at(0));
    return std::make_shared<StringPtr::element_type>(buf.str());
}

/**
 * Scheme output @em display function.
 */
static Cell display(const varg& args)
{
    if (args.size() > 1) {
        Port& port = get<Port>(const_cast<Cell&>(args.at(1)));

        port.stream() << pscm::display(args[0]);
    } else
        std::cout << pscm::display(args.at(0));

    return none;
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
 * @brief Scheme @em make-list function.
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

static Cell apply(Scheme& scm, const SymenvPtr& senv, const Cell& proc, const varg& args)
{
    if (pscm::is_proc(proc)) {
        Cons arg[2], cns[4];
        Cell argv = pscm::alist(arg, Intern::_quote, nil);
        Cell expr = pscm::alist(cns, Intern::_apply, proc, argv, nil);

        if (args.empty()) {
            set_cdr(cddr(expr), nil);
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

struct continuation_exception {
    continuation_exception(const Cell& cell)
        : continuation{ cell }
    {
    }
    Cell continuation;
};

/**
 * Call with current continuation.
 *
 * Simple implementation as escape continuation
 */
static Cell callcc(Scheme& scm, const SymenvPtr& senv, const varg& args)
{
    std::function<Cell(Scheme&, const SymenvPtr&, const std::vector<Cell>&)>
        lambda = [](Scheme&, const SymenvPtr&, const varg& args) -> Cell {
        throw continuation_exception{ args.at(0) };
        return none;
    };

    varg arg{ scm.mkfun(std::move(lambda)) };
    try {
        return apply(scm, senv, args.at(0), arg);
    } catch (const continuation_exception& e) {
        return e.continuation;
    }
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

/**
 * @brief Scheme @em vector-ref function.
 * @verbatim (vector-ref #(x0 x1 x2 ... xn) 2) => x2) @endverbatim
 */
static Cell vector_ref(const varg& args)
{
    using size_type = VectorPtr::element_type::size_type;
    auto pos = static_cast<size_type>(get<Int>(get<Number>(args.at(1))));
    return get<VectorPtr>(args[0])->at(pos);
}

/**
 * @brief Scheme @em vector-set! function.
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

static Cell callw_port(Scheme& scm, const SymenvPtr& senv, Port port, const Cell& proc)
{
    Cons cons[4];
    Cell cell = scm.eval(senv, alist(cons, Intern::_apply, proc, port, nil));
    port.close();
    return cell;
}

static Cell callw_infile(Scheme& scm, const SymenvPtr& senv, const StringPtr& filnam, const Cell& proc)
{
    Port port;
    if (!port.open(*filnam, std::ios_base::in))
        throw std::ios_base::failure("couldn't open input file: '"s + *filnam + "'"s);

    Cons cons[4];
    return scm.eval(senv, alist(cons, Intern::_apply, proc, port, nil));
}

static Cell callw_outfile(Scheme& scm, const SymenvPtr& senv, const StringPtr& filnam, const Cell& proc)
{
    Port port;
    if (!port.open(*filnam, std::ios_base::out))
        throw std::ios_base::failure("couldn't open output file: '"s + *filnam + "'"s);

    Cons cons[4];
    return scm.eval(senv, alist(cons, Intern::_apply, proc, port, nil));
}

static Cell open_infile(const StringPtr& filnam)
{
    Port port;
    if (!port.open(*filnam, std::ios_base::in))
        throw std::ios_base::failure("couldn't open input file: '"s + *filnam + "'"s);

    return port;
}

static Cell open_outfile(const StringPtr& filnam)
{
    Port port;
    if (!port.open(*filnam, std::ios_base::out))
        throw std::ios_base::failure("couldn't open output file: '"s + *filnam + "'"s);

    return port;
}

/**
 * Scheme output @em write function.
 */
static Cell write(const varg& args)
{
    if (args.size() > 1) {
        Port& port = get<Port>(const_cast<Cell&>(args[1]));
        port.stream() << args[0];
    } else
        std::cout << args.at(0);

    return none;
}

/**
 * Scheme output @em newline function.
 */
static Cell newline(const varg& args)
{
    if (args.size() > 0) {
        Port& port = get<Port>(const_cast<Cell&>(args[0]));
        port.stream() << '\n';
    } else
        std::cout << '\n';

    return none;
}

/**
 * Scheme output @em write-char function.
 */
static Cell flush(const varg& args)
{
    if (args.size() > 0) {
        Port& port = get<Port>(const_cast<Cell&>(args[0]));
        port.stream().flush();
    } else
        std::cout.flush();

    return none;
}

/**
 * Scheme output @em write-char function.
 */
static Cell write_char(const varg& args)
{
    if (args.size() > 1) {
        Port& port = get<Port>(const_cast<Cell&>(args[1]));
        port.stream() << get<Char>(args[0]);
    } else
        std::cout << get<Char>(args.at(0));

    return none;
}

/**
 * Scheme output @em write-char function.
 */
static Cell write_str(const varg& args)
{
    using size_type = StringPtr::element_type::size_type;
    const StringPtr& pstr = get<StringPtr>(args.at(0));

    Port port;
    if (args.size() > 1)
        port = get<Port>(args[1]);

    size_type ip = args.size() > 2 ? get<Int>(get<Number>(const_cast<Cell&>(args[2]))) : 0;
    size_type ie = args.size() > 3 ? get<Int>(get<Number>(const_cast<Cell&>(args[3]))) : pstr->size();

    if (ip || ie != pstr->size())
        port.stream() << pstr->substr(ip, ie);
    else
        port.stream() << *pstr;
    return none;
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

static Cell readline(const varg& args)
{
    std::string str;
    if (args.size() > 0) {
        Port port = get<Port>(args[0]);

        (port.is_open() && port.is_input())
            || ((void)(throw std::invalid_argument("port is closed")), 0);

        getline(port.stream(), str);
    } else {
        getline(std::cin, str);
    }
    return std::make_shared<StringPtr::element_type>(std::move(str));
}

static Cell read(Scheme& scm, const varg& args)
{
    Parser parser(scm);

    if (args.size() > 0) {
        Port port = get<Port>(args[0]);

        (port.is_open() && port.is_input())
            || ((void)(throw std::invalid_argument("port is closed")), 0);

        return parser.read(port.stream());
    } else
        return parser.read(std::cin);
}

static Cell read_char(const varg& args)
{
    Port port;
    if (args.size() > 0) {
        port = get<Port>(args[0]);
        (port.is_open() && port.is_input())
            || ((void)(throw std::invalid_argument("port is closed")), 0);
    }
    return static_cast<Char>(port.stream().get());
}

static Cell peek_char(const varg& args)
{
    Port port;
    if (args.size() > 0) {
        port = get<Port>(args[0]);
        (port.is_open() && port.is_input())
            || ((void)(throw std::invalid_argument("port is closed")), 0);
    }
    return static_cast<Char>(port.stream().peek());
}

/**
 * Scheme @em read-string function.
 * @todo This is a dummy implementation.
 */
static Cell read_str(Scheme& scm, const varg& args)
{
    Port port;
    Number num = get<Number>(args.at(0));
    if (is_int(num) || is_negative(num))
        throw std::invalid_argument("must be a nonnegative number");

    if (args.size() > 1) {
        port = get<Port>(args[0]);
        (port.is_open() && port.is_input())
            || ((void)(throw std::invalid_argument("port is closed")), 0);
    }
    Parser parser(scm);
    return parser.read(port.stream());
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
    GCollector gc;
    Port port;

    if (args.size() > 1) {
        port = get<Port>(args[0]);
        (port.is_open() && port.is_output())
            || ((void)(throw std::invalid_argument("port is must be an output port and open")), 0);
    }
    gc.dump(scm, port.stream());
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

static Cell error(const varg& args)
{
    std::string msg{ "error: " };
    msg.append(*get<StringPtr>(args.at(0)));

    throw std::invalid_argument(msg.c_str());
    return none;
}

static Cell foreach (Scheme& scm, const SymenvPtr& senv, const varg& args)
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
                vres->push_back(pscm::mkstr(m.str()));

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
        vres->push_back(pscm::mkstr(smatch.str()));
        str = smatch.suffix();
    }
    return vres->size() ? Cell{ vres } : Cell{ false };
}

} // namespace primop

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
        return args.size() > 2 ? pscm::hypot(get<Number>(args[0]),
                                     get<Number>(args[1]),
                                     get<Number>(args[2]))
                               : pscm::hypot(get<Number>(args.at(0)),
                                     get<Number>(args.at(1)));
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
        return scm.mksym(get<StringPtr>(args.at(0))->c_str());
    case Intern::op_gensym:
        return scm.mksym();

    /* Section 6.6: Characters */
    case Intern::op_ischar:
        return is_type<Char>(args.at(0));
    case Intern::op_charint:
        return mknum(get<Char>(args.at(0)));
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
        return args.size() > 1 ? mkvec(get<Number>(args[0]), args[1])
                               : mkvec(get<Number>(args.at(0)), none);
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
    case Intern::op_map:
        return primop::map(scm, senv, args);
    case Intern::op_foreach:
        return primop::foreach (scm, senv, args);

    /* Section 6.11: Exceptions */
    case Intern::op_error:
        return primop::error(args);
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
    case Intern::op_callw_port:
        return primop::callw_port(scm, senv, get<Port>(args.at(0)), args.at(1));
    case Intern::op_callw_infile:
        return primop::callw_infile(scm, senv, get<StringPtr>(args.at(0)), args.at(1));
    case Intern::op_callw_outfile:
        return primop::callw_outfile(scm, senv, get<StringPtr>(args.at(0)), args.at(1));
    case Intern::op_open_infile:
        return primop::open_infile(get<StringPtr>(args.at(0)));
    case Intern::op_open_outfile:
        return primop::open_outfile(get<StringPtr>(args.at(0)));
    case Intern::op_close_port:
    case Intern::op_close_inport:
    case Intern::op_close_outport:
        get<Port>(args.at(0)).close();
        return none;
    case Intern::op_readline:
        return primop::readline(args);
    case Intern::op_read:
        return primop::read(scm, args);
    case Intern::op_read_char:
        return primop::read_char(args);
    case Intern::op_peek_char:
        return primop::peek_char(args);
    case Intern::op_read_str:
        return primop::read_str(scm, args);
    case Intern::op_eof:
        return Char{ EOF };
    case Intern::op_iseof:
        return is_type<Char>(args.at(0)) && get<Char>(args[0]) == EOF;
    case Intern::op_flush:
        return primop::flush(args);
    case Intern::op_write:
        return primop::write(args);
    case Intern::op_display:
        return primop::display(args);
    case Intern::op_newline:
        return primop::newline(args);
    case Intern::op_write_char:
        return primop::write_char(args);
    case Intern::op_write_str:
        return primop::write_str(args);

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
    case Intern::op_usecount:
        return Number{ use_count(args.at(0)) };

    default:
        throw std::invalid_argument("invalid primary opcode");
    }
}
} // namespace pscm
