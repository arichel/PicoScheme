#include <set>

#include "eval.hpp"
#include "proc.hpp"
#include "stream.hpp"

namespace pscm {

struct Proc::Impl {

    Impl(const Symenv& senv, const Cell& args, const Cell& code)
    {
        if (is_unique_symbol_list(args) && is_pair(code)) {
            _senv = senv;
            _args = args;
            _code = { Intern::_begin, code };

        } else
            throw std::invalid_argument("invalid procedure definition");
    }

    bool is_unique_symbol_list(Cell args)
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

    bool operator!=(const Proc::Impl& impl) const noexcept
    {
        return _senv != impl._senv
            && _args != impl._args
            && _code != impl._code;
    }
    Symenv _senv;
    Cell _args;
    Cons _code;
};

Proc::Proc(Proc&&) noexcept = default;
Proc& Proc::operator=(Proc&&) noexcept = default;
Proc::~Proc() = default;

Proc::Proc(const Symenv& senv, const Cell& args, const Cell& code)
    : impl{ new Impl(senv, args, code) }
{
}

Proc::Proc(const Proc& proc)
    : impl{ new Impl(*proc.impl) }
{
}

Proc& Proc::operator=(const Proc& proc)
{
    impl.reset(new Impl(*proc.impl));
    return *this;
}

bool Proc::operator!=(const Proc& proc) const noexcept
{
    return *impl != *proc.impl;
}

bool Proc::operator==(const Proc& proc) const noexcept
{
    return !(*impl != *proc.impl);
}

std::pair<Symenv, Cell> Proc::apply(const Symenv& senv, Cell args, bool is_list) const
{
    auto newenv = std::make_shared<Symenv::element_type>(impl->_senv);

    Cell iter = impl->_args;

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
    return { newenv, &impl->_code };
}

std::pair<Symenv, Cell> apply(const Symenv& senv, const Cell& proc, const Cell& args, bool is_list)
{
    return std::get<Proc>(proc).apply(senv, args, is_list);
}

}; // namespace pscm
