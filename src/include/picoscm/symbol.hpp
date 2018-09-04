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
#include <unordered_map>
#include <unordered_set>

namespace pscm {

using namespace std::string_literals;

/**
 * @brief Symbol table of to provide unique symbols.
 *
 * A symbol table is bijective mapping between values of type T
 * to unique symbols of type Symtab<T>::Symbol.
 *
 * @tparam T     Value type of symbol, like std::string, char, int,...
 * @tparam Hash  Hash function object that implements a has function for values of type T.
 * @tparam Equal Function object for performing comparison on values of type T.
 */
template <typename T, typename Hash = std::hash<T>, typename Equal = std::equal_to<T>>
struct SymbolTable {

    /**
     * @brief Symbol structure as pointer handle.
     */
    struct Symbol {
        using value_type = T;
        using key_type = size_t;

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
         * @brief Conversion operator to map a symbol to a unique key value.
         */
        operator key_type() const noexcept
        {
            return reinterpret_cast<key_type>(ptr);
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

    private:
        Symbol(const T& val)
            : ptr{ &val }
        {
        }
        friend SymbolTable;

        std::add_pointer_t<std::add_const_t<T>> ptr; //! or const T*
    };

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
 * @brief Symbol environment as tree of unordered associative containers of
 *        symbol keys and values of type T.
 *
 * @tparam Sym Symbol type
 * @tparam T   Value type
 */
template <typename Sym, typename T>
class SymbolEnv {

public:
    using symbol_type = Sym;
    using value_type = T;

    using key_type = typename Sym::key_type;

    /**
     * @brief Construct a symbol environment as top- or sub-environment.
     * @param parent Optional, unless null-pointer. construct a sub-environment connected
     *               to the parent environment or a top-environment otherwise.
     */
    SymbolEnv(const std::shared_ptr<SymbolEnv>& parent = nullptr)
        : next{ parent }
    {
    }

    /**
     * @brief Construct to top environment and initialize it with {symbol,value} pairs.
     */
    SymbolEnv(std::initializer_list<std::pair<Sym, T>> args)
        : table{ args.size() }
    {
        for (auto& [sym, val] : args)
            add(sym, val);
    }

    /**
     * @brief Insert a new symbol and value or reassign value of an existing symbol
     *        in this environment only.
     */
    void add(const Sym& sym, const T& val)
    {
        table.insert_or_assign(sym, val);
    }

    /**
     * @brief Reassign value of first found symbol in this or any
     *        reachable parent environment.
     * @throw std::invalid_argument exception for unknown or unreachable symbols.
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

        throw std::invalid_argument("unknown symbol "s
            + static_cast<std::string>(sym.value()));
    }

    /**
     * @brief Lookup a symbol in this or any reachable parent environment
     *        and return its value.
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

        throw std::invalid_argument("unknown symbol "s
            + static_cast<std::string>(sym.value()));
    }

private:
    const std::shared_ptr<SymbolEnv> next = nullptr;
    std::unordered_map<key_type, T> table;
};

} // namespace pscm
#endif // SYMBOL_HPP
