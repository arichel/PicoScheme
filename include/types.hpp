#ifndef TYPES_HPP
#define TYPES_HPP

#include "number.hpp"
#include "proc.hpp"
#include "svector.hpp"

namespace pscm {

struct Cell;
enum class Intern;

using None = std::monostate;
using Nil = nullptr_t;
using Bool = bool;
using Char = char;
using Port = std::ostream;
using Cons = std::pair<Cell, Cell>;
using String = std::shared_ptr<std::basic_string<Char>>;
using Vector = SharedVector<Cell>;

using Variant = std::variant<
    None, Nil, Bool, Char, Number, Intern, Cons*, String,
    Vector, Symbol, Symenv, Proc, Port*>;

enum class Intern {
    _or,
    _and,
    _if,
    _when,
    _unless,
    _cond,
    _quote,
    _define,
    _setb,
    _begin,
    _lambda,
    _apply,

    /* Section 6.1: Equivalence predicates */
    op_eq,
    op_eqv,
    op_equal,

    /* Section 6.2: Numbers */
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
    op_polar,

    /* Section 6.3: Booleans */
    op_not,
    op_isbool,
    op_isbooleq,

    /* Section 6.4: Pair and lists */
    op_cons,
    op_car,
    op_cdr,
    op_caar,
    op_cddr,
    op_cadr,
    op_cdar,
    op_setcdr,
    op_setcar,
    op_list,
    op_ispair,
    op_islist,
    op_mklist,
    op_length,
    op_append,
    op_tail,
    op_ref,
    op_set,
    op_memq,
    op_memv,
    op_member,
    op_assq,
    op_assv,
    op_assoc,

    /* Section 6.5: Symbols */
    op_issym,
    op_symstr,
    op_strsym,

    /* Section 6.6: Characters */
    op_ischar,
    op_ischareq,
    op_ischarlt,
    op_ischargt,
    op_ischarle,
    op_ischarge,
    op_ischcieq,
    op_ischcilt,
    op_ischcigt,
    op_ischcile,
    op_ischcige,
    op_isalpha,
    op_isdigit,
    op_iswspace,
    op_isupper,
    op_islower,
    op_charint,
    op_digitval,
    op_upcase,
    op_downcase,
    op_foldcase,

    /* Section 6.7: Strings */
    op_mkstr,
    op_str,
    op_strlen,
    op_strref,
    op_strsetb,
    op_isstr,
    op_isstreq,
    op_isstrcieq,
    op_isstrgr,
    op_isstrcigr,
    op_isstrlt,
    op_isstrcilt,
    op_isstrge,
    op_isstrcige,
    op_isstrle,
    op_isstrcile,
    op_strupcase,
    op_strdowncase,
    op_strfoldcase,
    op_substr,
    op_strappend,
    op_strlist,
    op_liststr,
    op_strcopy,
    op_strcopyb,
    op_strfillb,

    /* Section 6.8: Vectors */
    op_isvec,
    op_mkvec,
    op_vec,
    op_veclen,
    op_vecref,
    op_vecsetb,
    op_veclist,
    op_listvec,
    op_vecstr,
    op_strvec,
    op_veccopy,
    op_veccopyb,
    op_vecappend,
    op_vecfillb,

    /* Section 6.9: Bytevectors */

    /* Section 6.10: Control features */
    op_isproc,
    op_map,
    op_strmap,
    op_vecmap,
    op_foreach,
    op_strforeach,
    op_vecforeach,
    op_callcc,
    op_values,
    op_callwithval,
    op_dynwind,

    /* */
};

Symbol sym(const char* name);
Symenv senv(const Symenv& env = nullptr);
Vector vec(Number size, const Cell& val);
String str(const Char* s);

}; // namespace pscm

#endif // TYPES_HPP
