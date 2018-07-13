/********************************************************************************/ /**
 * @file eval.hpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#include "eval.hpp"
#include "parser.hpp"
#include "primop.hpp"
#include "proc.hpp"

namespace pscm {

/**
 * @brief Evaluate a scheme (set! symbol expr) expression.
 *
 * Evaluate expression an set the symbol value to the result.
 * The symbol must be found in the argument environment or
 * any reachable parent environment.
 *
 * @return Special value none
 */
static Cell syntax_setb(const Symenv& senv, const Cell& args)
{
    senv->set(car(args), eval(senv, cadr(args)));
    return none;
}

/**
 * @brief  Evaluate a scheme (begin expr_0 ... expr_n) expression.
 *
 * Evaluate each expression in argument list up the last, which
 * is returned unevaluated. This last expression is evaluated at
 * the call site to support unbound tail-recursion.
 *
 * @return Unevaluated last expression or special symbol none for an
 *         empty argument list.
 */
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

static Cell syntax_when(const Symenv& senv, Cell args)
{
    if (is_true(eval(senv, car(args))) && is_pair(args = cdr(args))) {
        for (/* */; is_pair(cdr(args)); args = cdr(args))
            eval(senv, car(args));

        return car(args);
    }
    return none;
}

static Cell syntax_unless(const Symenv& senv, Cell args)
{
    if (is_false(eval(senv, car(args))) && is_pair(args = cdr(args))) {
        for (/* */; is_pair(cdr(args)); args = cdr(args))
            eval(senv, car(args));

        return car(args);
    }
    return none;
}

static Cell syntax_and(const Symenv& senv, Cell args)
{
    Cell res = true;

    if (is_pair(args)) {
        for (/* */; is_pair(cdr(args)); args = cdr(args))
            if (is_false(res = eval(senv, car(args))))
                return res;

        is_nil(cdr(args)) || (throw std::invalid_argument("not a proper list"), 0);
        return car(args);
    }
    return res;
}

static Cell syntax_or(const Symenv& senv, Cell args)
{
    Cell res = false;

    if (is_pair(args)) {
        for (/* */; is_pair(cdr(args)); args = cdr(args))
            if (is_true(res = eval(senv, car(args))))
                return res;

        is_nil(cdr(args)) || (throw std::invalid_argument("not a proper list"), 0);
        return car(args);
    }
    return res;
}

static Cell syntax_define(const Symenv& senv, Cell args)
{
    Cell expr = cdr(args);
    args = car(args);

    if (is_pair(args))
        senv->add(car(args), Proc{ senv, cdr(args), expr });
    else
        senv->add(args, eval(senv, car(expr)));

    return none;
}

Cell eval_list(const Symenv& senv, Cell list, bool is_list)
{
    if (!is_pair(list))
        return nil;

    if (is_list) {
        Cell head = cons(eval(senv, car(list)), cdr(list));
        list = cdr(list);

        for (Cell tail = head; is_pair(list); tail = cdr(tail), list = cdr(list))
            set_cdr(tail, cons(eval(senv, car(list)), cdr(list)));

        return head;
    }
    Cell tail, head;

    if (is_pair(cdr(list)))
        head = cons(eval(senv, car(list)), cdr(list));
    else
        head = eval(senv, car(list));

    for (tail = head, list = cdr(list); is_pair(list); tail = cdr(tail), list = cdr(list))
        if (is_pair(cdr(list)))
            set_cdr(tail, cons(eval(senv, car(list)), cdr(list)));
        else
            set_cdr(tail, eval(senv, car(list)));

    is_nil(tail) || is_pair(tail) || (throw std::invalid_argument("invalid apply argument list"), 0);
    return head;
}

static std::vector<Cell> eval_args(const Symenv& senv, Cell args, bool is_list = true)
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

    if (is_nil(last)) {
        if (!vec.empty())
            vec.pop_back();
        return vec;
    }
    vec.back() = car(last);
    for (args = cdr(last); is_pair(args); args = cdr(args))
        vec.push_back(car(args));

    return vec;
}

Cell eval(Symenv senv, Cell expr)
{
    Cell args, proc;

    for (;;) {
        if (is_symbol(expr))
            return senv->get(expr);

        if (!is_pair(expr))
            return expr;

        args = cdr(expr);
        proc = eval(senv, car(expr));

        if (is_proc(proc)) {
            tie(senv, args) = apply(senv, proc, args);
            expr = syntax_begin(senv, args);
            continue;
        }
        switch (Intern opcode = proc) {

        case Intern::_quote:
            return car(args);

        case Intern::_define:
            return syntax_define(senv, args);

        case Intern::_lambda:
            return Proc{ senv, car(args), cdr(args) };

        case Intern::_begin:
            expr = syntax_begin(senv, args);
            break;

        case Intern::_apply:
            if (is_intern(proc = eval(senv, car(args))))
                return call(senv, proc, eval_args(senv, cdr(args), false));

            tie(senv, args) = apply(senv, proc, cdr(args), false);
            expr = syntax_begin(senv, args);
            break;

        case Intern::_setb:
            return syntax_setb(senv, args);

        case Intern::_if:
            expr = syntax_if(senv, args);
            break;

        case Intern::_when:
            expr = syntax_when(senv, args);
            break;

        case Intern::_unless:
            expr = syntax_unless(senv, args);
            break;

        case Intern::_and:
            expr = syntax_and(senv, args);
            break;

        case Intern::_or:
            expr = syntax_or(senv, args);
            break;

        default:
            return call(senv, opcode, eval_args(senv, args));
        }
    }
}
}; // namespace pscm
