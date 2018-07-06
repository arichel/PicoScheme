#include "symbol.hpp"
#include "cell.hpp"

namespace pscm {

static Symtab symtab;

Symbol sym(const char* name) { return symtab[name]; }

static Symenv topenv{
    new Symenv::element_type{
        { sym("#t"), true },
        { sym("#true"), true },
        { sym("#f"), false },
        { sym("#false"), false },
        { sym("or"), Intern::_or },
        { sym("and"), Intern::_and },
        { sym("if"), Intern::_if },
        { sym("quote"), Intern::_quote },
        { sym("begin"), Intern::_begin },
        { sym("cond"), Intern::_cond },
        { sym("define"), Intern::_define },
        { sym("set!"), Intern::_setb },
        { sym("apply"), Intern::_apply },
        { sym("lambda"), Intern::_lambda },

        /*-------------------------------*/
        { sym("eq?"), Intern::op_eq },
        { sym("eqv?"), Intern::op_eqv },
        { sym("equal?"), Intern::op_equal },
        { sym("cons"), Intern::op_cons },
        { sym("car"), Intern::op_car },
        { sym("cdr"), Intern::op_cdr },
        { sym("set-car!"), Intern::op_setcar },
        { sym("set-cdr!"), Intern::op_setcdr },
        { sym("list"), Intern::op_list },

        /*-------------------------------*/
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
    }
};

Symenv senv(const Symenv& env)
{
    return std::make_shared<Symenv::element_type>(env ? env : topenv);
}

}; // namespace pscm
