#ifndef PROCEDURE_HPP
#define PROCEDURE_HPP

#include <functional>
#include <memory>
#include <utility>

#include "symbol.hpp"

namespace pscm {

struct Cell;
class Scheme;

using Symtab = SymbolTable<std::string>;
using Symbol = Symtab::Symbol;
using Symenv = SymbolEnv<Symbol, Cell>;
using SymenvPtr = std::shared_ptr<Symenv>;

/**
 * @brief Procedure type to represent a scheme closure.
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
     * @brief Construct a new closure.
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
     * @brief Closure application.
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
     * @brief Replace expression with the expanded closure macro.
     * @param expr (closure-macro arg0 ... arg_n)
     * @return The expanded macro body.
     */
    Cell expand(Scheme& scm, Cell& expr) const;

private:
    struct Closure;
    std::unique_ptr<Closure> impl;
};

class Function {
public:
    using function_type = std::function<Cell(Scheme& scm, const SymenvPtr&, const std::vector<Cell>&)>;

    Function(const Symbol& sym, function_type&& fun);

    Cell operator()(Scheme& scm, const SymenvPtr& senv, const std::vector<Cell>& args) const;

    const std::string& name() const;

private:
    const Symbol::value_type* valptr;
    function_type func;
};

}; // namespace pscm

#endif // PROCEDURE_HPP
