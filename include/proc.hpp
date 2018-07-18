#ifndef PROC_HPP
#define PROC_HPP

#include <memory>
#include <utility>

#include "symbol.hpp"

namespace pscm {

struct Cell;
using Symbol = SymbolTable<std::string>::Symbol;
using Symenv = std::shared_ptr<SymbolEnv<Symbol, Cell>>;

/**
 * @brief Procedure type to represent a scheme closure.
 *
 * @verbatim
 * (lambda args body)  => closure: [symenv, args, body]
 *
 *  with:
 *  args        := [symbol | nil | list | dotted-list]
 *  list        := nil | (expr . list)
 *  dotted-list := (list . expr)
 *  body        := (expr . list)
 * @endverbatim
 */
class Proc {
public:
    /**
     * @brief Construct a new closure.
     * @param senv  Symbol environment pointer to capture.
     * @param args  Formal lambda expression argument list or symbol.
     * @param code  Non empty list of one or more scheme expression forming the lambda body.
     */
    Proc(const Symenv& senv, const Cell& args, const Cell& code);
    Proc(const Proc& proc);
    Proc(Proc&& proc) noexcept;
    ~Proc();

    Proc& operator=(const Proc&);
    Proc& operator=(Proc&&) noexcept;

    bool operator!=(const Proc& proc) const noexcept;
    bool operator==(const Proc& proc) const noexcept;

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
    std::pair<Symenv, Cell> apply(const Symenv& senv, Cell args, bool is_list = true) const;

private:
    struct Closure;
    std::unique_ptr<Closure> impl;
};

/**
 * @brief Conveniance function to call the Proc::apply member function.
 */
std::pair<Symenv, Cell> apply(const Symenv& senv, const Proc& proc, const Cell& args, bool is_list = true);

}; // namespace pscm

#endif // PROC_HPP
