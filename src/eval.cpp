/********************************************************************************/ /**
 * @file eval.hpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#include "eval.hpp"

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

static std::vector<Cell> eval_vec(const Symenv& senv, Cell list)
{
    std::vector<Cell> vec;

    for (/* */; is_pair(list); list = cdr(list))
        vec.push_back(eval(senv, car(list)));

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

    if (is_proc(proc))
        tie(senv, proc, args) = apply(senv, proc, args);

    switch (get<Intern>(proc)) {

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
        expr = args;
        goto begin;

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

    case Intern::op_add:
        return fun_add(eval_vec(senv, args));

    default:
        throw std::invalid_argument("invalid opcode");
    }
}
}; // namespace pscm
