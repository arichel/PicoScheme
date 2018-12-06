/********************************************************************************/ /**
 * @file symbol.hpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#ifndef SYMBOL_HPP
#define SYMBOL_HPP

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "utils.hpp"

namespace pscm {

struct symenv_exception;

/**
 * Symbol table to provide unique symbols.
 *
 * A symbol table is a factory class to provide unique Symbols as a
 * surjective mapping between values of type T to symbols of
 * type Symtab<T>::Symbol.
 *
 * @tparam T     Value type of symbol, like std::string, char, int,...
 * @tparam Hash  Hash function object that implements a has function for values of type T.
 * @tparam Equal Function object for performing comparison on values of type T.
 */
template <typename T, typename Hash = std::hash<T>, typename Equal = std::equal_to<T>>
struct SymbolTable {
    //! A Symbol as handle to a pointer of type T into the symbol table
    struct Symbol {
        using value_type = T;

        Symbol() = delete;
        Symbol(const Symbol&) = default;
        Symbol(Symbol&&) = default;

        Symbol& operator=(const Symbol&) = default;
        Symbol& operator=(Symbol&& s) = default;

        //! Return a constant refererence to the symbol value T.
        const T& value() const noexcept { return *ptr; }

        //! Equality predicates.
        bool operator==(const Symbol& sym) const noexcept { return ptr == sym.ptr; }
        bool operator!=(const Symbol& sym) const noexcept { return ptr != sym.ptr; }
        bool operator<(const Symbol& sym) const noexcept { return ptr < sym.ptr; }

        struct hash : private std::hash<const T*> {
            using argument_type = Symbol;
            using result_type = std::size_t;

            result_type operator()(const Symbol& sym) const noexcept
            {
                return std::hash<const T*>::operator()(sym.ptr);
            }
        };

    private:
        Symbol(const T& val)
            : ptr{ &val }
        {
        }
        friend struct SymbolTable; //! needs access to private constructor
        const T* ptr;
    };
    /**
     * Construct a symbol table
     * @param bucket_count Initial hash table bucket count hint.
     */
    SymbolTable(size_t bucket_count = 0)
        : table(bucket_count)
    {
    }

    /**
     * Return a new or previously constructed symbol.
     * @tparam Val Value to in-place construct a new symbol.
     * @return Symbol of type Symtab<T>::Symbol.
     */
    template <typename Val>
    Symbol operator[](Val&& val) { return *table.emplace(std::forward<Val>(val)).first; }

    size_t size() { return table.size(); }

private:
    std::unordered_set<T, Hash, Equal> table;
};

//! Exception to be thrown by template class SymbolEnv for unknown symbols.
struct symenv_exception : public std::exception {

    template <typename Sym>
    symenv_exception(const Sym& sym)
    {
        reason.append(string_convert<char>(sym.value()));
    }
    const char* what() const noexcept override { return reason.c_str(); }

private:
    std::string reason{ "unknown symbol " };
};

/**
 * Symbol-value environment
 *
 * A symbol environment associates symbols to values. Symbol value bindings
 * are unique per environment. Severel environments form a child-parent tree.
 *
 * @tparam Sym Symbol type
 * @tparam T   Value type
 */
template <typename Sym, typename T, typename Hash = std::hash<Sym>>
class SymbolEnv : public std::enable_shared_from_this<SymbolEnv<Sym, T, Hash>> {

public:
    using symbol_type = Sym;
    using value_type = T;
    using shared_type = std::shared_ptr<SymbolEnv>;

    using std::enable_shared_from_this<SymbolEnv>::shared_from_this;
    using std::enable_shared_from_this<SymbolEnv>::weak_from_this;

    //! Create a new empty symbol environment, optionally as a child
    //! of the argument parent environment.
    static shared_type create(const shared_type& parent = nullptr)
    {
        return shared_type{ new SymbolEnv{ parent } };
    }

    //! Create a new symbol environment and initialize it with (symbol,value)-pairs
    static shared_type create(std::initializer_list<std::pair<Sym, T>> args,
        const shared_type& parent = nullptr)
    {
        return shared_type{ new SymbolEnv{ args, parent } };
    }

    //! Insert a new symbol and value or reassigns a bound value of an existing symbol
    //! in this environment only.
    void add(const Sym& sym, const T& val)
    {
        table.insert_or_assign(sym, val);
    }

    //! Insert or reassign zero or more (symbol,value)-pairs into this environment.
    void add(std::initializer_list<std::pair<Sym, T>> args)
    {
        for (auto& [sym, val] : args)
            add(sym, val);
    }
    /**
     * Reassign a bound value of the first found symbol in this or any
     * reachable parent environment.
     *
     * @throws a symenv_exception exception for unknown or unreachable symbols.
     */
    void set(const Sym& sym, const T& arg)
    {
        SymbolEnv* senv = this;

        do {
            auto iter = senv->table.find(sym);

            if (iter != senv->table.end()) {
                iter->second = arg;
                return;
            }

        } while ((senv = senv->next.get()));

        throw symenv_exception{ sym };
    }
    /**
     * Lookup a symbol in this or any reachable parent environment
     * and return its bound value.
     *
     * @throws a symenv_exception exception for unknown or unreachable symbols.
     */
    const T& get(const Sym& sym) const
    {
        const SymbolEnv* senv = this;

        do {
            auto iter = senv->table.find(sym);

            if (iter != senv->table.end())
                return iter->second;

        } while ((senv = senv->next.get()));

        throw symenv_exception{ sym };
    }
    /**
     * Cursor as (begin,end)-iterator range to iterate over all (symbol,value)-pairs
     * of this environment and to move to the next parent environment.
     */
    struct Cursor {
        auto begin() const { return env.lock()->table.begin(); }
        auto end() const { return env.lock()->table.end(); }
        auto symenv() const { return shared_type{ env }; }

        //! Move cursor to next parent environment or return std::nullopt
        //! for a top-environment.
        std::optional<Cursor> next() const
        {
            shared_type e{ env };
            return e->next ? std::optional<Cursor>{ Cursor{ e->next } }
                           : std::nullopt;
        }

    private:
        friend class SymbolEnv;
        Cursor(std::weak_ptr<SymbolEnv> env)
            : env{ std::move(env) }
        {
        }
        std::weak_ptr<SymbolEnv> env;
    };

    //! Return a cursor
    Cursor cursor() { return Cursor{ weak_from_this() }; }
    Cursor cursor() const { return Cursor{ weak_from_this() }; }

private:
    /**
     * Construct a symbol environment as top- or sub-environment.
     * @param parent Optional, unless null-pointer. construct a sub-environment connected
     *               to the parent environment or a top-environment otherwise.
     */
    SymbolEnv(const shared_type& parent = nullptr)
        : next{ parent }
    {
    }

    //! Construct a new top or child environment and initialize it with {symbol,value} pairs
    //! from initializer list.
    SymbolEnv(std::initializer_list<std::pair<Sym, T>> args, const shared_type& parent = nullptr)
        : next{ parent }
        , table{ args.size() }
    {
        for (auto& [sym, val] : args)
            add(sym, val);
    }

private:
    const std::shared_ptr<SymbolEnv> next = nullptr;
    std::unordered_map<Sym, T, Hash> table;
};

} // namespace pscm

#endif // SYMBOL_HPP
