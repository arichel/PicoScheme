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
        { sym("cons"), Intern::op_cons },
        { sym("car"), Intern::op_car },
        { sym("cdr"), Intern::op_cdr },
        { sym("set-car!"), Intern::op_setcar },
        { sym("set-cdr!"), Intern::op_setcdr },
        { sym("list"), Intern::op_list },

        { sym("+"), Intern::op_add },
        { sym("-"), Intern::op_sub },
        { sym("*"), Intern::op_mul },
        { sym("/"), Intern::op_div },
    }
};

Symenv senv(const Symenv& env)
{
    return std::make_shared<Symenv::element_type>(env ? env : topenv);
}

}; // namespace pscm
