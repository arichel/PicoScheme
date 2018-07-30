/********************************************************************************/ /**
 * @file eval.hpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#include "eval.hpp"
#include "cell.hpp"
#include "parser.hpp"
#include "primop.hpp"
#include "proc.hpp"

namespace pscm::syntax {

/**
 * Evaluate each expression in argument list up the last, which
 * is returned unevaluated. This last expression is evaluated at
 * the call site to support unbound tail-recursion.
 */
Cell _begin(const SymenvPtr& senv, Cell args)
{
    if (is_pair(args)) {
        for (/* */; is_pair(cdr(args)); args = cdr(args))
            eval(senv, car(args));

        return car(args);
    }
    return none;
}

static Cell _if(const SymenvPtr& senv, const Cell& args)
{
    if (is_true(eval(senv, car(args))))
        return cadr(args);
    else if (Cell last = cddr(args); !is_nil(last))
        return car(last);
    else
        return none;
}

/**
 * @brief Scheme syntax cond.
 *
 * @verbatim
 * (cond <clause>_1 <clause>_2 ...)
 *
 * <clause> := (<test> <expression> ...)
 *          |  (<test> => <expression> ...)
 *          |  (else  <expression> ...)
 * @endverbatim
 */
static Cell _cond(const SymenvPtr& senv, Cell args)
{
    Cell test = false, expr = nil;

    // for each clause evaluate <test> condition
    for (/* */; is_pair(args); args = cdr(args)) {
        is_pair(car(args)) || (void(throw std::invalid_argument("invalid cond syntax")), 0);

        if (is_false(test)) {
            test = eval(senv, caar(args));

            if (is_true(test))
                expr = cdar(args);
        }
    }
    if (is_true(test)) {
        if (is_nil(expr))
            return test;

        const Cell& first = car(expr);

        // clause: (<test> => <expr> ...)  -> (apply <expr> <test> nil) ...
        if (is_arrow(first) || (is_symbol(first) && is_arrow(eval(senv, first)))) {
            !is_else(test) || (void(throw std::invalid_argument("invalid cond syntax")), 0);

            Cons cell[4];
            for (expr = cdr(expr); is_pair(cdr(expr)); expr = cdr(expr))
                eval(senv, alist(cell, Intern::_apply, car(expr), test, nil));

            return list(Intern::_apply, car(expr), test, nil);
        } else
            return syntax::_begin(senv, expr);
    }
    return none;
}

static Cell _when(const SymenvPtr& senv, Cell args)
{
    if (is_true(eval(senv, car(args))) && is_pair(args = cdr(args))) {
        for (/* */; is_pair(cdr(args)); args = cdr(args))
            eval(senv, car(args));

        return car(args);
    }
    return none;
}

static Cell _unless(const SymenvPtr& senv, Cell args)
{
    if (is_false(eval(senv, car(args))) && is_pair(args = cdr(args))) {
        for (/* */; is_pair(cdr(args)); args = cdr(args))
            eval(senv, car(args));

        return car(args);
    }
    return none;
}

static Cell _and(const SymenvPtr& senv, Cell args)
{
    Cell res = true;

    if (is_pair(args)) {
        for (/* */; is_pair(cdr(args)); args = cdr(args))
            if (is_false(res = eval(senv, car(args))))
                return res;

        is_nil(cdr(args)) || (void(throw std::invalid_argument("not a proper list")), 0);
        return car(args);
    }
    return res;
}

static Cell _or(const SymenvPtr& senv, Cell args)
{
    Cell res = false;

    if (is_pair(args)) {
        for (/* */; is_pair(cdr(args)); args = cdr(args))
            if (is_true(res = eval(senv, car(args))))
                return res;

        is_nil(cdr(args)) || (void(throw std::invalid_argument("not a proper list")), 0);
        return car(args);
    }
    return res;
}

} // namespace pscm::syntax

namespace pscm {

Cell eval_list(const SymenvPtr& senv, Cell list, bool is_list)
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

    is_nil(tail) || is_pair(tail) || (void(throw std::invalid_argument("invalid apply argument list")), 0);
    return head;
}

/**
 * @brief Evaluate argument list into an argument vector.
 *
 * @param senv Symbol environment, where to evaluate the argument list.
 * @param args Argument list to evaluate.
 * @param is_list true:   procedure call argument list.
 *                false:  apply expression argument list, where the last list item
 *                        must be nil or an argument list itself.
 * @return Vector of evaluated arguments.
 */
static std::vector<Cell> eval_args(const SymenvPtr& senv, Cell args, bool is_list = true)
{
    std::vector<Cell> vec;

    if (is_list) { // expression: (proc x y ... z)
        for (/* */; is_pair(args); args = cdr(args))
            vec.push_back(eval(senv, car(args)));

        return vec;
    }
    // expression: (apply proc x y ... (args ...))
    Cell last = nil;

    // evaluate (x y ...)
    for (/* */; is_pair(args); args = cdr(args))
        vec.push_back(last = eval(senv, car(args)));

    if (is_nil(last)) { // last list (args ...) is nil
        if (!vec.empty())
            vec.pop_back();
        return vec;
    }
    // append arguments from last list (args ...)
    vec.back() = car(last);
    for (args = cdr(last); is_pair(args); args = cdr(args))
        vec.push_back(car(args));

    return vec;
}

Cell eval(SymenvPtr senv, Cell expr)
{
    Cell args, proc;

    for (;;) {
        if (is_symbol(expr))
            return senv->get(get<Symbol>(expr));

        if (!is_pair(expr))
            return expr;

        if (is_func(proc = eval(senv, car(expr))))
            return get<Func>(proc)(senv, eval_args(senv, cdr(expr)));

        if (is_proc(proc)) {
            if (is_macro(proc))
                expr = get<Proc>(proc).expand(expr);
            else {
                tie(senv, args) = get<Proc>(proc).apply(senv, cdr(expr));
                expr = syntax::_begin(senv, args);
            }
            continue;
        }

        args = cdr(expr);
        switch (auto opcode = get<Intern>(proc)) {

        case Intern::_quote:
            return car(args);

        case Intern::_setb:
            senv->set(get<Symbol>(car(args)), eval(senv, cadr(args)));
            return none;

        case Intern::_define:
            if (is_pair(car(args)))
                senv->add(get<Symbol>(caar(args)), Proc{ senv, cdar(args), cdr(args) });
            else
                senv->add(get<Symbol>(car(args)), eval(senv, cadr(args)));
            return none;

        case Intern::_lambda:
            return Proc{ senv, car(args), cdr(args) };

        case Intern::_macro:
            senv->add(get<Symbol>(caar(args)), Proc{ senv, cdar(args), cdr(args), true });
            return none;

        case Intern::_apply:
            if (is_intern(proc = eval(senv, car(args))))
                return call(senv, get<Intern>(proc), eval_args(senv, cdr(args), false));

            if (is_macro(proc))
                expr = get<Proc>(proc).expand(args);
            else {
                tie(senv, args) = get<Proc>(proc).apply(senv, cdr(args), false);
                expr = syntax::_begin(senv, args);
            }
            break;

        case Intern::_begin:
            expr = syntax::_begin(senv, args);
            break;

        case Intern::_if:
            expr = syntax::_if(senv, args);
            break;

        case Intern::_cond:
            expr = syntax::_cond(senv, args);
            break;

        case Intern::_when:
            expr = syntax::_when(senv, args);
            break;

        case Intern::_unless:
            expr = syntax::_unless(senv, args);
            break;

        case Intern::_and:
            expr = syntax::_and(senv, args);
            break;

        case Intern::_or:
            expr = syntax::_or(senv, args);
            break;

        default:
            return call(senv, opcode, eval_args(senv, args));
        }
    }
}

void repl(const SymenvPtr& symenv, std::istream& in, std::ostream& out)
{
    SymenvPtr env = senv(symenv);
    Parser parser;
    Cell expr = none;

    for (;;)
        try {
            for (;;) {
                out << "> ";
                expr = parser.read(in);
                expr = eval(env, expr);

                if (is_intern(expr) && get<Intern>(expr) == Intern::op_exit)
                    return;

                if (!is_none(expr)) {
                    out << expr << std::endl;
                    expr = none;
                }
            }
        } catch (std::exception& e) {
            if (is_none(expr))
                std::cerr << e.what() << std::endl;
            else
                std::cerr << e.what() << ": " << expr << std::endl;
        }
}

} // namespace pscm
