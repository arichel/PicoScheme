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
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace pscm {

/**
 * Symbol table to provide unique symbols.
 *
 * A symbol table is a factory class to provide unique Symbols as a surjective mapping between
 * values of type T to symbols of type Symtab<T>::Symbol.
 *
 * @tparam T     Value type of symbol, like std::string, char, int,...
 * @tparam Hash  Hash function object that implements a has function for values of type T.
 * @tparam Equal Function object for performing comparison on values of type T.
 */
template <typename T, typename Hash = std::hash<T>, typename Equal = std::equal_to<T>>
struct SymbolTable {

    /**
     * A Symbol as handle to a pointer of type T into the symbol table
     */
    struct Symbol {
        using value_type = T;

        Symbol() = delete;
        Symbol(const Symbol&) = default;
        Symbol(Symbol&&) = default;

        Symbol& operator=(const Symbol&) = default;
        Symbol& operator=(Symbol&& s) = default;

        /**
         * @brief Return a constant refererence to the symbol value T.
         */
        const T& value() const noexcept
        {
            return *ptr;
        }

        /**
         * @brief Equality predicate.
         */
        bool operator==(const Symbol& sym) const noexcept
        {
            return ptr == sym.ptr;
        }

        bool operator!=(const Symbol& sym) const noexcept
        {
            return ptr != sym.ptr;
        }

        bool operator<(const Symbol& sym) const noexcept
        {
            return ptr < sym.ptr;
        }

        struct hash {
            size_t operator()(const Symbol& sym) const noexcept
            {
                return reinterpret_cast<size_t>(sym.ptr);
            }
        };

    private:
        Symbol(const T& val)
            : ptr{ &val }
        {
        }
        friend SymbolTable; //! needs access to private constructor

        std::add_pointer_t<std::add_const_t<T>> ptr; //! or const T*
    };

    using symbol_type = Symbol;

    /**
     * @brief Construct a symbol table
     * @param bucket_count Initial hash table bucket count hint.
     */
    SymbolTable(size_t bucket_count = 0)
        : table(bucket_count)
    {
    }

    /**
     * Return a new or previously constructed symbol.
     *
     * @tparam Val Value to in-place construct a new symbol.
     * @return Symbol of type Symtab<T>::Symbol.
     */
    template <typename Val>
    Symbol operator[](Val&& val)
    {
        return *table.emplace(std::forward<Val>(val)).first;
    }

    size_t size()
    {
        return table.size();
    }

private:
    std::unordered_set<T, Hash, Equal> table;
};

/**
 * Symbol-value environment
 *
 * A symbol environment associates symbols to values. Symbol value bindings
 * are unique per environment. Several environments build child-parent trees
 *
 * @tparam Sym Symbol type
 * @tparam T   Value type
 */
template <typename Sym, typename T, typename Hash = std::hash<Sym>>
class SymbolEnv {

public:
    using symbol_type = Sym;
    using value_type = T;
    using shared_type = std::shared_ptr<SymbolEnv>;

    /**
     * @brief Construct a symbol environment as top- or sub-environment.
     * @param parent Optional, unless null-pointer. construct a sub-environment connected
     *               to the parent environment or a top-environment otherwise.
     */
    SymbolEnv(const shared_type& parent = nullptr)
        : next{ parent }
    {
    }

    /**
     * @brief Construct to top environment and initialize it with {symbol,value} pairs.
     */
    SymbolEnv(std::initializer_list<std::pair<Sym, T>> args, const shared_type& parent = nullptr)
        : next{ parent }
        , table{ args.size() }
    {
        for (auto& [sym, val] : args)
            add(sym, val);
    }

    /**
     * @brief Insert a new symbol and value or reassign a bound value of an existing symbol
     *        in this environment only.
     */
    void add(const Sym& sym, const T& val)
    {
        table.insert_or_assign(sym, val);
    }

    /**
     * Reassign a bound value of first found symbol in this or any
     * reachable parent environment.
     * @throw std::invalid_argument exception for an unknown or unreachable symbols.
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

        throw std::invalid_argument(std::string{ "unknown symbol " }
            + static_cast<std::string>(sym.value()));
    }

    /**
     * @brief Lookup a symbol in this or any reachable parent environment
     *        and return its bound value.
     *
     * @throw std::invalid_argument exception for unknown or unreachable symbols.
     */
    const T& get(const Sym& sym) const
    {
        const SymbolEnv* senv = this;

        do {
            auto iter = senv->table.find(sym);

            if (iter != senv->table.end())
                return iter->second;

        } while ((senv = senv->next.get()));

        throw std::invalid_argument(std::string{ "unknown symbol " }
            + static_cast<std::string>(sym.value()));
    }

private:
    const std::shared_ptr<SymbolEnv> next = nullptr;
    std::unordered_map<Sym, T, Hash> table;
};

} // namespace pscm

#endif // SYMBOL_HPP
