#include <set>

#include "cell.hpp"
#include "eval.hpp"
#include "proc.hpp"

namespace pscm {

/**
 * @brief Test argument list for unique symbols.
 *
 *  Predicate is used to check, that formal parameters of lambda
 *  expression  @verbatim (lambda (x y z ... x) code...) @endverbatim
 *  do not repeat in the argument list.
 */
static bool is_unique_symbol_list(Cell args)
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

/**
 * @brief Closure to capture an environment pointer, a formal
 *        argument list and a code list of one or more scheme expressions.
 */
struct Proc::Closure {

    Closure(const Symenv& senv, const Cell& args, const Cell& code)
        : senv{ senv }
        , args{ args }
        , code{ code }
    {
        if (!is_unique_symbol_list(args) || !is_pair(code))
            throw std::invalid_argument("invalid procedure definition");
    }

    bool operator!=(const Closure& impl) const noexcept
    {
        return senv != impl.senv
            && args != impl.args
            && code != impl.code;
    }

    Symenv senv; //!< Symbol environment pointer.
    Cell args; //!< Formal parameter symbol list or single symbol.
    Cell code; //!< Lambda body expression list.
};

Proc::Proc(Proc&&) noexcept = default;
Proc& Proc::operator=(Proc&&) noexcept = default;
Proc::~Proc() = default;

Proc::Proc(const Symenv& senv, const Cell& args, const Cell& code)
    : impl{ std::make_unique<Closure>(senv, args, code) }
{
}

Proc::Proc(const Proc& proc)
    : impl{ std::make_unique<Closure>(*proc.impl) }
{
}

Proc& Proc::operator=(const Proc& proc)
{
    *impl = *proc.impl;
    return *this;
}

bool Proc::operator!=(const Proc& proc) const noexcept
{
    return *impl != *proc.impl;
}

bool Proc::operator==(const Proc& proc) const noexcept
{
    return !(*impl != *proc.impl);
}

/**
 * Evaluate each argument list item in the current environment and assign
 * the result to symbols of the closure formal parameter list in a new
 * child environment of the previously captured closure environment.
 *
 * @remark A dotted formal parameter list or a single symbol argument
 *         requires additional cell-storage to build the evaluated
 *         argument list.
 */
std::pair<Symenv, Cell> Proc::apply(const Symenv& senv, Cell args, bool is_list) const
{
    // Create a new child environment and set the closure environment as father:
    Symenv newenv = std::make_shared<Symenv::element_type>(impl->senv);

    Cell iter = impl->args; // closure formal parameter symbol list

    if (is_list) { // Evaluate each list item of a (lambda args body) expression argument list:
        for (/* */; is_pair(iter) && is_pair(args); iter = cdr(iter), args = cdr(args))
            newenv->add(get<Symbol>(car(iter)), eval(senv, car(args)));

        // Handle the last symbol of a dotted formal parameter list or a single symbol lambda
        // argument. This symbol is assigned to the evaluated list of remaining expressions
        // which requires additional cons-cell storage.
        if (iter != args)
            newenv->add(get<Symbol>(iter), eval_list(senv, args, is_list));
    } else

        // Evaluate each argument of a (apply proc x y ... args) expression and add to newenv:
        for (/* */; is_pair(iter) && is_pair(args); iter = cdr(iter), args = cdr(args))
            if (is_pair(cdr(args)))
                newenv->add(get<Symbol>(car(iter)), eval(senv, car(args)));
            else {
                args = eval(senv, car(args)); // last list item must evaluate to nil or a list

                // Add each list item of this list to newenv:
                for (/* */; is_pair(iter) && is_pair(args); iter = cdr(iter), args = cdr(args))
                    newenv->add(get<Symbol>(car(iter)), car(args));

                if (iter != args) // dottet formal parmeter list:
                    newenv->add(get<Symbol>(iter), args);
            }

    return { newenv, impl->code };
}

std::pair<Symenv, Cell> apply(const Symenv& senv, const Proc& proc, const Cell& args, bool is_list)
{
    return proc.apply(senv, args, is_list);
}

}; // namespace pscm
