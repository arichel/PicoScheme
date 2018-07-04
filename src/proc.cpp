#include <set>

#include "eval.hpp"
#include "proc.hpp"
#include "stream.hpp"

namespace pscm {

Procedure::Procedure(const Symenv& senv, const Cell& args, const Cell& code)
{
    if (is_unique_symbol_list(args) && is_pair(code)) {
        _senv = senv;
        _args = args;
        _code = { Intern::_begin, code };

    } else
        throw std::invalid_argument("invalid procedure definition");
}

Cell Procedure::code() const
{
    return const_cast<Cons*>(&_code);
}

bool Procedure::is_unique_symbol_list(Cell args)
{
    using std::get;

    if (is_nil(args) || is_symbol(args))
        return true;

    std::set<Symbol::key_type> symset;

    for (/* */; is_pair(args); args = cdr(args)) {
        Cell sym = car(args);

        if (!is_symbol(sym) || !symset.insert(get<Symbol>(sym)).second)
            return false;
    }
    return is_nil(args) || (is_symbol(args) && symset.insert(get<Symbol>(args)).second);
}

std::pair<Symenv, Cell> Procedure::apply(const Symenv& senv, Cell args, bool is_list)
{
    auto newenv = std::make_shared<Symenv::element_type>(_senv);

    Cell iter = _args;

    if (is_list)
        for (/* */; is_pair(iter) && is_pair(args); iter = cdr(iter), args = cdr(args)) {

            newenv->add(car(iter), eval(senv, car(args)));
        }

    else
        for (/* */; is_pair(iter) && is_pair(args); iter = cdr(iter), args = cdr(args))
            if (is_pair(cdr(args)))
                newenv->add(car(iter), eval(senv, car(args)));

            else {
                args = eval(senv, car(args));

                for (/* */; is_pair(iter) && is_pair(args); iter = cdr(iter), args = cdr(args))
                    newenv->add(car(iter), car(args));

                is_nil(args) || (throw std::invalid_argument("invalid apply list"), 0);
                break;
            }
    if (iter != args) {
        is_symbol(iter) || (throw std::invalid_argument("invalid procedure arguments"), 0);

        newenv->add(iter, eval_list(senv, args));
    }
    return { newenv, (Cons*)&_code };
}

//struct ProcedureImpl {
//    bool is_unique_symbol_list(Cell args);

//    Symenv _senv;
//    Cell _args;
//    Cons _code;
//};

}; // namespace pscm
