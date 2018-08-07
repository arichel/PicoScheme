/*********************************************************************************/ /**
 * @file types.cpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#include <functional>

#include "cell.hpp"
#include "eval.hpp"
#include "parser.hpp"
#include "primop.hpp"
#include "types.hpp"

namespace pscm {

static constexpr size_t dflt_bucket_count = 1024; //<! Initial default hash table bucket count.

//! Global scheme symbol table:
static Symtab symtab(dflt_bucket_count);

//! Top environment, initialized with internal scheme symbols:
static SymenvPtr topenv{
    new SymenvPtr::element_type{
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
        { sym("number?"), Intern::op_isnum },
        { sym("complex?"), Intern::op_iscpx },
        { sym("real?"), Intern::op_isreal },
        { sym("rational?"), Intern::op_israt },
        { sym("integer?"), Intern::op_isint },
        { sym("exact?"), Intern::op_isexact },
        { sym("inexact?"), Intern::op_isinexact },
        { sym("exact-integer?"), Intern::op_isexactint },
        { sym("odd?"), Intern::op_isodd },
        { sym("even?"), Intern::op_iseven },
        { sym("="), Intern::op_numeq },
        { sym("<"), Intern::op_numlt },
        { sym(">"), Intern::op_numgt },
        { sym("<="), Intern::op_numle },
        { sym(">="), Intern::op_numge },
        { sym("+"), Intern::op_add },
        { sym("-"), Intern::op_sub },
        { sym("*"), Intern::op_mul },
        { sym("/"), Intern::op_div },
        { sym("min"), Intern::op_min },
        { sym("max"), Intern::op_max },
        { sym("positive?"), Intern::op_ispos },
        { sym("negative?"), Intern::op_isneg },
        { sym("zero?"), Intern::op_zero },
        { sym("modulo"), Intern::op_mod },
        { sym("remainder"), Intern::op_rem },
        { sym("quotient"), Intern::op_quotient },
        { sym("floor"), Intern::op_floor },
        { sym("ceil"), Intern::op_ceil },
        { sym("trunc"), Intern::op_trunc },
        { sym("round"), Intern::op_round },
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
        { sym("cbrt"), Intern::op_cbrt },
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
        { sym("hypot"), Intern::op_hypot },
        { sym("string->number"), Intern::op_strnum },
        { sym("number->string"), Intern::op_numstr },

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
        { sym("caddr"), Intern::op_caddr },
        { sym("set-car!"), Intern::op_setcar },
        { sym("set-cdr!"), Intern::op_setcdr },
        { sym("list"), Intern::op_list },
        { sym("null?"), Intern::op_isnil },
        { sym("pair?"), Intern::op_ispair },
        { sym("list?"), Intern::op_islist },
        { sym("make-list"), Intern::op_mklist },
        { sym("append"), Intern::op_append },
        { sym("length"), Intern::op_length },
        { sym("list-ref"), Intern::op_listref },
        { sym("list-set!"), Intern::op_listsetb },
        { sym("list-copy"), Intern::op_listcopy },
        { sym("reverse"), Intern::op_reverse },
        { sym("reverse!"), Intern::op_reverseb },
        { sym("memq"), Intern::op_memq },
        { sym("memv"), Intern::op_memv },
        { sym("member"), Intern::op_member },
        { sym("assq"), Intern::op_assq },
        { sym("assv"), Intern::op_assv },
        { sym("assoc"), Intern::op_assoc },

        /* Section 6.5: Symbols */
        { sym("symbol?"), Intern::op_issym },
        { sym("symbol->string"), Intern::op_symstr },
        { sym("string->symbol"), Intern::op_strsym },
        { sym("gensym"), Intern::op_gensym },

        /* Section 6.6: Characters */
        { sym("char?"), Intern::op_ischar },
        { sym("char->integer"), Intern::op_charint },
        { sym("integer->char"), Intern::op_intchar },
        { sym("char=?"), Intern::op_ischareq },
        { sym("char<?"), Intern::op_ischarlt },
        { sym("char>?"), Intern::op_ischargt },
        { sym("char<=?"), Intern::op_ischarle },
        { sym("char>=?"), Intern::op_ischarge },
        { sym("char-ci=?"), Intern::op_ischcieq },
        { sym("char-ci<?"), Intern::op_ischcilt },
        { sym("char-ci>?"), Intern::op_ischcigt },
        { sym("char-ci<=?"), Intern::op_ischcile },
        { sym("char-ci>=?"), Intern::op_ischcige },
        { sym("char-alphabetic?"), Intern::op_isalpha },
        { sym("char-numeric?"), Intern::op_isdigit },
        { sym("char-whitespace?"), Intern::op_iswspace },
        { sym("char-upper-case?"), Intern::op_isupper },
        { sym("char-lower-case?"), Intern::op_islower },
        { sym("digit-value"), Intern::op_digitval },
        { sym("char-upcase"), Intern::op_upcase },
        { sym("char-downcase"), Intern::op_downcase },

        /* Section 6.7: Strings */
        { sym("string?"), Intern::op_isstr },
        { sym("string"), Intern::op_str },
        { sym("make-string"), Intern::op_mkstr },
        { sym("string-ref"), Intern::op_strref },
        { sym("string-set!"), Intern::op_strsetb },
        { sym("string-length"), Intern::op_strlen },
        { sym("string=?"), Intern::op_isstreq },
        { sym("string-ci=?"), Intern::op_isstrcieq },
        { sym("string-upcase"), Intern::op_strupcase },
        { sym("string-downcase"), Intern::op_strdowncase },
        { sym("string-upcase!"), Intern::op_strupcaseb },
        { sym("string-downcase!"), Intern::op_strdowncaseb },
        { sym("string-append"), Intern::op_strappend },
        { sym("string-append!"), Intern::op_strappendb },
        { sym("string->list"), Intern::op_strlist },
        { sym("list->string"), Intern::op_liststr },
        { sym("substring"), Intern::op_substr },
        { sym("string-copy"), Intern::op_strcopy },
        { sym("string-copy!"), Intern::op_strcopyb },
        { sym("string-fill!"), Intern::op_strfillb },

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
        { sym("vector-append!"), Intern::op_vecappendb },
        { sym("vector-fill!"), Intern::op_vecfillb },

        /* Section 6.9: Bytevectors */

        /* Section 6.10: Control features */
        { sym("procedure?"), Intern::op_isproc },
        { sym("map"), Intern::op_map },
        { sym("for-each"), Intern::op_foreach },

        /* Section 6.11: Exceptions */
        { sym("error"), Intern::op_error },
        { sym("exit"), Intern::op_exit },

        /* Section 6.12: Environments and evaluation */
        { sym("interaction-environment"), Intern::op_replenv },
        { sym("eval"), Intern::op_eval },
        { sym("repl"), Intern::op_repl },
        { sym("macro-expand"), Intern::op_macroexp },

        /* Section 6.13: Input and output */
        { sym("port?"), Intern::op_isport },
        { sym("input-port?"), Intern::op_isinport },
        { sym("output-port?"), Intern::op_isoutport },
        { sym("textual-port?"), Intern::op_istxtport },
        { sym("binary-port?"), Intern::op_isbinport },
        { sym("call-with-input-file"), Intern::op_callw_infile },
        { sym("call-with-output-file"), Intern::op_callw_outfile },
        { sym("open-input-file"), Intern::op_open_infile },
        { sym("open-output-file"), Intern::op_open_outfile },
        { sym("eof-object?"), Intern::op_iseof },
        { sym("eof-object"), Intern::op_eof },
        { sym("flush-output-port"), Intern::op_flush },
        { sym("read-line"), Intern::op_readline },
        { sym("read-char"), Intern::op_read_char },
        { sym("peek-char"), Intern::op_peek_char },
        { sym("read-string"), Intern::op_read_str },
        { sym("write"), Intern::op_write },
        { sym("display"), Intern::op_display },
        { sym("newline"), Intern::op_newline },
        { sym("write-char"), Intern::op_write_char },
        { sym("write-str"), Intern::op_write_str },

        /* Section 6.14: System interface */
        { sym("load"), Intern::op_load },
    }
};

Symbol sym(const char* name)
{
    return symtab[name];
}

Symbol gensym()
{
    Symbol::value_type str("symbol ");
    str.append(std::to_string(symtab.size()));
    return symtab[str.c_str()];
}

SymenvPtr senv(const SymenvPtr& env)
{
    return std::make_shared<SymenvPtr::element_type>(env ? env : topenv);
}

void addenv(const Symbol& sym, const Cell& cell, const SymenvPtr& env)
{
    if (env)
        env->add(sym, cell);
    else
        topenv->add(sym, cell);
}

FunctionPtr fun(const Symbol& sym, FunctionPtr::element_type::function_type&& fn,
    const SymenvPtr& env)
{
    auto fptr = std::make_shared<FunctionPtr::element_type>(sym, std::move(fn));

    addenv(sym, fptr, env);
    return fptr;
}

StringPtr str(const Char* s)
{
    return std::make_shared<StringPtr::element_type>(s);
}

VectorPtr vec(Number size, const Cell& val)
{
    using size_type = VectorPtr::element_type::size_type;

    (is_int(size) && get<Int>(size) >= 0)
        || ((void)(throw std::invalid_argument("vector length must be a non-negative integer")), 0);

    return std::make_shared<VectorPtr::element_type>(static_cast<size_type>(get<Int>(size)), val);
}

void load(const std::string& filnam, const SymenvPtr& symenv)
{
    const SymenvPtr& env = symenv ? symenv : topenv;

    std::ifstream in;
    Parser parser;
    Cell expr = none;

    in.exceptions(std::ifstream::badbit);

    try {
        in.open(filnam);
        if (!in.is_open())
            throw std::ios_base::failure("couldn't open input file: '"s + filnam + "'"s);

        do {
            expr = parser.read(in);
            expr = eval(env, expr);
            expr = none;
        } while (!in.eof());

    } catch (const std::exception& e) {
        if (is_none(expr))
            std::cerr << e.what() << '\n';
        else
            std::cerr << e.what() << ": " << expr << '\n';
    }
}

Cell call(const SymenvPtr& senv, const Cell& proc, const std::vector<Cell>& args)
{
    if (is_intern(proc))
        return call(senv, get<Intern>(proc), args);
    else
        return (*get<FunctionPtr>(proc))(senv, args);
}

} // namespace pscm
