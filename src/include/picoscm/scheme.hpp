/********************************************************************************/ /**
 * @file scheme.hpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#ifndef SCHEME_HPP
#define SCHEME_HPP

#include <list>

#include "cell.hpp"
#include "gc.hpp"

namespace pscm {

class GCollector;

/**
 * Scheme interpreter class.
 */
class Scheme {
public:
    //! Optional connect this scheme interpreter to the environment of another interpreter.
    Scheme(const SymenvPtr& env = nullptr);

    //! Return a shared pointer to the top environment of this interpreter.
    SymenvPtr getenv() const { return topenv; }

    //! Insert a new symbol and value or reassign an already bound value of an existing symbol
    //! at the top environment of this scheme interpreter.
    void addenv(const Symbol& sym, const Cell& val) { topenv->add(sym, val); }

    //! Insert or reassign zero or more symbol, value pairs into the
    //! top environment of this interpreter.
    void addenv(std::initializer_list<std::pair<Symbol, Cell>> args) { topenv->add(args); }

    //! Create a new empty child environment, connected to the argument parent environment
    //! or if null-pointer, connected to the top environment of this interpreter.
    SymenvPtr newenv(const SymenvPtr& env = nullptr) { return Symenv::create(env ? env : topenv); }

    /**
     * Return a pointer to a new cons-cell from the internal cons-cell store.
     * The new cons-cell is initialized by argument car and cdr values. The pointer
     * lifetime is managed by the internal garbage collector of this interpreter.
     *
     * @param  car Variant value to assign to cons-cell car slot.
     * @param  cdr Variant value to assign to cons-cell cdr slot.
     * @return Pointer to a new initialized Cons-cell.
     */
    template <typename CAR, typename CDR>
    Cons* cons(CAR&& car, CDR&& cdr)
    {
        if (store_size + dflt_gccycle_count < store.size()) {
            gc.collect(*this, topenv);
            store_size = store.size();
        }
        return pscm::cons(store, std::forward<CAR>(car), std::forward<CDR>(cdr));
    }

    //! Build a cons list of all arguments.
    template <typename T, typename... Args>
    Cons* list(T&& t, Args&&... args)
    {
        return pscm::list(store, std::forward<T>(t), std::forward<Args>(args)...);
    }

    //! Create a new symbol or return an existing symbol, build from
    //! the argument string.
    template <typename StringT>
    Symbol symbol(const StringT& str)
    {
        return symtab[string_convert<Char>(str)];
    }

    //! Create a new symbol, guarenteed not to exist before.
    Symbol symbol()
    {
        return symbol(std::string{ "symbol " }.append(std::to_string(symtab.size())));
    }

    /**
     * Create a new ::Function object and install it into the argument
     * environment and bound to a symbol build from the argument name string.
     *
     * External function signature:
     *   fun(Scheme& scm, const SymenvPtr& env, const std::vector<Cell>& argv) -> Cell
     *
     * @param env  Environment pointer, where to add this function. If null-pointer,
     *             add to the top-environment of this interpreter.
     * @param name Symbol name of this function.
     * @param fun  External function or functor.
     *
     * @returns A shared pointer to the created ::Function object.
     */
    template <typename StringT, typename FunctionT>
    FunctionPtr function(const SymenvPtr& env, const StringT& name, FunctionT&& fun)
    {
        auto sym = symbol(name);
        auto funptr = Function::create(sym, std::forward<FunctionT>(fun));

        if (env)
            env->add(sym, funptr);
        else
            topenv->add(sym, funptr);

        return funptr;
    }

    //! Create a new function and install it into the top-environment of this interpreter.
    template <typename StringT, typename FunctionT>
    FunctionPtr function(const StringT& name, FunctionT&& fun)
    {
        return function(nullptr, name, std::forward<FunctionT>(fun));
    }

    //! Create a new unnamed function object and install it into the argument environment
    //! of if null-pointer, into the top environment of this interpreter.
    template <typename FunctionT>
    FunctionPtr function(const SymenvPtr& env, FunctionT&& fun)
    {
        return function(env, String{ L"λ" }, std::forward<FunctionT>(fun));
    }

    PortPtr outPort() { return m_stdout; } //!< return a shared-pointer to the default input port
    PortPtr inPort() { return m_stdin; } //!< return a shared-pointer to the default output port

    //! Start a new read-eval-print loop and use argument environment or if null-pointer
    //! use the top-environment of this interpreter as interaction environment.
    void repl(const SymenvPtr& env = nullptr);

    //! Read scheme expressions from file and evaluate them at the argument
    //! environment or if null-pointer at the top-environment of this interpreter.
    void load(const String& filename, const SymenvPtr& env = nullptr);

    template <typename StringT>
    void load(const StringT& filename, const SymenvPtr& env = nullptr)
    {
        load(string_convert<Char>(filename), env);
    }

    /**
     * Evaluate a scheme expression at the argument symbol environment.
     *
     * @param env Shared pointer to the symbol environment, where to
     *            to evaluate expr.
     * @param expr Scheme expression to evaluate.
     * @return Evaluation result or special symbol @em none for no result.
     */
    Cell eval(SymenvPtr env, Cell expr);

    /**
     * Return a new list of evaluated expressions in argument list.
     *
     * @param env Symbol environment, where to evaluate the argument list.
     * @param args Argument list to evaluate.
     * @param is_list true:   procedure call argument list.
     *                false:  apply expression argument list, where the last list item
     *                        must be nil or an argument list itself.
     * @return List of evaluated argument expressions.
     */
    Cell eval_list(const SymenvPtr& env, Cell list, bool is_list = true);

    /**
     * Evaluate argument list into an argument vector.
     *
     * @param senv Symbol environment, where to evaluate the argument list.
     * @param args Argument list to evaluate.
     * @param is_list true:   procedure call argument list.
     *                false:  apply expression argument list, where the last list item
     *                        must be nil or an argument list itself.
     * @return Vector of evaluated arguments.
     */
    std::vector<Cell> eval_args(const SymenvPtr& env, Cell args, bool is_list = true);

    /**
     * Call an external function or procedure opcode.
     *
     * @param senv  The current symbol environment.
     * @param proc  Scheme function opcode as defined by enum class @ref pscm::Intern.
     * @param args  Function argument vector.
     * @return Function result or special symbol @ref pscm::none for a void function.
     */
    Cell apply(const SymenvPtr& env, Intern opcode, const std::vector<Cell>& args);
    Cell apply(const SymenvPtr& env, const FunctionPtr& proc, const std::vector<Cell>& args);
    Cell apply(const SymenvPtr& env, const Cell& cell, const std::vector<Cell>& args);
    std::pair<SymenvPtr, Cell> apply(const SymenvPtr& senv, const Cell& proc, const Cell& args, bool is_list = true);

    Cell expand(const Cell& macro, Cell& args);

    /**
     * Evaluate each expression in argument list up the last, which
     * is returned unevaluated. This last expression is evaluated at
     * the call site to support unbound tail-recursion.
     */
    Cell syntax_begin(const SymenvPtr& env, Cell args);

protected:
    Cell syntax_if(const SymenvPtr& env, const Cell& args);

    /**
     * Scheme syntax cond.
     *
     * @verbatim
     * (cond <clause>_1 <clause>_2 ...)
     *
     * <clause> := (<test> <expression> ...)
     *          |  (<test> => <expression> ...)
     *          |  (else  <expression> ...)
     * @endverbatim
     */
    Cell syntax_cond(const SymenvPtr& env, Cell args);

    Cell syntax_when(const SymenvPtr& env, Cell args);

    Cell syntax_unless(const SymenvPtr& env, Cell args);

    Cell syntax_and(const SymenvPtr& env, Cell args);

    Cell syntax_or(const SymenvPtr& env, Cell args);

private:
    friend class GCollector;
    static constexpr size_t dflt_bucket_count = 1024; //<! Initial default hash table bucket count.
    static constexpr size_t dflt_gccycle_count = 10000; //<! GC cycle after dflt_gccycle_count cons-cell allocations.

    using standard_port = StandardPort<Char>;
    PortPtr m_stdin = std::make_shared<standard_port>(standard_port::in);
    PortPtr m_stdout = std::make_shared<standard_port>(standard_port::out);

    GCollector gc;
    std::list<Cons> store;
    size_t store_size = 0;

    Symtab symtab{ dflt_bucket_count };
    SymenvPtr topenv = nullptr;
};

} // namespace pscm

#endif // SCHEME_HPP
