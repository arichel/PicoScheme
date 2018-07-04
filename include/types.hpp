#ifndef TYPES_HPP
#define TYPES_HPP

#include <cstddef>
#include <memory>
#include <string>
#include <variant>

#include "number.hpp"
#include "symbol.hpp"

namespace pscm {

struct Cell;
class Procedure;

using None = std::monostate;
using Nil = nullptr_t;
using Bool = bool;
using Char = char;
using Port = std::ostream;
using Cons = std::pair<Cell, Cell>;
using String = std::shared_ptr<std::basic_string<Char>>;
using Proc = std::shared_ptr<Procedure>;
using Symenv = std::shared_ptr<SymbolEnv<Cell>>;

enum class Intern {
    _or,
    _and,
    _if,
    _cond,
    _quote,
    _define,
    _setb,
    _begin,
    _lambda,
    _apply,

    op_cons,
    op_car,
    op_cdr,
    op_setcdr,
    op_setcar,
    op_list,

    op_add,
    op_sub,
    op_mul,
    op_div,

};

using Variant = std::variant<None, Nil, Bool, Number, Intern, Cons*, String, Symbol, Symenv, Proc, Port*>;

}; // namespace pscm

#endif // TYPES_HPP
