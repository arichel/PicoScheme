/********************************************************************************/ /**
 * @file gc.hpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#ifndef GC_HPP
#define GC_HPP

#include <set>

#include "types.hpp"

namespace pscm {

class Scheme;

/**
 * Rudimentary mark-sweep garbage collector.
 */
class GCollector {
public:
    //! Collect unreachable cons-cells, starting from the argument environment,
    //! or if null-pointer at the scheme interpreter top-environment.
    void collect(Scheme& scm, const SymenvPtr& env = nullptr);

    //! Dump the content of the scheme interpreter global cons-cell store.
    static void dump(const Scheme& scm, const Port<Char>& port = StandardPort<Char>{});

    void logging(bool); //! Enable/disable gc summary logging

private:
    bool is_marked(const Cons&) const noexcept;

    void mark(const Cell&);
    void mark(const Procedure&);
    void mark(const VectorPtr&);
    void mark(SymenvPtr);
    void mark(Cons&);

    std::set<size_t> mset;
    SymenvPtr end = nullptr;
    bool logon = false;
};

} // namespace pscm
#endif // GC_HPP
