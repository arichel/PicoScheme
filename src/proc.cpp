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
            && args != impl.args
            && code != impl.code
            && is_macro != impl.is_macro;
    }
    SymenvPtr senv; //!< Symbol environment pointer.
    Cell args; //!< Formal parameter symbol list or single symbol.
    Cell code; //!< Lambda body expression list.
    bool is_macro;
};

Proc::Proc(Proc&&) noexcept = default;
Proc& Proc::operator=(Proc&&) noexcept = default;
Proc::~Proc() = default;

Proc::Proc(const SymenvPtr& senv, const Cell& args, const Cell& code, bool is_macro)
    : impl{ std::make_unique<Closure>(senv, args, code, is_macro) }
{
}

Proc::Proc(const Proc& proc)
    : impl{ std::make_unique<Closure>(*proc.impl) }
{
}

bool Proc::is_macro() const noexcept
{
    return impl->is_macro;
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
 * First evaluate each argument list item in the current environment senv.
 * Assign the result to symbols of the closure formal parameter list into
 * a new child environment of the previously captured closure environment.
 *
 * @remark A dotted formal parameter list or a single symbol argument
 *         requires additional cell-storage to build the evaluated
 *         argument list.
 */
std::pair<SymenvPtr, Cell> Proc::apply(const SymenvPtr& senv, Cell args, bool is_list) const
{
    // Create a new child environment and set the closure environment as father:
    SymenvPtr newenv = std::make_shared<SymenvPtr::element_type>(impl->senv);

    Cell iter = impl->args; // closure formal parameter symbol list

    if (is_list) { // Evaluate each list item of a (lambda args body) expression argument list:
        for (/* */; is_pair(iter) && is_pair(args); iter = cdr(iter), args = cdr(args))
            newenv->add(get<Symbol>(car(iter)), eval(senv, car(args)));

        // Handle the last symbol of a dotted formal parameter list or a single symbol lambda
        // argument. This symbol is assigned to the evaluated list of remaining expressions
        // which requires additional cons-cell storage.
        if (iter != args)
            newenv->add(get<Symbol>(iter), eval_list(senv, args, is_list));
    } else {
        // Evaluate each argument of a (apply proc x y ... args) expression and add to newenv:
        for (/* */; is_pair(iter) && is_pair(cdr(args)); iter = cdr(iter), args = cdr(args))
            newenv->add(get<Symbol>(car(iter)), eval(senv, car(args)));

        if (is_nil(cdr(args))) {
            args = eval(senv, car(args)); // last list item must evaluate to nil or a list

            // Add each list item of this list to newenv:
            for (/* */; is_pair(iter) && is_pair(args); iter = cdr(iter), args = cdr(args))
                newenv->add(get<Symbol>(car(iter)), car(args));

            if (iter != args) // dottet formal parmeter list:
                newenv->add(get<Symbol>(iter), args);
        } else
            newenv->add(get<Symbol>(iter), eval_list(senv, args, is_list));
    }
    return { newenv, impl->code };
}

/**
 * @brief Expand a macro
 */
Cell Proc::expand(Cell& expr) const
{
    Cell args = cdr(expr), iter = impl->args; // macro formal parameter symbol list

    // Create a new child environment and set the closure environment as father:
    SymenvPtr newenv = std::make_shared<SymenvPtr::element_type>(impl->senv);

    // Add unevaluated macro parameters to new environment:
    for (/* */; is_pair(iter) && is_pair(args); iter = cdr(iter), args = cdr(args))
        newenv->add(get<Symbol>(car(iter)), car(args));

    if (iter != args)
        newenv->add(get<Symbol>(iter), args);

    // Expand and replace argument expression with evaluated macro:
    set_car(expr, Intern::_begin);
    set_car(cdr(expr), args = eval(newenv, syntax::_begin(newenv, impl->code)));
    set_cdr(cdr(expr), nil);
    return args;
}

std::pair<SymenvPtr, Cell> apply(const SymenvPtr& senv, const Proc& proc, const Cell& args, bool is_list)
{
    return proc.apply(senv, args, is_list);
}

Func::Func(const Symbol& sym, function_type&& fun)
    : valptr{ &sym.value() }
    , func{ std::move(fun) }
{
}

Cell Func::operator()(const SymenvPtr& senv, const std::vector<Cell>& args) const
{
    return func(senv, args);
}

const std::string& Func::name() const
{
    return static_cast<const std::string&>(*valptr);
}

} // namespace pscm
