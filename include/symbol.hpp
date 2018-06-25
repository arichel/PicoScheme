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
#include <string>
#include <unordered_set>

namespace pscm {

/**
 * @brief The Symbol struct
 */
struct Symbol {
    Symbol(const Symbol&) = default;
    Symbol(Symbol&&) = default;

    Symbol& operator=(const Symbol&) = default;
    Symbol& operator=(Symbol&& s) = default;

    const std::string& name() const noexcept { return *pname; }

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
    const std::string* pname;
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
} // namespace pscm
#endif // SYMBOL_HPP
