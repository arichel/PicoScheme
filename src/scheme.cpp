/*********************************************************************************/ /**
 * @file scheme.cpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#include <functional>
#include <iomanip>

#include "gc.hpp"
#include "parser.hpp"
#include "primop.hpp"
#include "scheme.hpp"

namespace pscm {

using namespace std::string_literals;

static_assert(std::is_same_v<Char, String::value_type>);
static_assert(std::is_same_v<String, StringPtr::element_type>);
static_assert(std::is_same_v<String, Symbol::value_type>);
static_assert(std::is_same_v<Port<Char>, PortPtr::element_type>);
static_assert(std::is_same_v<Symbol, Symtab::Symbol>);
static_assert(std::is_same_v<Symenv, SymenvPtr::element_type>);
static_assert(std::is_same_v<Function, FunctionPtr::element_type>);

StringPtr mkstr(const String& s) { return std::make_shared<String>(s); }
StringPtr mkstr(const Char* s) { return std::make_shared<String>(s); }

RegexPtr mkregex(const String& str)
{
    using regex = RegexPtr::element_type;
    regex::flag_type flags = regex::ECMAScript | regex::icase;
    return std::make_shared<RegexPtr::element_type>(str, flags);
}

VectorPtr mkvec(Number size, const Cell& val)
{
    using size_type = VectorPtr::element_type::size_type;

    (is_int(size) && get<Int>(size) >= 0)
        || ((void)(throw std::invalid_argument("vector length must be a non-negative integer")), 0);

    return std::make_shared<VectorPtr::element_type>(static_cast<size_type>(get<Int>(size)), val);
}

Symbol Scheme::mksym(const String& name) { return this->symtab[name]; }
Symbol Scheme::mksym(const Char* name) { return this->symtab[name]; }

Symbol Scheme::mksym()
{
    return symtab[String{ "symbol " }.append(std::to_string(symtab.size()))];
}

SymenvPtr Scheme::mkenv(const SymenvPtr& env)
{
    return Symenv::create(env ? env : topenv);
}

FunctionPtr Scheme::mkfun(Function::function_type&& fn)
{
    return std::make_shared<Function>(mksym("lambda"), std::move(fn));
}

FunctionPtr Scheme::mkfun(const String& name, Function::function_type&& fn, const SymenvPtr& env)
{
    auto sym = mksym(name);
    auto fptr = std::make_shared<Function>(sym, std::move(fn));

    if (env)
        env->add(sym, fptr);
    else
        topenv->add(sym, fptr);

    return fptr;
}

Cons* Scheme::cons(Cell&& car, Cell&& cdr) { return pscm::cons(store, std::move(car), std::move(cdr)); }
Cons* Scheme::cons(Cell&& car, const Cell& cdr) { return pscm::cons(store, std::move(car), cdr); }
Cons* Scheme::cons(const Cell& car, Cell&& cdr) { return pscm::cons(store, car, std::move(cdr)); }
Cons* Scheme::cons(const Cell& car, const Cell& cdr) { return pscm::cons(store, car, cdr); }

Cell Scheme::apply(const SymenvPtr& env, Intern opcode, const std::vector<Cell>& args)
{
    return pscm::call(*this, env, opcode, args);
}

Cell Scheme::apply(const SymenvPtr& env, const FunctionPtr& proc, const std::vector<Cell>& args)
{
    return (*proc)(*this, env, args);
}

Cell Scheme::apply(const SymenvPtr& env, const Cell& cell, const std::vector<Cell>& args)
{
    if (is_intern(cell))
        return apply(env, get<Intern>(cell), args);
    else
        return apply(env, get<FunctionPtr>(cell), args);
}

std::pair<SymenvPtr, Cell> Scheme::apply(const SymenvPtr& env, const Cell& proc, const Cell& args, bool is_list)
{
    return get<Procedure>(proc).apply(*this, env, args, is_list);
}

Cell Scheme::expand(const Cell& macro, Cell& args)
{
    return get<Procedure>(macro).expand(*this, args);
}

void Scheme::repl(const SymenvPtr& env)
{
    using Port = StandardPort<Char>;

    SymenvPtr senv = mkenv(env ? env : topenv);
    Parser parser(*this);
    Cell expr;

    Port in{ Port::in }, out{ Port::out };
    for (;;)
        try {
            for (;;) {
                out << "> ";
                expr = none;
                expr = parser.read(in);
                expr = eval(senv, expr);

                if (is_none(expr))
                    continue;

                if (is_exit(expr))
                    return;

                out << expr << std::endl;
            }
        } catch (std::exception& e) {
            if (is_none(expr))
                out << e.what() << std::endl;
            else
                out << e.what() << ": " << expr << std::endl;
        }
}

void Scheme::load(const std::string& filnam, const SymenvPtr& symenv)
{
    using file_port = FilePort<Char>;

    const SymenvPtr& env = symenv ? symenv : topenv;

    Parser parser(*this);
    Cell expr = none;

    try {
        file_port in{ filnam, file_port::in };
        if (!in.is_open())
            throw std::ios_base::failure("couldn't open input file: '"s + filnam + "'"s);

        while (!in.eof()) {
            expr = parser.read(in);
            expr = eval(env, expr);
            expr = none;
        }
    } catch (const std::exception& e) {
        if (is_none(expr))
            std::cerr << e.what() << '\n';
        else
            std::cerr << e.what() << ": " << expr << '\n';
    }
}

void Scheme::add_contants(Scheme& scm, const SymenvPtr& env)
{
    // clang-format off
    env->add(
        { { scm.mksym(u8"π"),    mknum(pi<Float>)       },
          { scm.mksym("%pi"),    mknum(pi<Float>)       },
          { scm.mksym("%e"),     mknum(e<Float>)        },
          { scm.mksym("%G"),     mknum(G<Float>)        },
          { scm.mksym("%c"),     mknum(c<Float>)        },
          { scm.mksym("%h"),     mknum(h<Float>)        },
          { scm.mksym("%qe"),    mknum(q_e<Float>)      },
          { scm.mksym("%NA"),    mknum(N_A<Float>)      },
          { scm.mksym("%R"),     mknum(R<Float>)        },
          { scm.mksym("%mu0"),   mknum(mu_0<Float>)     },
          { scm.mksym("%eps0"),  mknum(epsilon_0<Float>)},
          { scm.mksym("%sigma"), mknum(sigma<Float>)    },
         });
    // clang-format on
}

Cell Scheme::syntax_begin(const SymenvPtr& env, Cell args)
{
    if (is_pair(args)) {
        for (/* */; is_pair(cdr(args)); args = cdr(args))
            eval(env, car(args));

        return car(args);
    }
    return none;
}

Cell Scheme::syntax_if(const SymenvPtr& env, const Cell& args)
{
    if (is_true(eval(env, car(args))))
        return cadr(args);

    else if (const Cell& last = cddr(args); !is_nil(last))
        return car(last);

    else
        return none;
}

Cell Scheme::syntax_cond(const SymenvPtr& env, Cell args)
{
    Cell test = false, expr = nil;

    // For each clause evaluate <test> condition
    for (/* */; is_pair(args); args = cdr(args)) {
        is_pair(car(args)) || (void(throw std::invalid_argument("invalid cond syntax")), 0);

        if (is_false(test)) {
            test = eval(env, caar(args));

            if (is_true(test))
                expr = cdar(args);
        }
    }
    if (is_true(test)) {
        if (is_nil(expr))
            return test;

        const Cell& first = car(expr);

        // clause: (<test> => <expr> ...)  -> (apply <expr> <test> nil) ...
        if (is_arrow(first) || (is_symbol(first) && is_arrow(eval(env, first)))) {
            !is_else(test) || (void(throw std::invalid_argument("invalid cond syntax")), 0);

            Cons cons[4], argv[2];
            Cell apply_expr = pscm::list(cons, Intern::_apply, none,
                pscm::list(argv, Intern::_quote, test), nil);

            // For each expression, first replace none in apply_expr and then call eval:
            for (expr = cdr(expr); is_pair(cdr(expr)); expr = cdr(expr)) {
                set_car(cdr(apply_expr), car(expr));
                eval(env, apply_expr);
            }
            // Return last expression to evaluated at the call site to maintain unbound tail-recursion:
            return list(Intern::_apply, car(expr), list(Intern::_quote, test), nil);
        } else
            return syntax_begin(env, expr);
    }
    return none;
}

Cell Scheme::syntax_when(const SymenvPtr& env, Cell args)
{
    if (is_true(eval(env, car(args))) && is_pair(args = cdr(args))) {
        for (/* */; is_pair(cdr(args)); args = cdr(args))
            eval(env, car(args));

        return car(args);
    }
    return none;
}

Cell Scheme::syntax_unless(const SymenvPtr& env, Cell args)
{
    if (is_false(eval(env, car(args))) && is_pair(args = cdr(args))) {
        for (/* */; is_pair(cdr(args)); args = cdr(args))
            eval(env, car(args));

        return car(args);
    }
    return none;
}

Cell Scheme::syntax_and(const SymenvPtr& env, Cell args)
{
    Cell res = true;

    if (is_pair(args)) {
        for (/* */; is_pair(cdr(args)); args = cdr(args))
            if (is_false(res = eval(env, car(args))))
                return res;

        is_nil(cdr(args)) || (void(throw std::invalid_argument("not a proper list")), 0);
        return car(args);
    }
    return res;
}

Cell Scheme::syntax_or(const SymenvPtr& env, Cell args)
{
    Cell res = false;

    if (is_pair(args)) {
        for (/* */; is_pair(cdr(args)); args = cdr(args))
            if (is_true(res = eval(env, car(args))))
                return list(Intern::_quote, res);

        is_nil(cdr(args)) || (void(throw std::invalid_argument("not a proper list")), 0);
        return car(args);
    }
    return res;
}

Cell Scheme::eval_list(const SymenvPtr& env, Cell list, bool is_list)
{
    if (!is_pair(list))
        return nil;

    if (is_list) {
        Cell head = cons(eval(env, car(list)), cdr(list));
        list = cdr(list);

        for (Cell tail = head; is_pair(list); tail = cdr(tail), list = cdr(list))
            set_cdr(tail, cons(eval(env, car(list)), cdr(list)));

        return head;
    }
    Cell tail, head;

    if (is_pair(cdr(list)))
        head = cons(eval(env, car(list)), cdr(list));
    else
        head = eval(env, car(list));

    for (tail = head, list = cdr(list); is_pair(list); tail = cdr(tail), list = cdr(list))
        if (is_pair(cdr(list)))
            set_cdr(tail, cons(eval(env, car(list)), cdr(list)));
        else
            set_cdr(tail, eval(env, car(list)));

    is_nil(tail) || is_pair(tail)
        || (void(throw std::invalid_argument("invalid apply argument list")), 0);
    return head;
}

std::vector<Cell> Scheme::eval_args(const SymenvPtr& env, Cell args, bool is_list)
{
    std::vector<Cell> stack;

    if (is_list) { // expression: (proc x y ... z)
        for (/* */; is_pair(args); args = cdr(args))
            stack.push_back(eval(env, car(args)));

        return stack;
    }
    // expression: (apply proc x y ... (args ...))
    Cell last = nil;

    // evaluate (x y ...)
    for (/* */; is_pair(args); args = cdr(args))
        stack.push_back(last = eval(env, car(args)));

    if (is_nil(last)) { // last list (args ...) is nil
        if (!stack.empty())
            stack.pop_back();
        return stack;
    }
    // append arguments from last list (args ...)
    stack.back() = car(last);
    for (args = cdr(last); is_pair(args); args = cdr(args))
        stack.push_back(car(args));

    return stack;
}

Cell Scheme::eval(SymenvPtr env, Cell expr)
{
    Cell args, proc;

    for (;;) {
        if (is_symbol(expr))
            return env->get(get<Symbol>(expr));

        if (!is_pair(expr))
            return expr;

        if (is_func(proc = eval(env, car(expr))))
            return apply(env, proc, eval_args(env, cdr(expr)));

        if (is_proc(proc)) {
            if (is_macro(proc))
                expr = expand(proc, expr);
            else {
                tie(env, args) = apply(env, proc, cdr(expr));
                expr = syntax_begin(env, args);
            }
            continue;
        }
        args = cdr(expr);
        switch (auto opcode = get<Intern>(proc)) {

        case Intern::_quote:
            return car(args);

        case Intern::_setb:
            env->set(get<Symbol>(car(args)), eval(env, cadr(args)));
            return none;

        case Intern::_define:
            if (is_pair(car(args)))
                env->add(get<Symbol>(caar(args)), Procedure{ env, cdar(args), cdr(args) });
            else
                env->add(get<Symbol>(car(args)), eval(env, cadr(args)));
            return none;

        case Intern::_lambda:
            return Procedure{ env, car(args), cdr(args) };

        case Intern::_macro:
            env->add(get<Symbol>(caar(args)), Procedure{ env, cdar(args), cdr(args), true });
            return none;

        case Intern::_apply:
            if (is_proc(proc = eval(env, car(args)))) {
                if (is_macro(proc))
                    expr = expand(proc, args);
                else {
                    tie(env, args) = apply(env, proc, cdr(args), false);
                    expr = syntax_begin(env, args);
                }
                break;
            } else
                return apply(env, proc, eval_args(env, cdr(args), false));

        case Intern::_begin:
            expr = syntax_begin(env, args);
            break;

        case Intern::_if:
            expr = syntax_if(env, args);
            break;

        case Intern::_cond:
            expr = syntax_cond(env, args);
            break;

        case Intern::_when:
            expr = syntax_when(env, args);
            break;

        case Intern::_unless:
            expr = syntax_unless(env, args);
            break;

        case Intern::_and:
            expr = syntax_and(env, args);
            break;

        case Intern::_or:
            expr = syntax_or(env, args);
            break;

        default:
            return apply(env, opcode, eval_args(env, args));
        }
    }
}

} // namespace pscm
