#ifndef TYPES_HPP
#define TYPES_HPP

#include <cstddef>
#include <memory>
#include <string>
#include <variant>

#include "number.hpp"
#include "proc.hpp"
#include "symbol.hpp"

namespace pscm {

struct Cell;

using None = std::monostate;
using Nil = nullptr_t;
using Bool = bool;
using Char = char;
using Port = std::ostream;
using Cons = std::pair<Cell, Cell>;
using String = std::shared_ptr<std::basic_string<Char>>;
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
    op_eq,
    op_eqv,
    op_equal,

    op_numeq,
    op_numlt,
    op_numgt,
    op_numle,
    op_numge,
    op_zero,
    op_add,
    op_sub,
    op_mul,
    op_div,
    op_sin,
    op_cos,
    op_tan,
    op_asin,
    op_acos,
    op_atan,
    op_sinh,
    op_cosh,
    op_tanh,
    op_asinh,
    op_acosh,
    op_atanh,
    op_sqrt,
    op_exp,
    op_pow,
    op_square,
    op_log,
    op_log10,
    op_abs,
    op_real,
    op_imag,
    op_arg,
    op_conj,
    op_rect,
    op_polar
};

using Variant = std::variant<None, Nil, Bool, Number, Intern, Cons*, String, Symbol, Symenv, Proc, Port*>;

}; // namespace pscm

#endif // TYPES_HPP
