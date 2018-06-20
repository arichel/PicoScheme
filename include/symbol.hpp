#ifndef SYMBOL_HPP
#define SYMBOL_HPP

#include <string>
#include <iostream>
#include <unordered_map>

namespace pscm {

struct Symbol
{
    Symbol(const Symbol&) = default;
    Symbol(Symbol&&) = default;

    Symbol& operator= (const Symbol&) = default;
    Symbol& operator= (Symbol&& s) = default;

    const std::string& name() const noexcept {return *pname;}
    bool is_intern() const noexcept {return intern;}

    bool operator == (const Symbol& rhs) const noexcept
    {
        return pname == rhs.pname;
    }
    bool operator != (const Symbol& rhs) const noexcept
    {
        return pname != rhs.pname;
    }
protected:
    Symbol(const std::string &str, bool intern)
        :pname{&str}, intern{intern}
    {
    }
    friend struct Symtab;
    const std::string *pname;
    bool intern;
};

struct Symtab
{
    Symtab(size_t size = 10000)
        : table{size}
    {
    }
    Symbol operator [](const char *str)
    {
        return atom(str, false);
    }
    Symbol atom(const char *str, bool intern = true)
    {
        auto iter = table.insert(std::make_pair(std::string{str}, intern)).first;

        std::cout << str << " intern? " << intern << " --> {" << iter->first << " : " << iter->second << "} new: " << std::endl;

        intern == iter->second || (throw std::invalid_argument("invalid symbol name"), 0);

        return Symbol{iter->first, intern};
    }
    void print()
    {
        for(auto& [key, flag] : table)
            std::cout << "key: " << key << " flag: " << flag << std::endl;
    }
private:
    std::unordered_map<std::string, bool> table;

public:
    const Symbol s_nil {atom("()")}, s_true {atom("#t")}, s_false {atom("#f")}, s_none {atom("#none")};
};

static Symtab stab;
inline Symbol atom (const char *s) {return stab.atom(s, true);}
inline Symbol sym  (const char *s) {return stab.atom(s, false);}

} // namespace pscm
#endif // SYMBOL_HPP
