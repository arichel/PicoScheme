#ifndef TYPES_HPP
#define TYPES_HPP

#include <cstddef>
#include <deque>
#include <vector>

#include "number.hpp"
#include "proc.hpp"
#include "stream.hpp"

namespace pscm {

struct Cell;
enum class Intern;

using None = std::monostate;
using Nil = std::nullptr_t;
using Bool = bool;
using Char = char;
using Cons = std::pair<Cell, Cell>;
using StringPtr = std::shared_ptr<std::basic_string<Char>>;
using VectorPtr = std::shared_ptr<std::vector<Cell>>;
using FunctionPtr = std::shared_ptr<Func>;

using Variant = std::variant<None, Nil, Intern, Bool, Char, Number, Cons*,
    StringPtr, VectorPtr, Port, Symbol, SymenvPtr, FunctionPtr, Proc>;

/**
 * A scheme cell is a variant type of all supported scheme types.
 */
struct Cell : Variant {
    using base_type = Variant;
    using Variant::Variant;
};

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
    _define,
    _setb,
    _begin,
    _lambda,
    _macro,
    _apply,
    _quote,
    _quasiquote,
    _unquote,
    _unquotesplice,

    /* Section 6.1: Equivalence predicates */
    op_eq,
    op_eqv,
    op_equal,

    /* Section 6.2: Numbers */
    op_isnum,
    op_iscpx,
    op_isreal,
    op_israt,
    op_isint,
    op_isexact,
    op_isinexact,
    op_isexactint,
    op_ex2inex,
    op_inex2ex,
    op_isodd,
    op_iseven,
    op_numeq,
    op_numlt,
    op_numgt,
    op_numle,
    op_numge,
    op_min,
    op_max,
    op_ispos,
    op_isneg,
    op_zero,
    op_add,
    op_sub,
    op_mul,
    op_div,
    op_mod,
    op_rem,
    op_floor,
    op_ceil,
    op_trunc,
    op_round,
    op_quotient,
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
    op_cbrt,
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
    op_hypot,
    op_strnum,
    op_numstr,

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
    op_caddr,
    op_setcdr,
    op_setcar,
    op_list,
    op_isnil,
    op_ispair,
    op_islist,
    op_mklist,
    op_length,
    op_append,
    op_reverse,
    op_reverseb,
    op_tail,
    op_listref,
    op_listsetb,
    op_listcopy,
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
    op_gensym,

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
    op_intchar,
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
    op_isstrgt,
    op_isstrcigt,
    op_isstrlt,
    op_isstrcilt,
    op_isstrge,
    op_isstrcige,
    op_isstrle,
    op_isstrcile,
    op_strupcase,
    op_strdowncase,
    op_strupcaseb,
    op_strdowncaseb,
    op_strfoldcase,
    op_strappend,
    op_strappendb,
    op_strlist,
    op_liststr,
    op_substr,
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
    op_vecappendb,
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
    op_error,

    /* Section 6.12: Environments and evaluation */
    op_exit,
    op_replenv,
    op_repl,
    op_eval,
    op_macroexp,

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
    op_read_char,
    op_peek_char,
    op_readline,
    op_eof,
    op_flush,
    op_iseof,
    op_char_ready,
    op_read_str,
    op_read_u8,
    op_peek_u8,
    op_ready_u8,
    op_read_bytevec,
    op_read_bytevecb,
    op_write,
    op_display,
    op_write_shared,
    op_write_simple,
    op_newline,
    op_write_char,
    op_write_str,
    op_write_u8,
    op_write_bytevec,

    /* Section 6.14: System Interface */
    op_load,
    op_fileok,
    op_delfile,
    op_cmdline,
    op_exitb,
    op_getenv,
    op_currsec,
    op_currjiffy,
    op_jiffspsec,
    op_features,
};

SymenvPtr senv(const SymenvPtr& env = nullptr);

void addenv(const Symbol& sym, const Cell& cell, const SymenvPtr& env = nullptr);

FunctionPtr fun(const Symbol& sym, std::function<Cell(const SymenvPtr&, const std::vector<Cell>&)>&& fn,
    const SymenvPtr& env = nullptr);

VectorPtr vec(Number size, const Cell& val);

StringPtr str(const Char* s);

Symbol sym(const char* name);

Symbol gensym();

//!
void load(const std::string& filnam, const SymenvPtr& env = nullptr);

/**
 * Call an external function or procedure opcode.
 *
 *
 * @param senv  The current symbol environment.
 * @param proc  Scheme function opcode as defined by enum class @ref pscm::Intern.
 * @param args  Function argument vector.
 * @return Function result or special symbol @ref pscm::none for a void function.
 */
Cell call(const SymenvPtr& senv, const Cell& proc, const std::vector<Cell>& args);

class Scheme {

public:
    using Symbol = Symtab::Symbol;
    using Symenv = SymbolEnv<Symbol, Cell>;
    using SymenvPtr = std::shared_ptr<Symenv>;

    template <typename CAR, typename CDR>
    Cons* cons(CAR&& car, CDR&& cdr)
    {
        return &store.emplace_back(std::forward<CAR>(car), std::forward<CDR>(cdr));
    }
    Symbol mksym(const char* name) { return symtab[name]; }

    Symbol gensym();

    SymenvPtr newenv(const SymenvPtr& env);

    void addenv(const Symbol& sym, const Cell& cell, const SymenvPtr& env);

private:
    static constexpr size_t dflt_bucket_count = 1024; //<! Initial default hash table bucket count.

    std::deque<Cons> store;

    Symtab symtab{ dflt_bucket_count };

    SymenvPtr topenv{
        new Symenv{
            { mksym("#t"), true },
            { mksym("#true"), true },
            { mksym("#f"), false },
            { mksym("#false"), false },
            { mksym("or"), Intern::_or },
            { mksym("and"), Intern::_and },
            { mksym("if"), Intern::_if },
            { mksym("cond"), Intern::_cond },
            { mksym("else"), Intern::_else },
            { mksym("=>"), Intern::_arrow },
            { mksym("when"), Intern::_when },
            { mksym("unless"), Intern::_unless },
            { mksym("begin"), Intern::_begin },
            { mksym("define"), Intern::_define },
            { mksym("set!"), Intern::_setb },
            { mksym("lambda"), Intern::_lambda },
            { mksym("define-macro"), Intern::_macro },
            { mksym("quote"), Intern::_quote },
            { mksym("quasiquote"), Intern::_quasiquote },
            { mksym("unquote"), Intern::_unquote },
            { mksym("unquote-splicing"), Intern::_unquotesplice },
            { mksym("apply"), Intern::_apply },

            /* Section 6.1: Equivalence predicates */
            { mksym("eq?"), Intern::op_eq },
            { mksym("eqv?"), Intern::op_eqv },
            { mksym("equal?"), Intern::op_equal },

            /* Section 6.2: Numbers */
            { mksym("number?"), Intern::op_isnum },
            { mksym("complex?"), Intern::op_iscpx },
            { mksym("real?"), Intern::op_isreal },
            { mksym("rational?"), Intern::op_israt },
            { mksym("integer?"), Intern::op_isint },
            { mksym("exact?"), Intern::op_isexact },
            { mksym("inexact?"), Intern::op_isinexact },
            { mksym("exact-integer?"), Intern::op_isexactint },
            { mksym("exact->inexact"), Intern::op_ex2inex },
            { mksym("inexact->exact"), Intern::op_inex2ex },
            { mksym("even?"), Intern::op_iseven },
            { mksym("odd?"), Intern::op_isodd },
            { mksym("="), Intern::op_numeq },
            { mksym("<"), Intern::op_numlt },
            { mksym(">"), Intern::op_numgt },
            { mksym("<="), Intern::op_numle },
            { mksym(">="), Intern::op_numge },
            { mksym("+"), Intern::op_add },
            { mksym("-"), Intern::op_sub },
            { mksym("*"), Intern::op_mul },
            { mksym("/"), Intern::op_div },
            { mksym("min"), Intern::op_min },
            { mksym("max"), Intern::op_max },
            { mksym("positive?"), Intern::op_ispos },
            { mksym("negative?"), Intern::op_isneg },
            { mksym("zero?"), Intern::op_zero },
            { mksym("modulo"), Intern::op_mod },
            { mksym("remainder"), Intern::op_rem },
            { mksym("quotient"), Intern::op_quotient },
            { mksym("floor"), Intern::op_floor },
            { mksym("ceil"), Intern::op_ceil },
            { mksym("trunc"), Intern::op_trunc },
            { mksym("round"), Intern::op_round },
            { mksym("sin"), Intern::op_sin },
            { mksym("cos"), Intern::op_cos },
            { mksym("tan"), Intern::op_tan },
            { mksym("asin"), Intern::op_asin },
            { mksym("acos"), Intern::op_acos },
            { mksym("atan"), Intern::op_atan },
            { mksym("sinh"), Intern::op_sinh },
            { mksym("cosh"), Intern::op_cosh },
            { mksym("tanh"), Intern::op_tanh },
            { mksym("asinh"), Intern::op_asinh },
            { mksym("acosh"), Intern::op_acosh },
            { mksym("atanh"), Intern::op_atanh },
            { mksym("sqrt"), Intern::op_sqrt },
            { mksym("cbrt"), Intern::op_cbrt },
            { mksym("exp"), Intern::op_exp },
            { mksym("expt"), Intern::op_pow },
            { mksym("log"), Intern::op_log },
            { mksym("log10"), Intern::op_log10 },
            { mksym("square"), Intern::op_square },
            { mksym("real-part"), Intern::op_real },
            { mksym("imag-part"), Intern::op_imag },
            { mksym("magnitude"), Intern::op_abs },
            { mksym("abs"), Intern::op_abs },
            { mksym("angle"), Intern::op_arg },
            { mksym("make-rectangular"), Intern::op_rect },
            { mksym("make-polar"), Intern::op_polar },
            { mksym("conjugate"), Intern::op_conj },
            { mksym("hypot"), Intern::op_hypot },
            { mksym("string->number"), Intern::op_strnum },
            { mksym("number->string"), Intern::op_numstr },

            /* Section 6.3: Booleans */
            { mksym("not"), Intern::op_not },
            { mksym("boolean?"), Intern::op_isbool },
            { mksym("boolean=?"), Intern::op_isbooleq },

            /* Section 6.4: Pair and lists */
            { mksym("cons"), Intern::op_cons },
            { mksym("car"), Intern::op_car },
            { mksym("cdr"), Intern::op_cdr },
            { mksym("caar"), Intern::op_caar },
            { mksym("cddr"), Intern::op_cddr },
            { mksym("cadr"), Intern::op_cadr },
            { mksym("cdar"), Intern::op_cdar },
            { mksym("caddr"), Intern::op_caddr },
            { mksym("set-car!"), Intern::op_setcar },
            { mksym("set-cdr!"), Intern::op_setcdr },
            { mksym("list"), Intern::op_list },
            { mksym("null?"), Intern::op_isnil },
            { mksym("pair?"), Intern::op_ispair },
            { mksym("list?"), Intern::op_islist },
            { mksym("make-list"), Intern::op_mklist },
            { mksym("append"), Intern::op_append },
            { mksym("length"), Intern::op_length },
            { mksym("list-ref"), Intern::op_listref },
            { mksym("list-set!"), Intern::op_listsetb },
            { mksym("list-copy"), Intern::op_listcopy },
            { mksym("reverse"), Intern::op_reverse },
            { mksym("reverse!"), Intern::op_reverseb },
            { mksym("memq"), Intern::op_memq },
            { mksym("memv"), Intern::op_memv },
            { mksym("member"), Intern::op_member },
            { mksym("assq"), Intern::op_assq },
            { mksym("assv"), Intern::op_assv },
            { mksym("assoc"), Intern::op_assoc },

            /* Section 6.5: Symbols */
            { mksym("symbol?"), Intern::op_issym },
            { mksym("symbol->string"), Intern::op_symstr },
            { mksym("string->symbol"), Intern::op_strsym },
            { mksym("gensym"), Intern::op_gensym },

            /* Section 6.6: Characters */
            { mksym("char?"), Intern::op_ischar },
            { mksym("char->integer"), Intern::op_charint },
            { mksym("integer->char"), Intern::op_intchar },
            { mksym("char=?"), Intern::op_ischareq },
            { mksym("char<?"), Intern::op_ischarlt },
            { mksym("char>?"), Intern::op_ischargt },
            { mksym("char<=?"), Intern::op_ischarle },
            { mksym("char>=?"), Intern::op_ischarge },
            { mksym("char-ci=?"), Intern::op_ischcieq },
            { mksym("char-ci<?"), Intern::op_ischcilt },
            { mksym("char-ci>?"), Intern::op_ischcigt },
            { mksym("char-ci<=?"), Intern::op_ischcile },
            { mksym("char-ci>=?"), Intern::op_ischcige },
            { mksym("char-alphabetic?"), Intern::op_isalpha },
            { mksym("char-numeric?"), Intern::op_isdigit },
            { mksym("char-whitespace?"), Intern::op_iswspace },
            { mksym("char-upper-case?"), Intern::op_isupper },
            { mksym("char-lower-case?"), Intern::op_islower },
            { mksym("digit-value"), Intern::op_digitval },
            { mksym("char-upcase"), Intern::op_upcase },
            { mksym("char-downcase"), Intern::op_downcase },

            /* Section 6.7: Strings */
            { mksym("string?"), Intern::op_isstr },
            { mksym("string"), Intern::op_str },
            { mksym("make-string"), Intern::op_mkstr },
            { mksym("string-ref"), Intern::op_strref },
            { mksym("string-set!"), Intern::op_strsetb },
            { mksym("string-length"), Intern::op_strlen },
            { mksym("string=?"), Intern::op_isstreq },
            { mksym("string<?"), Intern::op_isstrlt },
            { mksym("string>?"), Intern::op_isstrgt },
            { mksym("string<=?"), Intern::op_isstrle },
            { mksym("string>=?"), Intern::op_isstrge },
            { mksym("string-ci=?"), Intern::op_isstrcieq },
            { mksym("string-ci=?"), Intern::op_isstrcieq },
            { mksym("string-ci<?"), Intern::op_isstrcilt },
            { mksym("string-ci>?"), Intern::op_isstrcigt },
            { mksym("string-ci<=?"), Intern::op_isstrcile },
            { mksym("string-ci>=?"), Intern::op_isstrcige },
            { mksym("string-upcase"), Intern::op_strupcase },
            { mksym("string-downcase"), Intern::op_strdowncase },
            { mksym("string-upcase!"), Intern::op_strupcaseb },
            { mksym("string-downcase!"), Intern::op_strdowncaseb },
            { mksym("string-append"), Intern::op_strappend },
            { mksym("string-append!"), Intern::op_strappendb },
            { mksym("string->list"), Intern::op_strlist },
            { mksym("list->string"), Intern::op_liststr },
            { mksym("substring"), Intern::op_substr },
            { mksym("string-copy"), Intern::op_strcopy },
            { mksym("string-copy!"), Intern::op_strcopyb },
            { mksym("string-fill!"), Intern::op_strfillb },

            /* Section 6.8: Vectors */
            { mksym("vector?"), Intern::op_isvec },
            { mksym("make-vector"), Intern::op_mkvec },
            { mksym("vector"), Intern::op_vec },
            { mksym("vector-length"), Intern::op_veclen },
            { mksym("vector-ref"), Intern::op_vecref },
            { mksym("vector-set!"), Intern::op_vecsetb },
            { mksym("vector->list"), Intern::op_veclist },
            { mksym("list->vector"), Intern::op_listvec },
            { mksym("vector-copy"), Intern::op_veccopy },
            { mksym("vector-copy!"), Intern::op_veccopyb },
            { mksym("vector-append"), Intern::op_vecappend },
            { mksym("vector-append!"), Intern::op_vecappendb },
            { mksym("vector-fill!"), Intern::op_vecfillb },

            /* Section 6.9: Bytevectors */

            /* Section 6.10: Control features */
            { mksym("procedure?"), Intern::op_isproc },
            { mksym("map"), Intern::op_map },
            { mksym("for-each"), Intern::op_foreach },
            { mksym("call/cc"), Intern::op_callcc },
            { mksym("call-with-current-continuation"), Intern::op_callcc },

            /* Section 6.11: Exceptions */
            { mksym("error"), Intern::op_error },
            { mksym("exit"), Intern::op_exit },

            /* Section 6.12: Environments and evaluation */
            { mksym("interaction-environment"), Intern::op_replenv },
            { mksym("eval"), Intern::op_eval },
            { mksym("repl"), Intern::op_repl },
            { mksym("macro-expand"), Intern::op_macroexp },

            /* Section 6.13: Input and output */
            { mksym("port?"), Intern::op_isport },
            { mksym("input-port?"), Intern::op_isinport },
            { mksym("output-port?"), Intern::op_isoutport },
            { mksym("textual-port?"), Intern::op_istxtport },
            { mksym("binary-port?"), Intern::op_isbinport },
            { mksym("call-with-input-file"), Intern::op_callw_infile },
            { mksym("call-with-output-file"), Intern::op_callw_outfile },
            { mksym("open-input-file"), Intern::op_open_infile },
            { mksym("open-output-file"), Intern::op_open_outfile },
            { mksym("close-port"), Intern::op_close_port },
            { mksym("close-input-port"), Intern::op_close_inport },
            { mksym("close-output-port"), Intern::op_close_outport },
            { mksym("eof-object?"), Intern::op_iseof },
            { mksym("eof-object"), Intern::op_eof },
            { mksym("flush-output-port"), Intern::op_flush },
            { mksym("read-line"), Intern::op_readline },
            { mksym("read-char"), Intern::op_read_char },
            { mksym("peek-char"), Intern::op_peek_char },
            { mksym("read-string"), Intern::op_read_str },
            { mksym("write"), Intern::op_write },
            { mksym("read"), Intern::op_read },
            { mksym("display"), Intern::op_display },
            { mksym("newline"), Intern::op_newline },
            { mksym("write-char"), Intern::op_write_char },
            { mksym("write-str"), Intern::op_write_str },

            /* Section 6.14: System interface */
            { mksym("load"), Intern::op_load },

            /* Extension: regular expressions */
        }
    };
};

} // namespace pscm

#endif // TYPES_HPP
