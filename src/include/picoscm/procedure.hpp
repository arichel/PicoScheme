#ifndef PROCEDURE_HPP
#define PROCEDURE_HPP

#include <functional>

#include "types.hpp"

namespace pscm {

class Scheme;

/**
 * Procedure type to represent a scheme closure.
 *
 * @verbatim
 * (lambda args body)  => closure: [symenv, args, body](define *world* '())
 *  with:
 *  args        := [symbol | nil | list | dotted-list]
 *  list        := nil | (expr . list)
 *  dotted-list := (list . expr)
 *  body        := (expr . list)
 * @endverbatim
 */
class Procedure {
public:
    /**
     * Construct a new closure.
     * @param senv  Symbol environment pointer to capture.
     * @param args  Formal lambda expression argument list or symbol.
     * @param code  Non empty list of one or more scheme expression forming the lambda body.
     */
    Procedure(const SymenvPtr& senv, const Cell& args, const Cell& code, bool is_macro = false);
    Procedure(const Procedure& proc);
    Procedure(Procedure&& proc) noexcept;
    ~Procedure();

    /// Predicate returns true if closure should be applied as macro.
    bool is_macro() const noexcept;

    Procedure& operator=(const Procedure&);
    Procedure& operator=(Procedure&&) noexcept;

    bool operator!=(const Procedure& proc) const noexcept;
    bool operator==(const Procedure& proc) const noexcept;

    /**
     * Closure application.
     *
     * @param senv  Current environment, where to evaluate expressions of the argument list.
     * @param args  Argument expression list of a scheme lambda or apply expression.
     *
     * @param is_list
     *        true:  args is a (lambda args body) expression argument list
     *        false: args is a (apply proc args) expression argument list,
     *               where the last element in expr must be nil or a list itself:
     *               (arg_0 ... arg_i  [list | nil])
     *
     * @return New child environment of the closure parent environment and the closure body
     *         expression list.
     */
    std::pair<SymenvPtr, Cell> apply(Scheme& scm, const SymenvPtr& env, Cell args, bool is_list = true) const;

    /**
     * Replace expression with the expanded closure macro.
     * @param expr (closure-macro arg0 ... arg_n)
     * @return The expanded macro body.
     */
    Cell expand(Scheme& scm, Cell& expr) const;

private:
    struct Closure;
    std::unique_ptr<Closure> impl;
};

/**
 * Functor wrapper for external function objects.
 *
 * External function signature:  func(Scheme& scm, SymenvPtr& env, std::vector<Cell> argv) -> Cell
 */
struct Function : public std::function<Cell(Scheme&, const SymenvPtr&, const std::vector<Cell>&)> {

    using function_type = std::function<Cell(Scheme&, const SymenvPtr&, const std::vector<Cell>&)>;
    /**
     * Function object constructor
     * @param sym Symbol bound to this function.
     * @param fun External procedure.
     */
    Function(const Symbol&, const function_type&);
    Function(const Symbol&, function_type&&);

    const Symbol::value_type& name() const { return sym.value(); };

private:
    const Symbol& sym;
};

} // namespace pscm

#endif // PROCEDURE_HPP
