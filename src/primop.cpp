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
#include <iostream>
#include <memory>

#include "cell.hpp"
#include "eval.hpp"
#include "parser.hpp"
#include "primop.hpp"

using varg = std::vector<pscm::Cell>;

namespace pscm::primop {

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
        rhs = std::get<Number>(static_cast<Cell>(*iter));

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
        rhs = std::get<Number>(static_cast<Cell>(*iter));

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
        rhs = std::get<Number>(*iter);

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
 * Scheme number addition + operator function.
 */
static Cell add(const varg& args)
{
    Number res = 0;

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

    for (auto iter = ++args.begin(); iter != args.end(); ++iter)
        res -= get<Number>(*iter);

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
 * Scheme output @em display function.
 */
static Cell display(const varg& args)
{
    if (args.size() > 1) {
        Port& port = std::get<Port>(const_cast<Cell&>(args.at(1)));

        port.stream() << pscm::display(args[0]);
    } else
        std::cout << pscm::display(args.at(0));

    return none;
}

/**
 * Scheme list @em append function.
 * @verbatim (append [list_0 list_1 ... expr]) => nil | expr | (list_0 . list_1 ... . expr) @endverbatim
 */
static Cell append(const varg& args)
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
 * @verbatim (make-list len [fill = none]) => (fill ... fill) @endverbatim
 */
static Cell makelist(const varg& args)
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
 * Scheme @em reverse list function.
 * @verbatim (reverse (list 1 2 ... n)) => (n n-1 ... 2 1) @endverbatim
 */
static Cell reverse(const varg& args)
{
    Cell list = args.at(0), head = nil;

    for (/* */; is_pair(list); head = cons(car(list), head), list = cdr(list))
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

/**
 * Scheme list @em member function.
 */
static Cell member(const SymenvPtr& senv, const varg& args)
{
    Cell list = args.at(1);
    const Cell& obj = args[0];

    if (args.size() > 2) {
        std::deque<Cons> vec;

        for (; is_pair(list); list = cdr(list)) {

            Cell expr = vlist(vec, Intern::_apply, args[2],
                vlist(vec, Intern::_quote, vlist(vec, obj, car(list))));

            if (is_true(eval(senv, expr)))
                return list;

            vec.clear();
        }

    } else
        for (; is_pair(list); list = cdr(list))
            if (is_equal(obj, car(list)))
                return list;
    is_nil(list)
        || ((void)(throw std::invalid_argument("member - invalid argument list")), 0);

    return false;
}

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
 * Scheme inplace @em string-append function.
 */
static Cell str_append(const varg& args)
{
    auto strptr = std::make_shared<StringPtr::element_type>(*get<StringPtr>(args.at(0)));

    for (auto ip = args.begin() + 1, ie = args.end(); ip != ie; ++ip)
        strptr->append(*get<StringPtr>(*ip));

    return strptr;
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
static Cell vec2list(const varg& args)
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

    Cell list = cons(vec->at(pos), nil), tail = list;

    for (auto ip = vec->begin() + pos + 1, ie = vec->begin() + end; ip != ie; ++ip, tail = cdr(tail))
        set_cdr(tail, cons(*ip, nil));

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

static Cell callw_port(const SymenvPtr& senv, Port port, const Cell& proc)
{
    Cons cons[4];
    Cell cell = eval(senv, alist(cons, Intern::_apply, proc, port, nil));
    port.close();
    return cell;
}

static Cell callw_infile(const SymenvPtr& senv, const StringPtr& filnam, const Cell& proc)
{
    Port port;
    if (!port.open(*filnam, std::ios_base::in))
        throw std::ios_base::failure("couldn't open input file: '"s + *filnam + "'"s);

    Cons cons[4];
    return eval(senv, alist(cons, Intern::_apply, proc, port, nil));
}

static Cell callw_outfile(const SymenvPtr& senv, const StringPtr& filnam, const Cell& proc)
{
    Port port;
    if (!port.open(*filnam, std::ios_base::out))
        throw std::ios_base::failure("couldn't open output file: '"s + *filnam + "'"s);

    Cons cons[4];
    return eval(senv, alist(cons, Intern::_apply, proc, port, nil));
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
        Port& port = std::get<Port>(const_cast<Cell&>(args[1]));
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
        Port& port = std::get<Port>(const_cast<Cell&>(args[0]));
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
        Port& port = std::get<Port>(const_cast<Cell&>(args[0]));
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
        Port& port = std::get<Port>(const_cast<Cell&>(args[1]));
        port.stream() << std::get<Char>(args[0]);
    } else
        std::cout << std::get<Char>(args.at(0));

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
        port = std::get<Port>(args[1]);

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
static Cell list(const varg& args)
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

static Cell readline(const varg& args)
{
    StringPtr line = str("");

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

static Cell read(const varg& args)
{
    Port port;
    if (args.size() > 0) {
        port = get<Port>(args[0]);
        (port.is_open() && port.is_input())
            || ((void)(throw std::invalid_argument("port is closed")), 0);
    }
    Parser parser;
    return parser.read(port.stream());
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
static Cell read_str(const varg& args)
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
    Parser parser;
    return parser.read(port.stream());
}

static Cell macroexp(const SymenvPtr& senv, const varg& args)
{
    Cell expr = args.at(0);

    if (!is_pair(expr))
        return expr;

    Cell proc = eval(senv, car(expr));
    if (!is_macro(proc))
        return expr;

    return get<Proc>(proc).expand(expr);
}

static Cell error(const varg& args)
{
    std::string msg{ "error: " };
    msg.append(*get<StringPtr>(args.at(0)));

    throw std::invalid_argument(msg.c_str());
    return none;
}

/**
 * Map version for a procedure and single list argument.
 */
static Cell map(const SymenvPtr& senv, const Proc& proc, Cell list)
{
    if (is_nil(list))
        return nil;

    Cons cns[4], arg[2];
    Cell argv = pscm::alist(arg, Intern::_quote, car(list));
    Cell expr = pscm::alist(cns, Intern::_apply, proc, argv, nil);
    Cell head = cons(eval(senv, expr), nil);

    list = cdr(list);
    for (Cell tail = head; is_pair(list); list = cdr(list), tail = cdr(tail)) {
        set_car(cdr(argv), car(list));
        set_cdr(tail, cons(eval(senv, expr), nil));
    }
    return head;
}

/**
 * Map version for primop or functions and a single list argument.
 */
static Cell map(const SymenvPtr& senv, const Cell& proc, Cell list)
{
    if (is_proc(proc))
        return map(senv, get<Proc>(proc), list);

    if (is_nil(list))
        return nil;

    std::vector<Cell> argv{ 1, car(list) };
    Cell head = cons(call(senv, proc, argv), nil);

    list = cdr(list);
    for (Cell tail = head; is_pair(list); list = cdr(list), tail = cdr(tail)) {
        argv.front() = car(list);
        set_cdr(tail, cons(call(senv, proc, argv), nil));
    }
    return head;
}

/**
 * Map version for procedures and multiple list arguments.
 */
static Cell map(const SymenvPtr& senv, const Proc& proc, varg& lists)
{
    if (lists.size() <= 1)
        return map(senv, proc, lists.at(0));

    Cons cns[3], arg[2];
    Cell argv = pscm::alist(arg, Intern::_quote, nil);
    Cell expr = pscm::alist(cns, Intern::_apply, proc, argv);
    std::vector<Cons> vec{ lists.size() };

    Cell head = nil, tail = nil;

    for (;;) {
        size_t i = 0;
        for (auto& l : lists)
            if (is_pair(l)) {
                vec[i] = Cons{ car(l), nil };
                if (i)
                    vec[i - 1].second = static_cast<Cons*>(&vec[i]);

                i++;
                l = cdr(l);
            } else
                return head;

        set_car(cdr(argv), &vec.front());
        if (is_nil(head))
            head = tail = cons(eval(senv, expr), nil);
        else {
            set_cdr(tail, cons(eval(senv, expr), nil));
            tail = cdr(tail);
        }
    }
}
/**
 * Map version for primop or functions and multiple list arguments.
 */
static Cell map(const SymenvPtr& senv, const Cell& proc, varg& lists)
{
    if (lists.size() <= 1)
        return map(senv, proc, lists.at(0));

    if (is_proc(proc))
        return map(senv, get<Proc>(proc), lists);

    varg args{ lists.size() };
    Cell head = nil, tail = nil;

    for (;;) {
        size_t i = 0;
        for (auto& l : lists)
            if (is_pair(l)) {
                args[i++] = car(l);
                l = cdr(l);
            } else
                return head;

        if (is_nil(head))
            head = tail = cons(call(senv, proc, args), nil);
        else {
            set_cdr(tail, cons(call(senv, proc, args), nil));
            tail = cdr(tail);
        }
    }
}

static Cell map(const SymenvPtr& senv, const varg& args)
{
    args.size() > 1
        || ((void)(throw std::invalid_argument("map - not enough arguments")), 0);

    if (args.size() <= 2) // single list version:
        return map(senv, args.at(0), args.at(1));

    // multiple list version:
    std::vector<Cell> lists{ args.begin() + 1, args.end() };
    return map(senv, args.front(), lists);
}
} // namespace primop

namespace pscm {

Cell call(const SymenvPtr& senv, Intern primop, const varg& args)
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
        return get<Number>(args.at(1)) < get<Number>(args[0]) ? args[1] : args[0];
    case Intern::op_max:
        return get<Number>(args.at(1)) > get<Number>(args[0]) ? args[1] : args[0];
    case Intern::op_ispos:
        return get<Number>(args.at(0)) > Number{ 0 };
    case Intern::op_isneg:
        return get<Number>(args.at(0)) < Number{ 0 };
    case Intern::op_zero:
        return is_zero(get<Number>(args.at(0)));
    case Intern::op_floor:
        return pscm::floor(get<Number>(args.at(0)));
    case Intern::op_ceil:
        return pscm::ceil(get<Number>(args.at(0)));
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
    /* Section 6.3: Booleans */
    case Intern::op_not:
        return !is_true(args.at(0));
    case Intern::op_isbool:
        return is_bool(args.at(0));
    case Intern::op_isbooleq:
        return primop::booleq(args);

    /* Section 6.4: Pair and lists */
    case Intern::op_cons:
        return cons(args.at(0), args.at(1));
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
        return primop::list(args);
    case Intern::op_mklist:
        return primop::makelist(args);
    case Intern::op_isnil:
        return is_nil(args.at(0));
    case Intern::op_ispair:
        return is_pair(args.at(0));
    case Intern::op_islist:
        return is_list(args.at(0));
    case Intern::op_append:
        return primop::append(args);
    case Intern::op_length:
        return Number{ list_length(args.at(0)) };
    case Intern::op_listref:
        return primop::listref(args);
    case Intern::op_listsetb:
        return primop::listsetb(args);
    case Intern::op_reverse:
        return primop::reverse(args);
    case Intern::op_reverseb:
        return primop::reverseb(args);
    case Intern::op_member:
        return primop::member(senv, args);
    /* Section 6.5: Symbols */
    case Intern::op_issym:
        return is_symbol(args.at(0));
    case Intern::op_symstr:
        return std::make_shared<StringPtr::element_type>(get<Symbol>(args.at(0)).value());
    case Intern::op_strsym:
        return sym(get<StringPtr>(args.at(0))->c_str());
    case Intern::op_gensym:
        return gensym();

    /* Section 6.6: Characters */
    case Intern::op_ischar:
        return is_type<Char>(args.at(0));
    case Intern::op_charint:
        return num(std::get<Char>(args.at(0)));

    /* Section 6.7: Strings */
    case Intern::op_isstr:
        return is_type<StringPtr>(args.at(0));
    case Intern::op_mkstr:
        return primop::mkstring(args);
    case Intern::op_strappend:
        return primop::str_append(args);

    /* Section 6.8: Vectors */
    case Intern::op_isvec:
        return is_type<VectorPtr>(args.at(0));
    case Intern::op_mkvec:
        return args.size() > 1 ? vec(get<Number>(args[0]), args[1])
                               : vec(get<Number>(args.at(0)), none);
    case Intern::op_vec:
        return std::make_shared<VectorPtr::element_type>(args);
    case Intern::op_veclen:
        return Number{ get<VectorPtr>(args.at(0))->size() };
    case Intern::op_vecref:
        return primop::vector_ref(args);
    case Intern::op_vecsetb:
        return primop::vector_setb(args);
    case Intern::op_veclist:
        return primop::vec2list(args);
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
        return is_proc(args.at(0)) || is_func(args[0])
            || (is_intern(args[0]) && std::get<Intern>(args[0]) >= Intern::op_eq);
    case Intern::op_map:
        return primop::map(senv, args);

    /* Section 6.11: Exceptions */
    case Intern::op_error:
        return primop::error(args);
    case Intern::op_exit:
        return Intern::op_exit;

    /* Section 6.12: Environments and evaluation */
    case Intern::op_replenv:
        return senv;
    case Intern::op_repl:
        repl(senv);
        return none;
    case Intern::op_eval:
        return eval(args.size() > 1 ? get<SymenvPtr>(args[1]) : senv, args[0]);
    case Intern::op_macroexp:
        return primop::macroexp(senv, args);

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
        return primop::callw_port(senv, get<Port>(args.at(0)), args.at(1));
    case Intern::op_callw_infile:
        return primop::callw_infile(senv, get<StringPtr>(args.at(0)), args.at(1));
    case Intern::op_callw_outfile:
        return primop::callw_outfile(senv, get<StringPtr>(args.at(0)), args.at(1));
    case Intern::op_open_infile:
        return primop::open_infile(get<StringPtr>(args.at(0)));
    case Intern::op_open_outfile:
        return primop::open_outfile(get<StringPtr>(args.at(0)));
    case Intern::op_readline:
        return primop::readline(args);
    case Intern::op_read:
        return primop::read(args);
    case Intern::op_read_char:
        return primop::read_char(args);
    case Intern::op_peek_char:
        return primop::peek_char(args);
    case Intern::op_read_str:
        return primop::read_str(args);
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
        load(*get<StringPtr>(args.at(0)), senv);
        return none;

    default:
        throw std::invalid_argument("invalid primary opcode");
    }
}
} // namespace pscm
