/*********************************************************************************/ /**
 * @file cell.cpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#include <assert.h>
#include <deque>

#include "cell.hpp"
#include "eval.hpp"
#include "stream.hpp"
#include "utils.hpp"

namespace pscm {

using std::get;

//! Global cons type store
static std::deque<Cons> store;
static Symtab stab;

Cell sym(const char* name) { return stab[name]; }

//! Return a new cons-cell from the global cons-store
//! A cons-cell is basically a type tagged pointer into the global cons-store.
template <typename CAR, typename CDR, typename Store = std::deque<Cons>>
static Cons* cons(Store& store, CAR&& car, CDR&& cdr)
{
    return &store.emplace_back(std::forward<CAR>(car), std::forward<CDR>(cdr));
}

Cell cons(Cell&& car, Cell&& cdr) { return cons(store, std::move(car), std::move(cdr)); }
Cell cons(Cell&& car, const Cell& cdr) { return cons(store, std::move(car), cdr); }
Cell cons(const Cell& car, Cell&& cdr) { return cons(store, car, std::move(cdr)); }
Cell cons(const Cell& car, const Cell& cdr) { return cons(store, car, cdr); }

struct Procedure {
    Procedure(const Symenv& senv, const Cell& args, const Cell& code)
    {
        if (is_unique_symbol_list(args) && is_pair(code)) {
            _senv = senv;
            _args = args;
            _code = code;
        } else
            throw std::invalid_argument("invalid procedure definition");
    }

    std::tuple<Symenv, Cell, Cell> apply(const Symenv& senv, Cell args) const
    {
        auto newenv = std::make_shared<Symenv::element_type>(_senv);

        Cell iter = _args;
        for (/* */; is_pair(iter) && is_pair(args); iter = cdr(iter), args = cdr(args))
            newenv->add(car(iter), eval(senv, car(args)));

        if (iter != args) {
            is_symbol(iter) || (throw std::invalid_argument("invalid procedure arguments"), 0);

            newenv->add(iter, eval_list(senv, args));
        }
        return { newenv, Intern::_begin, _code };
    }

private:
    bool is_unique_symbol_list(Cell args)
    {
        using std::get;

        if (is_nil(args) || is_symbol(args))
            return true;

        std::set<Symbol::key_type> symset;

        for (/* */; is_pair(args); args = cdr(args)) {
            Cell sym = car(args);

            if (!is_symbol(sym) || !symset.insert(get<Symbol>(sym)).second)
                return false;
        }
        return is_nil(args) || (is_symbol(args) && symset.insert(get<Symbol>(args)).second);
    }
    Symenv _senv;
    Cell _args = nil;
    Cell _code = nil;
};

Cell lambda(const Symenv& senv, const Cell& args, const Cell& code)
{
    return std::make_shared<Proc::element_type>(senv, args, code);
}

std::tuple<Symenv, Cell, Cell> apply(const Symenv& senv, const Proc& proc, const Cell& args)
{
    return proc->apply(senv, args);
}

bool is_list(Cell cell)
{
    for (Cell slow{ cell }; is_pair(cell); cell = cdr(cell), slow = cdr(slow))
        if (!is_pair(cell = cdr(cell)) || cell == slow)
            break;
    return is_nil(cell);
}

Int list_length(Cell list)
{
    Int len = 0, slw = 0;

    for (Cell slow{ list }; is_pair(list); list = cdr(list), slow = cdr(slow), ++len, ++slw) {
        ++len;
        if (!is_pair(list = cdr(list)))
            break;

        if (list == slow)
            return ++slw;
    }
    return len;
}

Cell list_ref(Cell list, Int k)
{
    for (/* */; k > 0 && is_pair(list); list = cdr(list), --k)
        ;

    !k || (throw std::invalid_argument("invalid list length"), 0);
    return car(list);
}

//void foreach (Func fun, Cell list)
//{
//    for (Cell slow{ list }; is_pair(list); list = cdr(list), slow = cdr(slow)) {
//        fun(car(list));

//        if (!is_pair(list = cdr(list)) || list == slow)
//            break;

//        fun(car(list));
//    }
//}

//Cell fun_foreach(const Cell& args)
//{
//    auto fun = get<Func>(car(args));

//    foreach (fun, cdr(args))
//        ;
//    return none;
//}

Cell fun_write(const std::vector<Cell>& args)
{
    Port* port = args.size() > 1 ? get<Port*>(args[1]) : &std::cout;

    *port << args.at(0);
    return none;
}

Cell fun_add(const std::vector<Cell>& args)
{
    Number sum = 0;

    for (Number val : args)
        sum += val;

    return sum;
}

}; // namespace pscm
