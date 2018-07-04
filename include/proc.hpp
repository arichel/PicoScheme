#ifndef PROC_HPP
#define PROC_HPP

#include <memory>

#include "symbol.hpp"

namespace pscm {

struct Cell;
using Symenv = std::shared_ptr<SymbolEnv<Cell>>;

class Proc {
public:
    Proc(const Symenv& senv, const Cell& args, const Cell& code);
    Proc(const Proc& proc);
    Proc(Proc&& proc) noexcept;
    ~Proc();

    Proc& operator=(const Proc&);
    Proc& operator=(Proc&&) noexcept;

    bool operator!=(const Proc& proc) const noexcept;
    bool operator==(const Proc& proc) const noexcept;

    std::pair<Symenv, Cell> apply(const Symenv& senv, Cell args, bool is_list) const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

std::pair<Symenv, Cell> apply(const Symenv& senv, const Cell& proc, const Cell& args, bool is_list);

}; // namespace pscm

#endif // PROC_HPP
