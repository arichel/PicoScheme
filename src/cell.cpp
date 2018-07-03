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
#include "proc.hpp"
#include "stream.hpp"
#include "utils.hpp"

namespace pscm {

//! Global cons type store
static std::deque<Cons> store;

static Symtab stab;

static Symenv topenv{
    new Symenv::element_type{
        { stab["#t"], true },
        { stab["#true"], true },
        { stab["#f"], false },
        { stab["#false"], false },
        { stab["or"], Intern::_or },
        { stab["and"], Intern::_and },
        { stab["if"], Intern::_if },
        { stab["quote"], Intern::_quote },
        { stab["begin"], Intern::_begin },
        { stab["cond"], Intern::_cond },
        { stab["define"], Intern::_define },
        { stab["set!"], Intern::_setb },
        { stab["apply"], Intern::_apply },
        { stab["lambda"], Intern::_lambda },

        /*-------------------------------*/
        { stab["cons"], Intern::op_cons },
        { stab["car"], Intern::op_car },
        { stab["cdr"], Intern::op_cdr },
        { stab["set-car!"], Intern::op_setcar },
        { stab["set-cdr!"], Intern::op_setcdr },
        { stab["list"], Intern::op_list },

        { stab["+"], Intern::op_add },
        { stab["-"], Intern::op_sub },
        { stab["*"], Intern::op_mul },
        { stab["/"], Intern::op_div },

    }
};

Cell sym(const char* name) { return stab[name]; }

Cell senv(const Symenv& env)
{
    return std::make_shared<Symenv::element_type>(env ? env : topenv);
}

size_t store_size()
{
    return store.size();
}

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

}; // namespace pscm
