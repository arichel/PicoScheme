#ifndef PROC_HPP
#define PROC_HPP

#include <memory>

#include "cell.hpp"

namespace pscm {

class Procedure {
public:
    Procedure(const Symenv& senv, const Cell& args, const Cell& code);

    std::pair<Symenv, Cell> apply(const Symenv& senv, Cell args, bool is_list);

    Cell code() const;

private:
    bool is_unique_symbol_list(Cell args);

    Symenv _senv;
    Cell _args;
    Cons _code;
};

inline std::pair<Symenv, Cell> apply(const Symenv& senv, const Proc& proc, const Cell& args, bool is_list)
{
    return proc->apply(senv, args, is_list);
}

static Proc lambda(const Symenv& senv, const Cell& args, const Cell& code)
{
    return std::make_shared<Procedure>(senv, args, code);
}

}; // namespace pscm

#endif // PROC_HPP
