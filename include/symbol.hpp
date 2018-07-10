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
struct Symbol {
    using name_type = std::string;
    using key_type = size_t;

    Symbol(const Symbol&) = default;
    Symbol(Symbol&&) = default;

    Symbol& operator=(const Symbol&) = default;
    Symbol& operator=(Symbol&& s) = default;

    const name_type& name() const noexcept
    {
        return *pname;
    }

    operator key_type() const noexcept
    {
        return reinterpret_cast<key_type>(pname);
    }

    bool operator==(const Symbol& rhs) const noexcept
    {
        return pname == rhs.pname;
    }
    bool operator!=(const Symbol& rhs) const noexcept
    {
        return pname != rhs.pname;
    }

protected:
    Symbol(const std::string& str)
        : pname{ &str }
    {
    }
    Symbol(const std::string* str)
        : pname{ str }
    {
    }

    friend struct Symtab;
    friend struct Symtab2;
    const name_type* pname;
};

struct Symtab {
    Symtab(size_t size = 10000)
        : table{ size }
    {
    }
    Symbol operator[](const char* str)
    {
        return Symbol{ *table.emplace(str).first };
    }

private:
    std::unordered_set<std::string> table;
};

template <typename T,
    typename Hash = std::hash<T>,
    typename Equal = std::equal_to<T>>
struct Symtab3 {
    using value_type = T;

    struct Symbol {
        using value_type = typename Symtab3<T>::value_type;
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
        Symbol() = delete;

    private:
        Symbol(const value_type& val)
            : val{ val }
        {
        }
        friend Symtab3<T>;
        const value_type& val;
    };
    Symtab3(size_t size = 0)
        : table(size)
    {
    }

    template <typename Val>
    Symbol operator[](Val&& val)
    {
        return *table.emplace(std::forward<Val>(val)).first;
    }
    //private:
    std::unordered_set<T, Hash, Equal> table;
};

/**
 *
 */
template <typename T>
class SymbolEnv {

public:
    using table_type = std::unordered_map<Symbol::key_type, T>;
    using shared_type = std::shared_ptr<SymbolEnv<T>>;

    SymbolEnv(const shared_type& next = nullptr)
        : next(next)
    {
    }

    SymbolEnv(std::initializer_list<std::pair<Symbol, T>> args)
    {
        for (auto& [sym, val] : args)
            add(sym, val);
    }

    void add(const Symbol& sym, const T& arg)
    {
        table.insert_or_assign(sym, arg);
    }

    void set(const Symbol& sym, const T& arg)
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

    const T& get(const Symbol& sym) const
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

std::shared_ptr<SymbolEnv<Cell>> senv(const std::shared_ptr<SymbolEnv<Cell>>& env = nullptr);

Symbol sym(const char* name);

} // namespace pscm
#endif // SYMBOL_HPP
