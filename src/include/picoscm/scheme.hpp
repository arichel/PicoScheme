#ifndef SCHEME_HPP
#define SCHEME_HPP

#include <list>

#include "cell.hpp"

namespace pscm {

VectorPtr mkvec(Number size, const Cell& val);
StringPtr mkstr(const String& s);
StringPtr mkstr(const Char*);
RegexPtr mkregex(const String& str);

class Scheme {
public:
    /**
     * Construct a new cons cell-pair from the global cell store and
     * return a pointer to it.
     *
     * Pointer life time is managed by the garbage collector.
     *
     * @param  car Cell to assign to Cons->first.
     * @param  cdr Cell to assign to Cons->second.
     * @return Pointer to a new Cons pair.
     */
    Cons* cons(Cell&& car, Cell&& cdr);
    Cons* cons(Cell&& car, const Cell& cdr);
    Cons* cons(const Cell& car, Cell&& cdr);
    Cons* cons(const Cell& car, const Cell& cdr);

    //! Recursion base case
    Cell list() { return nil; }

    //! Build a cons list of all arguments
    template <typename T, typename... Args>
    Cons* list(T&& t, Args&&... args)
    {
        return cons(std::forward<T>(t), list(std::forward<Args>(args)...));
    }

    SymenvPtr mkenv(const SymenvPtr& env = nullptr);
    FunctionPtr mkfun(Function::function_type&&);
    FunctionPtr mkfun(const String& name, Function::function_type&&, const SymenvPtr& env = nullptr);

    Symbol mksym(const Char* name);
    Symbol mksym(const String& name);
    Symbol mksym();

    /**
     * Interactive scheme read eval print loop.
     *
     * @param symenv
     *        Optional, unless null-pointer, a shared parent environment pointer.
     *        or use default environment as parent.
     *
     * @param in  Optional, an input stream or use default std::cin.
     * @param out Optional, an output stream or use default std::cout.
     */
    void repl(const SymenvPtr& env = nullptr, std::istream& in = std::cin, std::ostream& out = std::cout);

    void load(const std::string& filnam, const SymenvPtr& env = nullptr);

    /**
     * Evaluate a scheme expression at the argument symbol environment.
     *
     * @param senv Shared pointer to the symbol environment, where to
     *             to evaluate expr.
     * @param expr Scheme expression to evaluate.
     * @return Evaluation result or special symbol @em none for no result.
     */
    Cell eval(SymenvPtr env, Cell expr);

    /**
     * Return a new list of evaluated expressions in argument list.
     *
     * @param senv Symbol environment, where to evaluate the argument list.
     * @param args Argument list to evaluate.
     * @param is_list true:   procedure call argument list.
     *                false:  apply expression argument list, where the last list item
     *                        must be nil or an argument list itself.
     * @return List of evaluated argument expressions.
     */
    Cell eval_list(const SymenvPtr& env, Cell list, bool is_list = true);

    /**
     * Evaluate argument list into an argument vector.
     *
     * @param senv Symbol environment, where to evaluate the argument list.
     * @param args Argument list to evaluate.
     * @param is_list true:   procedure call argument list.
     *                false:  apply expression argument list, where the last list item
     *                        must be nil or an argument list itself.
     * @return Vector of evaluated arguments.
     */
    std::vector<Cell> eval_args(const SymenvPtr& env, Cell args, bool is_list = true);

    /**
     * Call an external function or procedure opcode.
     *
     * @param senv  The current symbol environment.
     * @param proc  Scheme function opcode as defined by enum class @ref pscm::Intern.
     * @param args  Function argument vector.
     * @return Function result or special symbol @ref pscm::none for a void function.
     */
    Cell apply(const SymenvPtr& env, Intern opcode, const std::vector<Cell>& args);
    Cell apply(const SymenvPtr& env, const FunctionPtr& proc, const std::vector<Cell>& args);
    Cell apply(const SymenvPtr& env, const Cell& cell, const std::vector<Cell>& args);
    std::pair<SymenvPtr, Cell> apply(const SymenvPtr& senv, const Cell& proc, const Cell& args, bool is_list = true);

    Cell expand(const Cell& macro, Cell& args);

    /**
     * Evaluate each expression in argument list up the last, which
     * is returned unevaluated. This last expression is evaluated at
     * the call site to support unbound tail-recursion.
     */
    Cell syntax_begin(const SymenvPtr& env, Cell args);

protected:
    Cell syntax_if(const SymenvPtr& env, const Cell& args);

    /**
     * Scheme syntax cond.
     *
     * @verbatim
     * (cond <clause>_1 <clause>_2 ...)
     *
     * <clause> := (<test> <expression> ...)
     *          |  (<test> => <expression> ...)
     *          |  (else  <expression> ...)
     * @endverbatim
     */
    Cell syntax_cond(const SymenvPtr& env, Cell args);

    Cell syntax_when(const SymenvPtr& env, Cell args);

    Cell syntax_unless(const SymenvPtr& env, Cell args);

    Cell syntax_and(const SymenvPtr& env, Cell args);

    Cell syntax_or(const SymenvPtr& env, Cell args);

private:
    friend class GCollector;

    static constexpr size_t dflt_bucket_count = 1024; //<! Initial default hash table bucket count.
    Symtab symtab{ dflt_bucket_count };

    std::list<Cons> store;

    SymenvPtr topenv = Symenv::create(
        {
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
            { mksym("gc"), Intern::op_gc },
            { mksym("gc-dump"), Intern::op_gcdump },
            { mksym("macro-expand"), Intern::op_macroexp },

            /* Section 6.13: Input and output */
            // input-port-open?
            // output-port-open?

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
            { mksym("regex"), Intern::op_regex },
            { mksym("regex-match"), Intern::op_regex_match },
            { mksym("regex-search"), Intern::op_regex_search },

            { mksym("use-count"), Intern::op_usecount },
        });
};
} // namespace pscm

#endif // SCHEME_HPP
