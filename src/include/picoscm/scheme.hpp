#ifndef SCHEME_HPP
#define SCHEME_HPP

#include <list>

#include "cell.hpp"
#include "gc.hpp"

namespace pscm {

class Scheme {
public:
    Scheme(/*const SymenvPtr& env = nullptr*/)
    {
        add_contants(*this, topenv);
    }
    /**
     * Construct a new cons cell-pair from the internal cell store and
     * return a pointer to it. Pointer lifetime is managed by the
     * garbage collector.
     *
     * @param  car Cell to assign to Cons->first.
     * @param  cdr Cell to assign to Cons->second.
     * @return Pointer to a new Cons pair.
     */
    template <typename CAR, typename CDR>
    Cons* cons(CAR&& car, CDR&& cdr)
    {
        if (store_size < store.size() && !(store.size() % dflt_gc_cycle)) {
            gc.collect(*this, topenv);
            store_size = store.size();
        }
        return pscm::cons(store, std::forward<CAR>(car), std::forward<CDR>(cdr));
    }

    //! Build a cons list of all arguments
    template <typename T, typename... Args>
    Cons* list(T&& t, Args&&... args)
    {
        return pscm::list(store, std::forward<T>(t), std::forward<Args>(args)...);
    }

    //! Return a
    template <typename StringT>
    Symbol symbol(const StringT& str)
    {
        return symtab[string_convert<Char>(str)];
    }

    Symbol symbol()
    {
        return symbol(std::string{ "symbol " }.append(std::to_string(symtab.size())));
    }

    /**
     * Create a new Function object and add it to the argument environment bound
     * to a symbol of argument name.
     *
     * External function signature:
     *   func(Scheme& scm, const SymenvPtr& env, const std::vector<Cell>& argv) -> Cell
     *
     * @param env  Environment pointer, where to add this function. If null-pointer,
     *             add to top-environment.
     * @param name Symbol name of this function.
     * @param fun  External function or functor.
     */
    template <typename StringT, typename FunctionT>
    FunctionPtr function(const SymenvPtr& env, const StringT& name, FunctionT&& fun)
    {
        auto sym = symbol(name);
        auto funptr = Function::create(sym, std::forward<FunctionT>(fun));

        if (env)
            env->add(sym, funptr);
        else
            topenv->add(sym, funptr);

        return funptr;
    }

    template <typename FunctionT>
    FunctionPtr function(const SymenvPtr& env, FunctionT&& fun)
    {
        return function(env, String{ L"λ" }, std::forward<FunctionT>(fun));
    }

    void addenv(const Symbol& sym, const Cell& cell) { topenv->add(sym, cell); }

    SymenvPtr newenv(const SymenvPtr& env = nullptr)
    {
        return Symenv::create(env ? env : topenv);
    }

    PortPtr stdin() { return m_stdin; }
    PortPtr stdout() { return m_stdout; }

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
    void repl(const SymenvPtr& env = nullptr);

    void load(const String& filename, const SymenvPtr& env = nullptr);

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

    // clang-format off
    static void add_contants (Scheme& scm, const SymenvPtr& env);
//    static void add_syntax   (Scheme& scm, const SymenvPtr& env) const;
//    static void add_numeric  (Scheme& scm, const SymenvPtr& env) const;
//    static void add_char     (Scheme& scm, const SymenvPtr& env) const;
//    static void add_boolean  (Scheme& scm, const SymenvPtr& env) const;
//    static void add_symbol   (Scheme& scm, const SymenvPtr& env) const;
//    static void add_list     (Scheme& scm, const SymenvPtr& env) const;
//    static void add_string   (Scheme& scm, const SymenvPtr& env) const;
//    static void add_vector   (Scheme& scm, const SymenvPtr& env) const;
//    static void add_bytevec  (Scheme& scm, const SymenvPtr& env) const;
//    static void add_control  (Scheme& scm, const SymenvPtr& env) const;
//    static void add_except   (Scheme& scm, const SymenvPtr& env) const;
//    static void add_evalenv  (Scheme& scm, const SymenvPtr& env) const;
//    static void add_port     (Scheme& scm, const SymenvPtr& env) const;
    // clang-format on

    //void make_environment(const SymenvPtr& parent = nullptr);

    static constexpr size_t dflt_bucket_count = 1024; //<! Initial default hash table bucket count.
    static constexpr size_t dflt_gc_cycle = 10000;

    GCollector gc;
    Symtab symtab{ dflt_bucket_count };
    std::list<Cons> store;
    size_t store_size = 0;

    using standard_port = StandardPort<Char>;
    PortPtr m_stdin = std::make_shared<standard_port>(standard_port::in);
    PortPtr m_stdout = std::make_shared<standard_port>(standard_port::out);

    SymenvPtr topenv = Symenv::create(
        {
            { symbol("#true"), true },
            { symbol("#t"), true },
            { symbol("#f"), false },
            { symbol("#false"), false },
            { symbol("or"), Intern::_or },
            { symbol("and"), Intern::_and },
            { symbol("if"), Intern::_if },
            { symbol("cond"), Intern::_cond },
            { symbol("else"), Intern::_else },
            { symbol("=>"), Intern::_arrow },
            { symbol("when"), Intern::_when },
            { symbol("unless"), Intern::_unless },
            { symbol("begin"), Intern::_begin },
            { symbol("define"), Intern::_define },
            { symbol("set!"), Intern::_setb },
            { symbol("lambda"), Intern::_lambda },
            { symbol("define-macro"), Intern::_macro },
            { symbol("quote"), Intern::_quote },
            { symbol("quasiquote"), Intern::_quasiquote },
            { symbol("unquote"), Intern::_unquote },
            { symbol("unquote-splicing"), Intern::_unquotesplice },
            { symbol("apply"), Intern::_apply },

            /* Section 6.1: Equivalence predicates */
            { symbol("eq?"), Intern::op_eq },
            { symbol("eqv?"), Intern::op_eqv },
            { symbol("equal?"), Intern::op_equal },

            /* Section 6.2: Numbers */
            { symbol("number?"), Intern::op_isnum },
            { symbol("complex?"), Intern::op_iscpx },
            { symbol("real?"), Intern::op_isreal },
            { symbol("rational?"), Intern::op_israt },
            { symbol("integer?"), Intern::op_isint },
            { symbol("exact?"), Intern::op_isexact },
            { symbol("inexact?"), Intern::op_isinexact },
            { symbol("exact-integer?"), Intern::op_isexactint },
            { symbol("exact->inexact"), Intern::op_ex2inex },
            { symbol("inexact->exact"), Intern::op_inex2ex },
            { symbol("even?"), Intern::op_iseven },
            { symbol("odd?"), Intern::op_isodd },
            { symbol("="), Intern::op_numeq },
            { symbol("<"), Intern::op_numlt },
            { symbol(">"), Intern::op_numgt },
            { symbol("<="), Intern::op_numle },
            { symbol(">="), Intern::op_numge },
            { symbol("+"), Intern::op_add },
            { symbol("-"), Intern::op_sub },
            { symbol("*"), Intern::op_mul },
            { symbol("/"), Intern::op_div },
            { symbol("min"), Intern::op_min },
            { symbol("max"), Intern::op_max },
            { symbol("positive?"), Intern::op_ispos },
            { symbol("negative?"), Intern::op_isneg },
            { symbol("zero?"), Intern::op_zero },
            { symbol("modulo"), Intern::op_mod },
            { symbol("remainder"), Intern::op_rem },
            { symbol("quotient"), Intern::op_quotient },
            { symbol("floor"), Intern::op_floor },
            { symbol("ceil"), Intern::op_ceil },
            { symbol("trunc"), Intern::op_trunc },
            { symbol("round"), Intern::op_round },
            { symbol("sin"), Intern::op_sin },
            { symbol("cos"), Intern::op_cos },
            { symbol("tan"), Intern::op_tan },
            { symbol("asin"), Intern::op_asin },
            { symbol("acos"), Intern::op_acos },
            { symbol("atan"), Intern::op_atan },
            { symbol("sinh"), Intern::op_sinh },
            { symbol("cosh"), Intern::op_cosh },
            { symbol("tanh"), Intern::op_tanh },
            { symbol("asinh"), Intern::op_asinh },
            { symbol("acosh"), Intern::op_acosh },
            { symbol("atanh"), Intern::op_atanh },
            { symbol("sqrt"), Intern::op_sqrt },
            { symbol("cbrt"), Intern::op_cbrt },
            { symbol("exp"), Intern::op_exp },
            { symbol("expt"), Intern::op_pow },
            { symbol("log"), Intern::op_log },
            { symbol("log10"), Intern::op_log10 },
            { symbol("square"), Intern::op_square },
            { symbol("real-part"), Intern::op_real },
            { symbol("imag-part"), Intern::op_imag },
            { symbol("magnitude"), Intern::op_abs },
            { symbol("abs"), Intern::op_abs },
            { symbol("angle"), Intern::op_arg },
            { symbol("make-rectangular"), Intern::op_rect },
            { symbol("make-polar"), Intern::op_polar },
            { symbol("conjugate"), Intern::op_conj },
            { symbol("hypot"), Intern::op_hypot },
            { symbol("string->number"), Intern::op_strnum },
            { symbol("number->string"), Intern::op_numstr },

            /* Section 6.3: Booleans */
            { symbol("not"), Intern::op_not },
            { symbol("boolean?"), Intern::op_isbool },
            { symbol("boolean=?"), Intern::op_isbooleq },

            /* Section 6.4: Pair and lists */
            { symbol("cons"), Intern::op_cons },
            { symbol("car"), Intern::op_car },
            { symbol("cdr"), Intern::op_cdr },
            { symbol("caar"), Intern::op_caar },
            { symbol("cddr"), Intern::op_cddr },
            { symbol("cadr"), Intern::op_cadr },
            { symbol("cdar"), Intern::op_cdar },
            { symbol("caddr"), Intern::op_caddr },
            { symbol("set-car!"), Intern::op_setcar },
            { symbol("set-cdr!"), Intern::op_setcdr },
            { symbol("list"), Intern::op_list },
            { symbol("null?"), Intern::op_isnil },
            { symbol("pair?"), Intern::op_ispair },
            { symbol("list?"), Intern::op_islist },
            { symbol("make-list"), Intern::op_mklist },
            { symbol("append"), Intern::op_append },
            { symbol("length"), Intern::op_length },
            { symbol("list-ref"), Intern::op_listref },
            { symbol("list-set!"), Intern::op_listsetb },
            { symbol("list-copy"), Intern::op_listcopy },
            { symbol("reverse"), Intern::op_reverse },
            { symbol("reverse!"), Intern::op_reverseb },
            { symbol("memq"), Intern::op_memq },
            { symbol("memv"), Intern::op_memv },
            { symbol("member"), Intern::op_member },
            { symbol("assq"), Intern::op_assq },
            { symbol("assv"), Intern::op_assv },
            { symbol("assoc"), Intern::op_assoc },

            /* Section 6.5: Symbols */
            { symbol("symbol?"), Intern::op_issym },
            { symbol("symbol->string"), Intern::op_symstr },
            { symbol("string->symbol"), Intern::op_strsym },
            { symbol("gensym"), Intern::op_gensym },

            /* Section 6.6: Characters */
            { symbol("char?"), Intern::op_ischar },
            { symbol("char->integer"), Intern::op_charint },
            { symbol("integer->char"), Intern::op_intchar },
            { symbol("char=?"), Intern::op_ischareq },
            { symbol("char<?"), Intern::op_ischarlt },
            { symbol("char>?"), Intern::op_ischargt },
            { symbol("char<=?"), Intern::op_ischarle },
            { symbol("char>=?"), Intern::op_ischarge },
            { symbol("char-ci=?"), Intern::op_ischcieq },
            { symbol("char-ci<?"), Intern::op_ischcilt },
            { symbol("char-ci>?"), Intern::op_ischcigt },
            { symbol("char-ci<=?"), Intern::op_ischcile },
            { symbol("char-ci>=?"), Intern::op_ischcige },
            { symbol("char-alphabetic?"), Intern::op_isalpha },
            { symbol("char-numeric?"), Intern::op_isdigit },
            { symbol("char-whitespace?"), Intern::op_iswspace },
            { symbol("char-upper-case?"), Intern::op_isupper },
            { symbol("char-lower-case?"), Intern::op_islower },
            { symbol("digit-value"), Intern::op_digitval },
            { symbol("char-upcase"), Intern::op_upcase },
            { symbol("char-downcase"), Intern::op_downcase },

            /* Section 6.7:using namespace std::string_literals; Strings */
            { symbol("string?"), Intern::op_isstr },
            { symbol("string"), Intern::op_str },
            { symbol("make-string"), Intern::op_mkstr },
            { symbol("string-ref"), Intern::op_strref },
            { symbol("string-set!"), Intern::op_strsetb },
            { symbol("string-length"), Intern::op_strlen },
            { symbol("string=?"), Intern::op_isstreq },
            { symbol("string<?"), Intern::op_isstrlt },
            { symbol("string>?"), Intern::op_isstrgt },
            { symbol("string<=?"), Intern::op_isstrle },
            { symbol("string>=?"), Intern::op_isstrge },
            { symbol("string-ci=?"), Intern::op_isstrcieq },
            { symbol("string-ci=?"), Intern::op_isstrcieq },
            { symbol("string-ci<?"), Intern::op_isstrcilt },
            { symbol("string-ci>?"), Intern::op_isstrcigt },
            { symbol("string-ci<=?"), Intern::op_isstrcile },
            { symbol("string-ci>=?"), Intern::op_isstrcige },
            { symbol("string-upcase"), Intern::op_strupcase },
            { symbol("string-downcase"), Intern::op_strdowncase },
            { symbol("string-upcase!"), Intern::op_strupcaseb },
            { symbol("string-downcase!"), Intern::op_strdowncaseb },
            { symbol("string-append"), Intern::op_strappend },
            { symbol("string-append!"), Intern::op_strappendb },
            { symbol("string->list"), Intern::op_strlist },
            { symbol("list->string"), Intern::op_liststr },
            { symbol("substring"), Intern::op_substr },
            { symbol("string-copy"), Intern::op_strcopy },
            { symbol("string-copy!"), Intern::op_strcopyb },
            { symbol("string-fill!"), Intern::op_strfillb },

            /* Section 6.8: Vectors */
            { symbol("vector?"), Intern::op_isvec },
            { symbol("make-vector"), Intern::op_mkvec },
            { symbol("vector"), Intern::op_vec },
            { symbol("vector-length"), Intern::op_veclen },
            { symbol("vector-ref"), Intern::op_vecref },
            { symbol("vector-set!"), Intern::op_vecsetb },
            { symbol("vector->list"), Intern::op_veclist },
            { symbol("list->vector"), Intern::op_listvec },
            { symbol("vector-copy"), Intern::op_veccopy },
            { symbol("vector-copy!"), Intern::op_veccopyb },
            { symbol("vector-append"), Intern::op_vecappend },
            { symbol("vector-append!"), Intern::op_vecappendb },
            { symbol("vector-fill!"), Intern::op_vecfillb },

            /* Section 6.9: Bytevectors */

            /* Section 6.10: Control features */
            { symbol("procedure?"), Intern::op_isproc },
            { symbol("map"), Intern::op_map },
            { symbol("for-each"), Intern::op_foreach },
            { symbol("call/cc"), Intern::op_callcc },
            { symbol("call-with-current-continuation"), Intern::op_callcc },
            { symbol("call-with-values"), Intern::op_callwval },

            /* Section 6.11: Exceptions */
            { symbol("error"), Intern::op_error },
            { symbol("with-exception-handler"), Intern::op_with_exception },
            { symbol("exit"), Intern::op_exit },

            /* Section 6.12: Environments and evaluation */
            { symbol("interaction-environment"), Intern::op_replenv },
            { symbol("eval"), Intern::op_eval },
            { symbol("repl"), Intern::op_repl },
            { symbol("gc"), Intern::op_gc },
            { symbol("gc-dump"), Intern::op_gcdump },
            { symbol("macro-expand"), Intern::op_macroexp },

            /* Section 6.13: Input and output */
            // input-port-open?
            // output-port-open?

            { symbol("port?"), Intern::op_isport },
            { symbol("input-port?"), Intern::op_isinport },
            { symbol("output-port?"), Intern::op_isoutport },
            { symbol("textual-port?"), Intern::op_istxtport },
            { symbol("binary-port?"), Intern::op_isbinport },
            { symbol("call-with-input-file"), Intern::op_callw_infile },
            { symbol("call-with-output-file"), Intern::op_callw_outfile },
            { symbol("open-input-file"), Intern::op_open_infile },
            { symbol("open-output-file"), Intern::op_open_outfile },
            { symbol("close-port"), Intern::op_close_port },
            { symbol("close-input-port"), Intern::op_close_inport },
            { symbol("close-output-port"), Intern::op_close_outport },
            { symbol("eof-object?"), Intern::op_iseof },
            { symbol("eof-object"), Intern::op_eof },
            { symbol("flush-output-port"), Intern::op_flush },
            { symbol("read-line"), Intern::op_readline },
            { symbol("read-char"), Intern::op_read_char },
            { symbol("peek-char"), Intern::op_peek_char },
            { symbol("read-string"), Intern::op_read_str },
            { symbol("write"), Intern::op_write },
            { symbol("read"), Intern::op_read },
            { symbol("display"), Intern::op_display },
            { symbol("newline"), Intern::op_newline },
            { symbol("write-char"), Intern::op_write_char },
            { symbol("write-str"), Intern::op_write_str },

            /* Section 6.14: System interface */
            { symbol("load"), Intern::op_load },

            /* Extension: regular expressions */
            { symbol("regex"), Intern::op_regex },
            { symbol("regex-match"), Intern::op_regex_match },
            { symbol("regex-search"), Intern::op_regex_search },

            { symbol("use-count"), Intern::op_usecount },
        });
};

template <typename T>
void load(Scheme& scm, const T& filename, const SymenvPtr& env = nullptr)
{
    scm.load(string_convert<Char>(filename), env);
}

template <typename CharT>
StringPtr str(const CharT* str)
{
    return std::make_shared<String>(string_convert<Char>(str));
}

template <typename StringT, typename = std::enable_if<!std::is_pointer_v<StringT>>>
StringPtr str(const StringT& str)
{
    return std::make_shared<String>(string_convert<Char>(str));
}

template <typename StringT, typename FunctionT>
FunctionPtr fun(Scheme& scm, const SymenvPtr& env, const StringT& name, FunctionT&& fun)
{
    return scm.function(env, name, std::forward<FunctionT>(fun));
}

template <typename StringT, typename FunctionT>
FunctionPtr fun(Scheme& scm, const StringT& name, FunctionT&& fun)
{
    return scm.function(/*env*/ nullptr, name, std::forward<FunctionT>(fun));
}

template <typename T>
VectorPtr vec(size_t size, T&& val)
{
    return std::make_shared<VectorPtr::element_type>(size, std::forward<T>(val));
}

template <typename StringT>
RegexPtr regex(const StringT& str)
{
    using regex = RegexPtr::element_type;
    regex::flag_type flags = regex::ECMAScript | regex::icase;
    return std::make_shared<RegexPtr::element_type>(string_convert<Char>(str), flags);
}

} // namespace pscm

#endif // SCHEME_HPP
