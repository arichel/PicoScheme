#include <iostream>
#include <string>
#include <variant>
#include <fstream>
#include <assert.h>

#include "cell.hpp"

using namespace std;

namespace pscm {

template<class T>
struct always_false : std::false_type
{
};

template<typename... Ts>
struct overloads : Ts...
{
    using Ts::operator()...;
};

template<typename... Ts>
overloads(Ts...) -> overloads<Ts...>;

template<typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator << (std::basic_ostream<CharT, Traits>&, const Cell&);

template<typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator << (std::basic_ostream<CharT, Traits>& os, const Cons& cons)
{
    const Cons *iter = &cons;

    os << '(' << iter->first;

    while(is_pair(iter->second))
    {
        iter = type<Cons*>(iter->second);
        os << ' ' << iter->first;
    }
    if (is_nil(iter->second))
        os << ')';
    else
        os << " . " << iter->second << ')';

    return os;
}

template<typename CharT, typename Traits>
std::basic_ostream<CharT,Traits>& operator << (std::basic_ostream<CharT, Traits>& os, const Cell& args)
{
    overloads fun
    {
    [&os](Nil)         {os << "()";},
    [&os](None)        {os << "#none";},
    [&os](Bool arg)    {os << (arg ? "#T" : "#F");},
    [&os](Int arg)     {os << arg;},
    [&os](Float arg)   {os << arg;},
    [&os](String* arg) {os << '"' << *arg << '"';},
    [&os](Port*)       {os << "port";},
    [&os](Cons* arg)   {os << *arg;},
    [&os](Func*)       {os << "function";},
    [](auto arg)       {static_assert(always_false<decltype(arg)>::value, "callable overload is missing");}
    };
    std::visit(fun, static_cast<const Cell::base_type&>(args));
    return os;
}

Cell fun_write(const Cell& args)
{
    Port *port = is_nil(cdr(args)) ? &std::cout : std::get<Port*>(cadr(args));
    *port << car(args);
    return none;
}

template<typename Fun>
void foreach(Fun&& fun, const Cell& list)
{
    Cell iter = list;

    while(!is_nil(iter))
    {
        fun(car(iter));
        iter = cdr(iter);
    }
}

Cell fun_foreach(const Cell& args)
{
    auto fun = type<Func*>(car(args));

    foreach(*fun, cdr(args));
    return none;
}

}; // namespace pscm
