#include <set>

#include "scheme.hpp"

namespace pscm {

/**
 *  Test argument list for unique symbols.
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

    std::set<Symbol> symset;

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
struct Procedure::Closure {

    Closure(const SymenvPtr& senv, const Cell& args, const Cell& code, bool is_macro)
        : senv{ senv }
        , args{ args }
        , code{ code }
        , is_macro{ is_macro }
    {
        if (!is_unique_symbol_list(args) || !is_pair(code))
            throw std::invalid_argument("invalid procedure definition");
    }

    bool operator!=(const Closure& impl) const noexcept
    {
        return senv != impl.senv
            || args != impl.args
            || code != impl.code
            || is_macro != impl.is_macro;
    }
    SymenvPtr senv; //!< Symbol environment pointer.
    Cell args; //!< Formal parameter symbol list or single symbol.
    Cell code; //!< Lambda body expression list.
    bool is_macro;
};

Procedure::Procedure(Procedure&&) noexcept = default;
Procedure& Procedure::operator=(Procedure&&) noexcept = default;
Procedure::~Procedure() = default;

Procedure::Procedure(const SymenvPtr& senv, const Cell& args, const Cell& code, bool is_macro)
    : impl{ std::make_unique<Closure>(senv, args, code, is_macro) }
{
}

Procedure::Procedure(const Procedure& proc)
    : impl{ std::make_unique<Closure>(*proc.impl) }
{
}

bool Procedure::is_macro() const noexcept
{
    return impl->is_macro;
}

Procedure& Procedure::operator=(const Procedure& proc)
{
    *impl = *proc.impl;
    return *this;
}

bool Procedure::operator!=(const Procedure& proc) const noexcept
{
    return *impl != *proc.impl;
}

bool Procedure::operator==(const Procedure& proc) const noexcept
{
    return !(*impl != *proc.impl);
}

/**
 * First evaluate items in the argument list in the current environment senv.
 * Assign the result to symbols of the closure formal parameter list into
 * a new child environment of the previously captured closure environment.
 *
 * @remark A dotted formal parameter list or a single symbol argument
 *         requires additional cell-storage to build the evaluated
 *         argument list.
 */
std::pair<SymenvPtr, Cell> Procedure::apply(Scheme& scm, const SymenvPtr& env, Cell args, bool is_list) const
{
    // Create a new child environment and set the closure environment as father:
    SymenvPtr newenv = scm.mkenv(impl->senv);

    Cell iter = impl->args; // closure formal parameter symbol list

    if (is_list) { // Evaluate each list item of a (lambda args body) expression argument list:
        for (/* */; is_pair(iter) && is_pair(args); iter = cdr(iter), args = cdr(args))
            newenv->add(get<Symbol>(car(iter)), scm.eval(env, car(args)));

        // Handle the last symbol of a dotted formal parameter list or a single symbol lambda
        // argument. This symbol is assigned to the evaluated list of remaining expressions
        // which requires additional cons-cell storage.
        if (iter != args)
            newenv->add(get<Symbol>(iter), scm.eval_list(env, args, is_list));
    } else {
        // Evaluate each argument of a (apply proc x y ... args) expression and add to newenv:
        for (/* */; is_pair(iter) && is_pair(cdr(args)); iter = cdr(iter), args = cdr(args))
            newenv->add(get<Symbol>(car(iter)), scm.eval(env, car(args)));

        if (is_nil(cdr(args))) {
            args = scm.eval(env, car(args)); // last list item must evaluate to nil or a list

            // Add each list item of this list to newenv:
            for (/* */; is_pair(iter) && is_pair(args); iter = cdr(iter), args = cdr(args))
                newenv->add(get<Symbol>(car(iter)), car(args));

            if (iter != args) // dottet formal parmeter list:
                newenv->add(get<Symbol>(iter), args);
        } else
            newenv->add(get<Symbol>(iter), scm.eval_list(env, args, is_list));
    }
    return { newenv, impl->code };
}

/**
 * @brief Expand a macro
 */
Cell Procedure::expand(Scheme& scm, Cell& expr) const
{
    is_macro() || (void(throw std::invalid_argument("expand - not a macro")), 0);

    Cell args = cdr(expr), iter = impl->args; // macro formal parameter symbol list

    // Create a new child environment and set the closure environment as father:
    SymenvPtr newenv = scm.mkenv(impl->senv);

    // Add unevaluated macro parameters to new environment:
    for (/* */; is_pair(iter) && is_pair(args); iter = cdr(iter), args = cdr(args))
        newenv->add(get<Symbol>(car(iter)), car(args));

    if (iter != args)
        newenv->add(get<Symbol>(iter), args);

    // Expand and replace argument expression with evaluated macro:
    set_car(expr, Intern::_begin);
    set_car(cdr(expr), args = scm.eval(newenv, scm.syntax_begin(newenv, impl->code)));
    set_cdr(cdr(expr), nil);
    return args;
}

Function::Function(const Symbol& sym, const function_type& fun)
    : function_type{ fun }
    , sym{ sym }
{
}

Function::Function(const Symbol& sym, function_type&& fun)
    : function_type{ std::move(fun) }
    , sym{ sym }
{
}

} // namespace pscm
