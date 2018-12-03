#ifndef GC_HPP
#define GC_HPP

#include <iostream>
#include <set>

#include "types.hpp"

namespace pscm {

class Scheme;

class GCollector {
public:
    void collect(Scheme& scm, const SymenvPtr& env);
    void dump(const Scheme& scm, std::wostream& os = std::wcerr);
    void logging(bool);

private:
    bool is_marked(const Cons&) const noexcept;

    void mark(const Cell&);
    void mark(const Procedure&);
    void mark(const VectorPtr&);
    void mark(const SymenvPtr&);
    void mark(Cons&);

    std::set<VectorPtr::element_type*> vset;
    std::set<SymenvPtr::element_type*> eset;
    bool logon = false;
};

} // namespace pscm
#endif // GC_HPP
