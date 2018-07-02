/********************************************************************************/ /**
 * @file eval.hpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#include "eval.hpp"
#include "stream.hpp"

namespace pscm {

using std::get;

static Cell syntax_setb(const Symenv& senv, const Cell& args)
{
    setenv(senv, car(args), eval(senv, cadr(args)));
    return none;
}

static Cell syntax_begin(const Symenv& senv, Cell args)
{
    if (is_pair(args)) {
        for (/* */; is_pair(cdr(args)); args = cdr(args))
            eval(senv, car(args));

        return car(args);
    }
    return none;
}

static Cell syntax_if(const Symenv& senv, const Cell& args)
{
    if (is_true(eval(senv, car(args))))
        return cadr(args);
    else if (Cell last = cddr(args); !is_nil(last))
        return car(last);
    else
        return none;
}

static Cell syntax_and(const Symenv& senv, Cell args)
{
    Cell res = true;

    for (/* */; is_pair(args); args = cdr(args))
        if (is_false(res = eval(senv, car(args))))
            return res;

    return res;
}

static Cell syntax_or(const Symenv& senv, Cell args)
{
    Cell res = false;

    for (/* */; is_pair(args); args = cdr(args))
        if (is_true(res = eval(senv, car(args))))
            return res;

    return res;
}

static Cell syntax_define(const Symenv& senv, Cell args)
{
    Cell expr = cdr(args);
    args = car(args);

    if (is_pair(args))
        addenv(senv, car(args), lambda(senv, cdr(args), expr));
    else
        addenv(senv, args, eval(senv, car(expr)));

    return none;
}

Cell eval_list(const Symenv& senv, Cell list)
{
    if (is_pair(list)) {
        Cell head = cons(eval(senv, car(list)), cdr(list));
        list = cdr(list);

        for (Cell tail = head; is_pair(list); tail = cdr(tail), list = cdr(list))
            set_cdr(tail, cons(eval(senv, car(list)), cdr(list)));

        return head;
    } else
        return nil;
}

static std::vector<Cell> eval_args(const Symenv& senv, Cell args, bool is_list)
{
    std::vector<Cell> vec;

    if (is_list) {
        for (/* */; is_pair(args); args = cdr(args))
            vec.push_back(eval(senv, car(args)));

        return vec;
    }
    Cell last = nil;

    for (/* */; is_pair(args); args = cdr(args))
        vec.push_back(last = eval(senv, car(args)));

    if (is_nil(last))
        return vec;

    vec.back() = car(last);
    for (args = cdr(last); is_pair(args); args = cdr(args))
        vec.push_back(car(args));

    return vec;
}

Cell eval(Symenv senv, Cell expr)
{
begin:
    if (is_symbol(expr))
        return getenv(senv, expr);

    if (!is_pair(expr))
        return expr;

    Cell args = cdr(expr), proc = eval(senv, car(expr));

    if (is_proc(proc)) {
        tie(senv, expr) = apply(senv, proc, args, true);
        goto begin;
    }
    switch (Intern primop = proc) {

    case Intern::_quote:
        return car(args);

    case Intern::_define:
        return syntax_define(senv, args);

    case Intern::_lambda:
        return lambda(senv, car(args), cdr(args));

    case Intern::_begin:
        expr = syntax_begin(senv, args);
        goto begin;

    case Intern::_apply:
        proc = eval(senv, car(args));
        args = cdr(args);

        if (is_proc(proc)) {
            tie(senv, expr) = apply(senv, proc, args, false);
            goto begin;
        } else
            return call(senv, proc, eval_args(senv, args, false));

    case Intern::_setb:
        return syntax_setb(senv, args);

    case Intern::_if:
        expr = syntax_if(senv, args);
        goto begin;

    case Intern::_and:
        expr = syntax_and(senv, args);
        goto begin;

    case Intern::_or:
        expr = syntax_or(senv, args);
        goto begin;

    default:
        return call(senv, primop, eval_args(senv, args, true));
    }
}

Cell call(const Symenv& senv, Intern primop, const std::vector<Cell>& args)
{
    switch (primop) {
    case Intern::op_add:
        return fun_add(args);

    default:
        throw std::invalid_argument("invalid primary operation");
    }
}

}; // namespace pscm
