#ifndef TYPES_HPP
#define TYPES_HPP

#include <vector>

#include "number.hpp"
#include "proc.hpp"
#include "stream.hpp"
#include "svector.hpp"

namespace pscm {

struct Cell;
enum class Intern;

using None = std::monostate;
using Nil = nullptr_t;
using Bool = bool;
using Char = char;
using Cons = std::pair<Cell, Cell>;
using String = std::shared_ptr<std::basic_string<Char>>;
using Vector = SharedVector<Cell, std::vector<Cell>>;

using Variant = std::variant<None, Nil, Bool, Char, Number, Intern, Cons*, String,
    Vector, Port, Symbol, Symenv, Proc>;

enum class Intern {
    /* Scheme syntax opcodes: */
    _or,
    _and,
    _if,
    _cond,
    _else,
    _arrow,
    _when,
    _unless,
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
    op_symeql,

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

    /* Section 6.11: Exceptions */
    /* Section 6.12: Environments and evaluation */
    /* Section 6.13: Input and output */
    op_isport,
    op_isinport,
    op_isoutport,
    op_istxtport,
    op_isbinport,
    op_isinport_open,
    op_isoutport_open,
    op_inport,
    op_outport,
    op_errport,
    op_callw_port,
    op_callw_infile,
    op_callw_outfile,
    op_with_infile,
    op_with_outfile,
    op_open_infile,
    op_open_inbinfile,
    op_open_outfile,
    op_open_outbinfile,
    op_close_port,
    op_close_inport,
    op_close_outport,
    op_open_instr,
    op_open_outstr,
    op_open_inbytevec,
    op_open_outbytevec,
    op_get_outbytevec,
    op_read,
    op_readchar,
    op_peekchar,
    op_readline,
    op_eof,
    op_iseof,
    op_charready,
    op_readstr,
    op_readu8,
    op_peeku8,
    op_u8ready,
    op_readbytevec,
    op_readbytevecb,
    op_write,
    op_write_shared,
    op_write_simple,
    op_display,
    op_newline,
    op_write_char,
    op_write_str,
    op_write_u8,
    op_write_bytevec,
    op_flush_outport,

    /* Section 6.14: System Interface */
    op_load,
    op_fileok,
    op_delfile,
    op_cmdline,
    op_exit,
    op_exitb,
    op_getenv,
    op_currsec,
    op_currjiffy,
    op_jiffspsec,
    op_features,
};

Symbol sym(const char* name);
Symenv senv(const Symenv& env = nullptr);
Vector vec(Number size, const Cell& val);
String str(const Char* s);

}; // namespace pscm

#endif // TYPES_HPP
