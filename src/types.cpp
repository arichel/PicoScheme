/*********************************************************************************/ /**
 * @file types.cpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/

#include "types.hpp"
#include "cell.hpp"

namespace pscm {

static constexpr size_t dflt_bucket_count = 1024; //<! Initial default hash table bucket count.

//! Global scheme symbol table:
static SymbolTable<String::element_type> symtab(dflt_bucket_count);

//! Top environment, initialized with internal scheme symbols:
static Symenv topenv{
    new Symenv::element_type{
        { sym("#t"), true },
        { sym("#true"), true },
        { sym("#f"), false },
        { sym("#false"), false },
        { sym("or"), Intern::_or },
        { sym("and"), Intern::_and },
        { sym("if"), Intern::_if },
        { sym("cond"), Intern::_cond },
        { sym("else"), Intern::_else },
        { sym("=>"), Intern::_arrow },
        { sym("when"), Intern::_when },
        { sym("unless"), Intern::_unless },
        { sym("begin"), Intern::_begin },
        { sym("define"), Intern::_define },
        { sym("set!"), Intern::_setb },
        { sym("apply"), Intern::_apply },
        { sym("lambda"), Intern::_lambda },
        { sym("define-macro"), Intern::_macro },
        { sym("quote"), Intern::_quote },
        { sym("quasiquote"), Intern::_quasiquote },
        { sym("unquote"), Intern::_unquote },
        { sym("unquote-splicing"), Intern::_unquotesplice },

        /* Section 6.1: Equivalence predicates */
        { sym("eq?"), Intern::op_eq },
        { sym("eqv?"), Intern::op_eqv },
        { sym("equal?"), Intern::op_equal },

        /* Section 6.2: Numbers */
        { sym("="), Intern::op_numeq },
        { sym("<"), Intern::op_numlt },
        { sym(">"), Intern::op_numgt },
        { sym("<="), Intern::op_numle },
        { sym(">="), Intern::op_numge },
        { sym("+"), Intern::op_add },
        { sym("-"), Intern::op_sub },
        { sym("*"), Intern::op_mul },
        { sym("/"), Intern::op_div },
        { sym("zero?"), Intern::op_zero },
        { sym("sin"), Intern::op_sin },
        { sym("cos"), Intern::op_cos },
        { sym("tan"), Intern::op_tan },
        { sym("asin"), Intern::op_asin },
        { sym("acos"), Intern::op_acos },
        { sym("atan"), Intern::op_atan },
        { sym("sinh"), Intern::op_sinh },
        { sym("cosh"), Intern::op_cosh },
        { sym("tanh"), Intern::op_tanh },
        { sym("asinh"), Intern::op_asinh },
        { sym("acosh"), Intern::op_acosh },
        { sym("atanh"), Intern::op_atanh },
        { sym("sqrt"), Intern::op_sqrt },
        { sym("exp"), Intern::op_exp },
        { sym("expt"), Intern::op_pow },
        { sym("log"), Intern::op_log },
        { sym("log10"), Intern::op_log10 },
        { sym("square"), Intern::op_square },
        { sym("real-part"), Intern::op_real },
        { sym("imag-part"), Intern::op_imag },
        { sym("magnitude"), Intern::op_abs },
        { sym("abs"), Intern::op_abs },
        { sym("angle"), Intern::op_arg },
        { sym("make-rectangular"), Intern::op_rect },
        { sym("make-polar"), Intern::op_polar },
        { sym("conjugate"), Intern::op_conj },

        /* Section 6.3: Booleans */
        { sym("not"), Intern::op_not },
        { sym("boolean?"), Intern::op_isbool },
        { sym("boolean=?"), Intern::op_isbooleq },

        /* Section 6.4: Pair and lists */
        { sym("cons"), Intern::op_cons },
        { sym("car"), Intern::op_car },
        { sym("cdr"), Intern::op_cdr },
        { sym("caar"), Intern::op_caar },
        { sym("cddr"), Intern::op_cddr },
        { sym("cadr"), Intern::op_cadr },
        { sym("cdar"), Intern::op_cdar },
        { sym("set-car!"), Intern::op_setcar },
        { sym("set-cdr!"), Intern::op_setcdr },
        { sym("list"), Intern::op_list },
        { sym("pair?"), Intern::op_ispair },
        { sym("list?"), Intern::op_islist },
        { sym("make-list"), Intern::op_mklist },
        { sym("append"), Intern::op_append },
        { sym("length"), Intern::op_length },
        { sym("list-ref"), Intern::op_listref },
        { sym("list-set!"), Intern::op_listsetb },
        { sym("reverse"), Intern::op_reverse },
        { sym("reverse!"), Intern::op_reverseb },

        /* Section 6.5: Symbols */
        { sym("symbol?"), Intern::op_issym },
        { sym("symbol->string"), Intern::op_symstr },
        { sym("string->symbol"), Intern::op_strsym },

        /* Section 6.6: Characters */
        { sym("char?"), Intern::op_ischar },
        { sym("char->integer"), Intern::op_charint },

        /* Section 6.7: Strings */
        { sym("string?"), Intern::op_isstr },
        { sym("make-string"), Intern::op_mkstr },

        /* Section 6.8: Vectors */
        { sym("vector?"), Intern::op_isvec },
        { sym("make-vector"), Intern::op_mkvec },
        { sym("vector"), Intern::op_vec },
        { sym("vector-length"), Intern::op_veclen },
        { sym("vector-ref"), Intern::op_vecref },
        { sym("vector-set!"), Intern::op_vecsetb },
        { sym("vector->list"), Intern::op_veclist },
        { sym("list->vector"), Intern::op_listvec },
        { sym("vector-copy"), Intern::op_veccopy },
        { sym("vector-copy!"), Intern::op_veccopyb },
        { sym("vector-append"), Intern::op_vecappend },
        { sym("vector-fill!"), Intern::op_vecfillb },

        /* Section 6.9: Bytevectors */

        /* Section 6.10: Control features */
        { sym("procedure?"), Intern::op_isproc },

        /* Section 6.11: Exceptions */
        /* Section 6.12: Environments and evaluation */
        { sym("exit"), Intern::op_exit },
        { sym("interaction-environment"), Intern::op_replenv },
        { sym("eval"), Intern::op_eval },
        { sym("macro-expand"), Intern::op_macroexp },

        /* Section 6.13: Input and output */
        { sym("port?"), Intern::op_isport },
        { sym("input-port?"), Intern::op_isinport },
        { sym("output-port?"), Intern::op_isoutport },
        { sym("textual-port?"), Intern::op_istxtport },
        { sym("binary-port?"), Intern::op_isbinport },
        { sym("call-with-input-file"), Intern::op_callw_infile },
        { sym("open-input-file"), Intern::op_open_infile },
        { sym("open-output-file"), Intern::op_open_outfile },
        { sym("read-line"), Intern::op_readline },
    }
};

Symbol sym(const char* name)
{
    return symtab[name];
}

Symenv senv(const Symenv& env)
{
    return std::make_shared<Symenv::element_type>(env ? env : topenv);
}

String str(const Char* s)
{
    return std::make_shared<String::element_type>(s);
}

VectorPtr vec(Number size, const Cell& val)
{
    using size_type = VectorPtr::element_type::size_type;

    (is_int(size) && get<Int>(size) >= 0)
        || ((void)(throw std::invalid_argument("vector length must be a non-negative integer")), 0);

    return std::make_shared<VectorPtr::element_type>(static_cast<size_type>(get<Int>(size)), val);
}

} // namespace pscm
