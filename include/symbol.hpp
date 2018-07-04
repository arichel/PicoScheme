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

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

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
    friend struct Symtab;
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
