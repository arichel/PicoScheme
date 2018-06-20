#include <iostream>
#include <string>
#include <variant>
#include <fstream>
#include <assert.h>

#include "cell.hpp"


namespace pscm {

/**
 * @brief Build a vistor type for std::visit, where the function application
 *        operator () is overloaded for each template argument type.
 */
template<typename... Ts> struct overloads : Ts... {using Ts::operator()...;};
template<typename... Ts> overloads(Ts...) -> overloads<Ts...>; //!< construct overloads type
template<class T> struct always_false : std::false_type {};    //!< return false for every type

//! Forward declaration of the output stream operator for Cell types.
template<typename CharT, typename Traits>
static std::basic_ostream<CharT, Traits>& operator << (std::basic_ostream<CharT, Traits>&, const Cell&);

/**
 * @brief Output stream operator for Cons type arguments.
 */
template<typename CharT, typename Traits>
static std::basic_ostream<CharT, Traits>& operator << (std::basic_ostream<CharT, Traits>& os, Cons *cons)
{
    Cell iter { cons };

    os << '(' << car(iter);
    iter = cdr(iter);

    for(Cell slow {iter}; is_pair(iter); iter = cdr(iter), slow = cdr(slow))
    {
        os << ' ' << car(iter);

        if(!is_pair(iter = cdr(iter)) || slow == iter)
        {
            if (slow == iter)
                return os << " ...)";  // circular list detected

            break;
        }
        os << ' ' << car(iter);
    }
    if (is_nil(iter))
        os << ')';                     // list end
    else
        os << " . " << iter << ')';    // dotted pair end

    return os;
}

/**
 * @brief Output stream operator for Cell type arguments.
 */
template<typename CharT, typename Traits>
static std::basic_ostream<CharT,Traits>& operator << (std::basic_ostream<CharT, Traits>& os, const Cell& cell)
{
    overloads fun
    {
    [&os](Nil)         {os << "()";},
    [&os](None)        {os << "#none";},
    [&os](Bool arg)    {os << (arg ? "#T" : "#F");},
    [&os](Int arg)     {os << arg;},
    [&os](Float arg)   {os << arg;},
    [&os](Symbol arg)  {os << '<' << arg.name() << '>';},
    [&os](String arg)  {os << '"' << *arg << '"';},
    [&os](Port*)       {os << "port";},
    [&os](Cons* arg)   {os << arg;},
    [&os](Func)        {os << "function";},


    /* catch missing overloads and emit compile time error message */
    [](auto arg){static_assert(always_false<decltype(arg)>::value, "callable overload is missing");}
    };
    std::visit(std::move(fun), static_cast<const Cell::base_type&>(cell));
    return os;
}

Cell fun_write(const Cell& args)
{
    Port *port = is_nil(cdr(args)) ?
                &std::cout : std::get<Port*>(cadr(args));

    *port << car(args);
    return none;
}

Bool is_list(Cell cell)
{
   for(Cell slow {cell}; is_pair(cell); cell = cdr(cell), slow = cdr(slow))
       if (!is_pair(cell = cdr(cell)) || cell == slow)
           break;
   return is_nil(cell);
}

Int list_length(Cell list)
{
    Int len = 0, slw =0;

    for(Cell slow {list}; is_pair(list); list = cdr(list), slow = cdr(slow), ++len, ++slw)
    {
        ++len;
        if (!is_pair(list = cdr(list)))
            break;

        if (list == slow)
            return ++slw;
    }
    return len;
}

Cell list_ref(Cell list, Int k)
{
    for(/* */; k > 0 && is_pair(list); list = cdr(list), --k)
        ;

    !k || (throw std::invalid_argument("invalid list length"), 0);
    return car(list);
}


void foreach(Func fun, Cell list)
{
    for(Cell slow {list}; is_pair(list); list = cdr(list), slow = cdr(slow))
    {
        fun(car(list));

        if (!is_pair(list = cdr(list)) || list == slow)
            break;

        fun(car(list));
    }
}

Cell fun_foreach(const Cell& args)
{
    auto fun = std::get<Func>(car(args));

    foreach(fun, cdr(args));
    return none;
}

}; // namespace pscm

