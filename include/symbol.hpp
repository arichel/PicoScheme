/*********************************************************************************/ /**
                                                                                     * @file symbol.hpp
                                                                                     *
                                                                                     * @version   0.1
                                                                                     * @date      2018-
                                                                                     * @author    Paul Pudewills
                                                                                     * @copyright MIT License
                                                                                     *************************************************************************************/
#ifndef SYMBOL_HPP
#define SYMBOL_HPP

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

namespace pscm {

/**
 * @brief The Symbol struct
 */
template <typename T,
    typename Hash = std::hash<T>,
    typename Equal = std::equal_to<T>>
struct Symtab {

    struct Symbol {
        using value_type = T;
        using key_type = size_t;

        const value_type& value() const noexcept
        {
            return val;
        }
        operator key_type() const noexcept
        {
            return reinterpret_cast<key_type>(&val);
        }
        bool operator==(const Symbol& sym) const noexcept
        {
            return &val == &sym.val;
        }
        bool operator!=(const Symbol& sym) const noexcept
        {
            return &val != &sym.val;
        }
        //Symbol() = delete;
        Symbol(const Symbol&) = default;
        Symbol(Symbol&&) = default;
        Symbol& operator=(const Symbol&) = default;
        Symbol& operator=(Symbol&&) = default;

    private:
        Symbol(const value_type& val)
            : val{ val }
        {
        }
        friend Symtab<T>;
        const value_type& val;
    };
    Symtab(size_t size = 0)
        : table(size)
    {
    }

    template <typename Val>
    Symbol operator[](Val&& val)
    {
        return *table.emplace(std::forward<Val>(val)).first;
    }

private:
    std::unordered_set<T, Hash, Equal> table;
};

/**
 *
 */
template <typename Sym, typename T>
class SymbolEnv {

public:
    using table_type = std::unordered_map<typename Sym::key_type, T>;
    using shared_type = std::shared_ptr<SymbolEnv<Sym, T>>;

    SymbolEnv(const shared_type& next = nullptr)
        : next(next)
    {
    }

    SymbolEnv(std::initializer_list<std::pair<Sym, T>> args)
    {
        for (auto& [sym, val] : args)
            add(sym, val);
    }

    void add(const Sym& sym, const T& arg)
    {
        table.insert_or_assign(sym, arg);
    }

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

        throw std::invalid_argument("unknown symbol");
    }

    const T& get(const Sym& sym) const
    {
        const SymbolEnv* senv = this;

        do {
            auto iter = senv->table.find(sym);

            if (iter != senv->table.end())
                return iter->second;

        } while ((senv = senv->next.get()));

        throw std::invalid_argument("unknown symbol");
    }

private:
    table_type table;
    const shared_type next;
};

struct Cell;

using Symbol = Symtab<std::string>::Symbol;

std::shared_ptr<SymbolEnv<Symbol, Cell>> senv(const std::shared_ptr<SymbolEnv<Symbol, Cell>>& env = nullptr);

Symbol sym(const char* name);

} // namespace pscm
#endif // SYMBOL_HPP
